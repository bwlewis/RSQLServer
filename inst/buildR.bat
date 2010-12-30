
e:\blewis\mingw64\bin\gendef.exe sql_serv_driver.dll
e:\blewis\mingw64\bin\dlltool.exe -l sql_serv_driver.a -d sql_serv_driver.def -k -A
"c:\Program Files\R\R-2.12.0\bin\x64\Rcmd.exe" SHLIB RSQLServer.c sql_serv_driver.a 
copy *.dll libs\x64\
