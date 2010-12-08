cl /LD /clr /FU System.dll /FU System.Data.dll /FU System.Xml.dll sql_serv_driver.cpp

@echo READY TO BUILD WITH R:
@echo dlltool -l sql_serv_driver.a -d sql_serv_driver.def -k -A
@echo Rcmd.exe SHLIB RSQLServer.c sql_serv_driver.a
