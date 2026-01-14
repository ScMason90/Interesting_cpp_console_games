@echo off
echo Cleaning intermediate files...

:: Delete common intermediate files
:: del /s /q *.obj
:: Delete compiler-generated .obj files but preserve model resources
for %%f in (*.obj) do (
    if not "%%~dpf"=="Source_Files\" (
        del "%%f"
    )
)

del /s /q *.iobj
del /s /q *.pdb
del /s /q *.idb
del /s /q *.ipch
del /s /q *.tlog
del /s /q *.log
del /s /q *.vc.db
del /s /q *.lastbuildstate
del /s /q *.ilk
del /s /q *.exp
del /s /q *.pch

:: Delete Visual Studio cache folders
rd /s /q .vs
rd /s /q x64
rd /s /q Debug
rd /s /q Release

echo Cleanup complete!
pause
