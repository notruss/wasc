set PATH=%PATH%;c:\_progs\mingw64\bin
set CC=gcc.exe

rem mingw32-make
mingw32-make -j %NUMBER_OF_PROCESSORS%

pause
