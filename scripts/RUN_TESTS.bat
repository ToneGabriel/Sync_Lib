@echo off

:: Define build folder
set ROOT_DIR=%~dp0..
set BUILD_DIR=%~dp0..\build

:: Build and run project tests
echo.
echo Building and running tests...
echo.
cmake -G "Ninja" -S "%ROOT_DIR%" -B "%BUILD_DIR%"
cmake --build "%BUILD_DIR%"
ctest --test-dir "%BUILD_DIR%"

pause
