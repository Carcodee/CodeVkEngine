@echo off
cd /d "%~dp0"
setlocal EnableExtensions EnableDelayedExpansion
set CompilerExe="%VULKAN_SDK%\Bin\glslangValidator.exe"
set OptimizerConfig="OptimizerConfig.cfg"
set "errorfound="

for /r glsl/ %%I in (*.vert) do (
    set outname=%%I
    set outname=!outname:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !outname!
    
   %CompilerExe% -V "%%I" -l --target-env vulkan1.2 -o "!outname!".spv || set "errorfound=1"
   
)

for /r glsl/ %%I in (*.frag) do (
    set outname=%%I
    set outname=!outname:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !outname!
    
   %CompilerExe% -V "%%I" -l --target-env vulkan1.2 -o "!outname!".spv || set "errorfound=1"
)

for /r glsl/ %%I in (*.comp) do (
    set outname=%%I
    set outname=!outname:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !outname!
    
   %CompilerExe% -V "%%I" -l --target-env vulkan1.2 -o "!outname!".spv || set "errorfound=1"
)
if defined errorfound (
    echo.
    echo Errors were found during compilation.
    pause
)
