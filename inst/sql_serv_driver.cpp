
// compile (dll): /LD /clr /FU System.dll /FU System.Data.dll /FU System.Xml.dll

#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
#include <comdef.h>
#include <gcroot.h>
#include <iostream>

// Return types
#define sINT    1
#define sDOUBLE 2
#define sCHAR   3

#using <System.Data.dll>
#using <mscorlib.dll>
#using <System.dll>
#using <System.Xml.dll>

using namespace std;
using namespace System;
using namespace System::Data;
using namespace System::Runtime::InteropServices;
using namespace System::Data::SqlClient;
using namespace System::Collections::Generic;
using namespace System::Diagnostics;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace System::Text;
using namespace System::Reflection;


#pragma managed
class RSqlClient 
{
public:

  RSqlClient(string con) 
  {
    cn = gcnew SqlConnection();
    try {
      cn->ConnectionString = %String(con.c_str());
      cn->Open();
      ok = true;
      cursor = 0;
    }
    catch(...) {
      cn = NULL;
      ok = false;
    }
  }

  bool valid()
  {
    return ok;
  }

  void query(string s)
  {
    String ^q;
    SqlCommand^ command;
    try {
      q = %String(s.c_str());
      command = gcnew SqlCommand(q, cn);
      reader = command->ExecuteReader();
      cursor = 0;
    }
    catch(...) {
printf("query error\n");
      ok = false;
    }
  }

// Return the number of columns for the current row
  int ncol()
  {
    try {
      return reader->FieldCount;
    } catch(...) {
printf("ncol error\n");
      return 0;
    }
  }

// types must be large enough to hold a return type for each column.
// XXX We only support a small subset of possible types right now.
  void getTypes(int *types)
  {
    int j;
    if(!ok) return;
    for(j=0; j < reader->FieldCount; ++j) {
      Type^ t = reader->GetFieldType(j);
      if(t == double::typeid || t == Decimal::typeid) {
        // Double
        types[j] = sDOUBLE;
      }
      else if(t == int::typeid || t == Int16::typeid || t == Int32::typeid) {
        // Integer
        types[j] = sINT;
      }
      else {
        // Character
        types[j] = sCHAR;
      }
    }
  }

// names must be large enough to hold a name for each column.
  void getColumnNames(char **names)
  {
    int j;
    if(!ok) return;
    for(j=0; j < reader->FieldCount; ++j) {
      names[j] = (char*) Marshal::StringToHGlobalAnsi(reader->GetName(j)).ToPointer();
    }
  }

// Fetch a block of at most n rows, placing output in 'data' and
// returning the number of rows fetched. The types indicate the desired type
// of each output column. Aside from strings, the data in 'data' must already
// be allocated. Strings are allocated internally by .Net and must be later
// freed with the 'Free' method below.
  int fetchRows(int n, int *types, void **data)
  {
    int k, j = 0;
    if(!ok) return 0;
    if(!reader) return 0;
    if(!reader->HasRows) return 0;
    while(reader->Read() && (j < n)) {
      for(k=0;k < ncol(); ++k){
        if(types[k] == sINT) ((int*)data[k])[j] = (int)reader->GetInt32(k);
	else if(types[k] == sDOUBLE) ((double *)data[k])[j] =
		reader->GetDouble(k);
	else if(types[k] == sCHAR) ((char **)data[k])[j] = 
		(char *) Marshal::StringToHGlobalAnsi(
				reader->GetString(k)).ToPointer();
      }
      j++;
    }
    cursor += j;
    return j;
  }

  int getCursor()
  {
    return cursor;
  }

  void Free(void *x)
  {
    Marshal::FreeHGlobal(IntPtr(x));
  }

private:
  gcroot<SqlDataReader ^>reader;
  gcroot<SqlConnection ^>cn;
  int cursor;
  bool ok;
};



#pragma unmanaged

extern "C" __declspec( dllexport ) void *dbconnect(const char *s)
{
  RSqlClient *q;
  std::string r = string(s);
  q = new RSqlClient(r);
  if(! q->valid()) {
    delete(q);
    return NULL;
  }
  return (void *)q;
}

extern "C" __declspec( dllexport ) void dbclose(void *q)
{
  if(q) delete((RSqlClient *)q);
}

extern "C" __declspec( dllexport ) void dbquery(void *q, const char *s)
{
  std::string r = string(s);
  if(q) ((RSqlClient *)q)->query(r);
}

extern "C" __declspec( dllexport ) int ncol(void *q)
{
  if(q) return ((RSqlClient *)q)->ncol();
  else return 0;
}

extern "C" __declspec( dllexport ) void getTypes(void *q, int *types)
{
  if(q) ((RSqlClient *)q)->getTypes(types);
}

extern "C" __declspec( dllexport ) void getColumnNames(void *q, char **n)
{
  if(q) ((RSqlClient *)q)->getColumnNames(n);
}

extern "C" __declspec( dllexport ) void freeHGlobal(void *q, void *x)
{
  if(q) ((RSqlClient *)q)->Free(x);
}

extern "C" __declspec( dllexport ) int
  fetch(void *q, int n, int *types, void **data)
{
  if(q) return ((RSqlClient *)q)->fetchRows(n, types, data);
  else return 0;
}

extern "C" __declspec( dllexport ) int cur(void *q)
{
  if(q) return ((RSqlClient *)q)->getCursor();
  else return 0;
}
