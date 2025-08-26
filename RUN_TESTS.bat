@echo off
echo Running all test...

:: Define build and logs folder
set BUILD_DIR=%~dp0build
set BIN_DIR=%BUILD_DIR%/bin
set LOGS_DIR=%~dp0logs

:: Create logs folder if it doesn't exist
if not exist "%LOGS_DIR%" (
    mkdir "%LOGS_DIR%"
)

:: Build project tests
echo.
echo Building tests...
echo.
cmake -G "Ninja" -B "%BUILD_DIR%"
cmake --build "%BUILD_DIR%"

:: Loop through all exe files in build/bin folder and output the results in logs folder
for %%F in ("%BIN_DIR%\*.exe") do (
    echo Running %%~nxF ...
    "%%F" > "%LOGS_DIR%\%%~nF.txt" 2>&1
)

echo.
echo All tests finished.
pause
