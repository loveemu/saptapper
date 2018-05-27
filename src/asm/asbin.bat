@echo off
@setlocal
@rem devkitadv required

@if "%1"=="" (
	echo No input files>&2
	exit /b 1
)

set PREFIX=arm-none-eabi-

%PREFIX%gcc -c -o "%~n1.o" "%1"
if errorlevel 1 exit /b
%PREFIX%objcopy -O binary "%~n1.o" "%~n1.bin"
if errorlevel 1 exit /b

exit /b 0
