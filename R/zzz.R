`.onLoad` <- function(libname, pkgname)
{
  library.dynam("RSQLServer", pkgname, libname)
}

`.onUnload` <- function(libpath)
{
  library.dynam.unload("RSF", libpath)
}
