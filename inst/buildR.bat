echo Make sure that Rcmd.exe is from the 64-bit R path.
echo Gendef and dlltool are part of MinGW.
gendef.exe sql_serv_driver.dll
dlltool.exe -l sql_serv_driver.a -d sql_serv_driver.def -k -A
Rcmd.exe SHLIB RSQLServer.c sql_serv_driver.a 
copy *.dll libs\x64\
