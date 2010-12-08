#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// XXX Move out to header
#define sINT 1
#define sDOUBLE 2
#define sCHAR 3
void *dbconnect(const char *);
void *dbclose(void *);
void dbquery(void *, const char *);
int ncol(void *);
void getTypes(void *, int*);
void getColumnNames(void *, char **);
void freeHGlobal(void *, void *);
int fetch(void *, int, int *, void **);
int cur(void *);

SEXP Rdbconnect(SEXP s)
{
  SEXP ret = R_NilValue;
  const char *c = CHAR(STRING_ELT(s,0));
  void *con = dbconnect(c);
  if(con)
    ret = R_MakeExternalPtr(con,R_NilValue,R_NilValue);
  return ret;
}

SEXP Rdbclose(SEXP s)
{
  void *q = R_ExternalPtrAddr(s);
  dbclose(q);
  R_ClearExternalPtr(s);
  return R_NilValue;
}

SEXP Rdbquery(SEXP p, SEXP s)
{
  void *q = R_ExternalPtrAddr(p);
  const char *c = CHAR(STRING_ELT(s,0));
  dbquery(q, c);
  return R_NilValue;
}

SEXP Rncol(SEXP p)
{
  void *q = R_ExternalPtrAddr(p);
  int j = ncol(q);
  return ScalarInteger(j);
}

SEXP RgetTypes(SEXP p)
{
  SEXP ret;
  void *q = R_ExternalPtrAddr(p);
//  int k = INTEGER(t)[0];
  int n = ncol(q);
  PROTECT(ret = allocVector(INTSXP, n));
  getTypes(q, INTEGER(ret));
  UNPROTECT(1);
  return ret;
}

SEXP RgetColumnNames(SEXP p)
{
  SEXP ret;
  int j;
  void *q = R_ExternalPtrAddr(p);
  int n = ncol(q);
  PROTECT(ret = allocVector(STRSXP, n));
  char **data = (char **)malloc(n*sizeof(char *));
  getColumnNames(q, (char **)data);
  for(j=0;j<n;++j) {
    SET_STRING_ELT(ret, j, mkChar(data[j]));
    freeHGlobal(q, (void *)data[j]);
  }
  free(data);
  UNPROTECT(1);
  return ret;
}

// Fetch at most N rows returning a data frame.
SEXP Rfetch(SEXP P, SEXP N)
{
  SEXP column;
  void **data;
  int *intp;
  double *nump;
  char buf[1024];
  int m,j,k,p = 0;
  int n = INTEGER(N)[0];
  void *q = R_ExternalPtrAddr(P);
  int nc = ncol(q);
  int *types = (int *)malloc(nc * sizeof(int));
  char **names = (char **)malloc(nc *sizeof(char *));
  getTypes(q, types);
  getColumnNames(q, names);
  SEXP ret = PROTECT(NEW_LIST(nc));
  SEXP cnames = PROTECT(allocVector(STRSXP, nc));
  p+=3;
  SEXP dataFrameString = PROTECT (allocVector (STRSXP, 1));
  p++;
// 'data' holds an unfortunate copy of the subset. The reason for the copy
// is string handling. :(
  data = (void **)malloc(nc * sizeof(char *));
  for(j=0;j<nc;++j){
    if(types[j] == sINT)    data[j] = (void *)malloc(n * sizeof(int));
    if(types[j] == sDOUBLE) data[j] = (void *)malloc(n * sizeof(double));
    if(types[j] == sCHAR)   data[j] = (void *)malloc(n * sizeof(char *));
  }
  m = fetch(q, n, types, data);  // m holds the number of rows returned

  // Set up the R m * nc data frame
  SEXP rnames = PROTECT(allocVector(STRSXP, m));
  for(j=0;j<nc;++j){
    column = R_NilValue;
    if(names[j]) {
      SET_STRING_ELT(cnames, j, mkChar(names[j]));
      freeHGlobal(q, (void *)names[j]);
    }
    if(types[j] == sINT) {
      SET_VECTOR_ELT(ret, j, allocVector(INTSXP, m));
      column = VECTOR_ELT(ret, j);
      intp = INTEGER(column);
      for(k=0;k<m;++k)
        intp[k] = ((int *)data[j])[k];
    }
    else if(types[j] == sDOUBLE) {
      SET_VECTOR_ELT(ret, j, allocVector(REALSXP, m));
      column = VECTOR_ELT(ret, j);
      nump = REAL(column);
      for(k=0;k<m;++k)
        nump[k] = ((double *)data[j])[k];
    }
    else if(types[j] == sCHAR) {
      SET_VECTOR_ELT(ret, j, allocVector(STRSXP, m));
      column = VECTOR_ELT(ret, j);
      for(k=0;k<m;++k) {
        SET_STRING_ELT(column, k, mkChar(((char **)data[j])[k]));
        freeHGlobal(q, ((void **)data[j])[k]);
      }
    }
    free(data[j]);
  }
  free(data);
  free(names);
  k = cur(q) - m;
  for(j=0;j<m;++j) {
    memset((void *)buf, 0, 1024);
    snprintf(buf, 1024, "Row%d", j+k);
    SET_STRING_ELT(rnames, j, mkChar(buf));
  }
  SET_STRING_ELT (dataFrameString, 0, mkChar ("data.frame"));
  setAttrib (ret, R_ClassSymbol, dataFrameString);
  setAttrib (ret, R_NamesSymbol, cnames);
  setAttrib (ret, R_RowNamesSymbol, rnames);
  UNPROTECT(p);
  return ret;
}
