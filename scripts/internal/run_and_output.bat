@echo off
:: %1 = path to test executable
:: %2 = path to output log file

"%~1" > "%~2" 2>&1
