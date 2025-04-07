@echo off

set CurrentDir=%cd%
set ProjectDir=%CurrentDir%\..\..
for %%I in ("%ProjectDir%") do set ProjectDir=%%~fI


setlocal EnableExtensions EnableDelayedExpansion
set CompilerExe="%VULKAN_SDK%\Bin\glslangValidator.exe"
set SlangCompExe="%ProjectDir%\dependencies\Slang\bin\slangc.exe"
set OptimizerConfig="OptimizerConfig.cfg"
set "errorfound="
@echo %SlangCompExe%


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

for /r slang/ %%I in (*.slang) do (
    set outname=%%I
    set outname=!outname:\slang\=\spirvSlang\!
    @echo compiling slang %%I
    @echo To !outname!
    
    findstr /c:"mainCS" "%%I" >nul && (
        %SlangCompExe% -target spirv -profile glsl_450 -stage compute -entry mainCS "%%I" -o "!outname!"_CS.spv || set "errorfound=1"
    ) || (
        %SlangCompExe% -target spirv -profile glsl_450 -stage vertex -entry mainVS "%%I" -o "!outname!"_VS.spv || set "errorfound=1"
        %SlangCompExe% -target spirv -profile glsl_450 -stage fragment -entry mainFS "%%I" -o "!outname!"_FS.spv || set "errorfound=1"
    )
)
if defined errorfound (
    echo.
    echo Errors were found during compilation.
    pause
)
