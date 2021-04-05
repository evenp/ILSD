@echo off
mkdir resources 2> Nul
mkdir resources\captures 2> Nul
mkdir resources\config 2> Nul
mkdir resources\exports 2> Nul
mkdir resources\selections 2> Nul
mkdir resources\selections\ctracks 2> Nul
mkdir resources\selections\ridges 2> Nul
mkdir resources\selections\hollows 2> Nul
mkdir resources\tiles 2> Nul
mkdir resources\strokes 2> Nul
mkdir resources\tests 2> Nul
mkdir resources\nvm 2> Nul
mkdir resources\til 2> Nul
mkdir resources\til\top 2> Nul
mkdir resources\til\mid 2> Nul
mkdir resources\til\eco 2> Nul
"tools/premake5.exe" --file=src/premakeFile.lua vs2019
pause
