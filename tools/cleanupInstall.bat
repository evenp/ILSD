@echo off
cd ..\
rmdir /S /Q .\deps\glfw\build >nul
rmdir /S /Q .\deps\shapelib\build >nul
rmdir .\src\Libs /S /Q >nul
rmdir .\binaries /S /Q >nul
rmdir .\intermediate /S /Q >nul
del /f /s /q .\.vs 1>nul
rmdir /s /q .vs >nul
del ILSD.sln >nul
del ILSD.vcxproj >nul
del ILSD.vcxproj.filters >nul
del ILSD.vcxproj.user >nul
pause
