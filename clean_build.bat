@echo off
echo Cleaning build artifacts...

:: Delete Windows executables
if exist MyProgram.exe del /f /q MyProgram.exe

:: Delete object files
for /r %%i in (*.obj) do del /f /q "%%i"
for /r %%i in (*.ilk) do del /f /q "%%i"
for /r %%i in (*.pdb) do del /f /q "%%i"
for /r %%i in (*.log) do del /f /q "%%i"
for /r %%i in (*.idb) do del /f /q "%%i"

:: Delete Visual Studio temporary folders
if exist x64 rmdir /s /q x64
if exist build rmdir /s /q build

echo Clean complete.
pause
