@echo off

set BUILD=

IF EXIST %1.txt (
	set /p BUILD=<%1.txt
) ELSE (
	set BUILD=0
)

set /a BUILD=%BUILD%+1
@echo %BUILD% >%1.txt

echo Successfully wrote build number to %1.txt.