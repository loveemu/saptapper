@rem devkitadv required

@if "%1"=="" @goto noargs

@gcc -o "%~n1.tmp.bin" "%1"
@if errorlevel 1 @goto abort
@objcopy -O binary "%~n1.tmp.bin"
@if errorlevel 1 @goto abort

@exit /b 0

:noargs
@echo No input files>&2
@exit /b 1

:abort
