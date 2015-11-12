mkdir ..\s7 ..\result
move *.s7 ..\s7
move AudioData.bin ..\result
move log.txt ..\result
mv data.y data.n
move data.n ..\s7

del data
del conf.ini
del libSiren7.dll
del siren7.exe
del clean.bat