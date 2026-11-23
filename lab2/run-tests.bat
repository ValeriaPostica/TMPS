@echo off
g++ -std=c++17 -Wall -Wextra -pedantic tests.cpp -o tests.exe
if errorlevel 1 exit /b 1
tests.exe
