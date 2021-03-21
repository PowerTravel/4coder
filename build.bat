@echo off

set opts=-FC -GR- -EHa- -nologo -Zi
set code=%cd%
call "W:\apps\4coder\custom\bin\buildsuper_x64-win.bat" "W:\4coderCustom\4coder_custom_layer.cpp" debug
copy .\custom_4coder.dll W:\apps\4coder\custom_4coder.dll
copy .\custom_4coder.pdb W:\apps\4coder\custom_4coder.pdb