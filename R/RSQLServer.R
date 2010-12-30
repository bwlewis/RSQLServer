`RSQLServer_connect` <- function(string)
{
  .Call("Rdbconnect", string)
}

`RSQLServer_close` <- function(con)
{
  if(is.null(con)) return(NULL)
  .Call("Rdbclose", con)
}

`RSQLServer_query` <- function(con, string)
{
  if(is.null(con)) return(NULL)
  .Call("Rdbquery", con, as.character(string))
}

`RSQLServer_ncol` <- function(con)
{
  if(is.null(con)) return(NULL)
  .Call("Rncol", con)
}

`RSQLServer_columnTypes` <- function(con)
{
  if(is.null(con)) return(NULL)
  .Call("RgetTypes", con)
}

`RSQLServer_columnNames` <- function(con)
{
  if(is.null(con)) return(NULL)
  .Call("RgetColumnNames", con)
}

`RSQLServer_fetch` <- function(con, n=1L)
{
  if(is.null(con)) return(NULL)
  if(n<1) return(NULL)
  .Call("Rfetch", con, as.integer(n))
}

`RSQLServer_fetchTable` <- function(con, n=2000000)
{
  if(is.null(con)) return(NULL)
  x <- .Call("Rfetch", con, as.integer(n))
  df <- x
  while(nrow(x) > 0) {
    x <- .Call("Rfetch", con, as.integer(n))
    if(nrow(x) > 0)
      df <- rbind(df, x)
  }
  df
}
