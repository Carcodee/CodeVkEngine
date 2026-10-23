@echo off
setlocal EnableExtensions EnableDelayedExpansion
pushd "%~dp0"

set current_dir=%~dp0
set project_dir=%current_dir%\..\..
for %%I in ("%project_dir%") do set project_dir=%%~fI


set compiler_exe="%VULKAN_SDK%\Bin\glslangValidator.exe"
set slang_comp_exe="%project_dir%\dependencies\Slang\bin\slangc.exe"
set optimizer_config="OptimizerConfig.cfg"
set "error_found="
@echo %slang_comp_exe%


for /r glsl/ %%I in (*.vert) do (
    set out_name=%%I
    set out_name=!out_name:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !out_name!
    
   %compiler_exe% -V "%%I" -l --target-env vulkan1.2 -o "!out_name!".spv || set "error_found=1"
   
)

for /r glsl/ %%I in (*.frag) do (
    set out_name=%%I
    set out_name=!out_name:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !out_name!
    
   %compiler_exe% -V "%%I" -l --target-env vulkan1.2 -o "!out_name!".spv || set "error_found=1"
)

for /r glsl/ %%I in (*.tesc) do (
    set out_name=%%I
    set out_name=!out_name:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !out_name!
    
   %compiler_exe% -V "%%I" -l --target-env vulkan1.2 -o "!out_name!".spv || set "error_found=1"
)

for /r glsl/ %%I in (*.tese) do (
    set out_name=%%I
    set out_name=!out_name:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !out_name!
    
   %compiler_exe% -V "%%I" -l --target-env vulkan1.2 -o "!out_name!".spv || set "error_found=1"
)

for /r glsl/ %%I in (*.geom) do (
    set out_name=%%I
    set out_name=!out_name:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !out_name!
    
   %compiler_exe% -V "%%I" -l --target-env vulkan1.2 -o "!out_name!".spv || set "error_found=1"
)

for /r glsl/ %%I in (*.comp) do (
    set out_name=%%I
    set out_name=!out_name:\glsl\=\spirvGlsl\!
    @echo compiling %%I
    @echo To !out_name!
    
   %compiler_exe% -V "%%I" -l --target-env vulkan1.2 -o "!out_name!".spv || set "error_found=1"
)

for /r slang/ %%I in (*.slang) do (
    set out_name=%%I
    set out_name=!out_name:\slang\=\spirvSlang\!
    @echo compiling slang %%I
    @echo To !out_name!
    
    findstr /c:"Main_Cs" "%%I" >nul && (
        %slang_comp_exe% -target spirv -profile glsl_450 -preserve-params -stage compute -entry Main_Cs "%%I" -o "!out_name!"_CS.spv || set "error_found=1"
    ) || (
        %slang_comp_exe% -target spirv -profile glsl_450 -preserve-params -stage vertex -entry Main_Vs "%%I" -o "!out_name!"_VS.spv || set "error_found=1"
        %slang_comp_exe% -target spirv -profile glsl_450 -preserve-params -stage fragment -entry Main_Fs "%%I" -o "!out_name!"_FS.spv || set "error_found=1"
    )
)
if defined error_found (
    echo.
    echo Errors were found during compilation.
    popd
    pause
    exit /b 1
)
popd
exit /b 0
