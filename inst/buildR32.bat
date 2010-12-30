e:\blewis\mingw64\bin\gendef.exe sql_serv_driver.dll

e:\blewis\mingw32\bin\dlltool.exe -l sql_serv_driver.a -d sql_serv_driver.def -k -A

"c:\Program Files\R\R-2.12.0\bin\i386\Rcmd.exe" SHLIB RSQLServer.c sql_serv_driver.a 
copy *.dll libs\i386
