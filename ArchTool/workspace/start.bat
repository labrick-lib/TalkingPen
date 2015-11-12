del AudioData.bin
del Index.exe
del libSiren7.dll
del log.txt
del siren7.exe

mkdir audio
move *.wav audio

copy tools\clean.bat audio
copy tools\Index.exe audio
copy tools\siren7.exe audio
copy tools\libSiren7.dll audio
copy s7\*.s7 audio
copy s7\data.n audio

cd audio
Index.exe