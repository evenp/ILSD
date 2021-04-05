@echo off

mkdir ..\src\Libs\ 2> NUL

REM COPYING IMGUI
echo -------IMGUI-------
mkdir ..\src\Libs\imgui\ 2> NUL
copy ..\deps\imgui\*.h ..\src\Libs\imgui
copy ..\deps\imgui\*.cpp ..\src\Libs\imgui
copy ..\deps\imgui\examples\imgui_impl_glfw.cpp ..\src\Libs\imgui
copy ..\deps\imgui\examples\imgui_impl_glfw.h ..\src\Libs\imgui
copy ..\deps\imgui\examples\imgui_impl_opengl3.cpp ..\src\Libs\imgui
copy ..\deps\imgui\examples\imgui_impl_opengl3.h ..\src\Libs\imgui

REM COPYING GLAD
echo -------GLAD-------
rmdir ..\src\Libs\glad\ /s /q 2> NUL
mkdir ..\src\Libs\glad\ 2> NUL
xcopy ..\deps\glad\* ..\src\Libs\glad /e /i /h

REM COPYING STB
echo -------STB-------
rmdir ..\src\Libs\stbi\ /s /q 2> NUL
mkdir ..\src\Libs\stbi\ 2> NUL
copy ..\deps\stb\stb_image.h ..\src\Libs\stbi
copy ..\deps\stb\stb_image_write.h ..\src\Libs\stbi


REM COPYING SHAPELIB's DLL
echo -------SHAPELIB's DLL-------
mkdir ..\binaries 2> NUL
mkdir ..\binaries\ILSD\Release 2> NUL
mkdir ..\binaries\ILSD\Debug 2> NUL
copy ..\deps\shapelib\build\Debug\shp.dll ..\binaries\ILSD\Release
copy ..\deps\shapelib\build\Debug\shp.dll ..\binaries\ILSD\Debug




pause
