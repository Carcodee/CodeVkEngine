param(
    [string]$CodeCudaProject = "",
    [switch]$Force
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$dependencyRoot = Join-Path $repoRoot "dependencies"
$cacheRoot = Join-Path $repoRoot ".dependency-cache"

if (-not $CodeCudaProject) {
    $CodeCudaProject = Join-Path (Split-Path -Parent $repoRoot) "CodeCudaEngine\project"
}
$CodeCudaProject = [System.IO.Path]::GetFullPath($CodeCudaProject)

$slangVersion = "2024.17"
$slangArchive = Join-Path $cacheRoot "slang-$slangVersion-windows-x86_64.zip"
$slangUrl = "https://github.com/shader-slang/slang/releases/download/v$slangVersion/slang-$slangVersion-windows-x86_64.zip"
$slangSha256 = "7F3DF6F8D27518902C7BE0440A78A423B4149B9FF7725A813F5BDF2458E5C480"

$glfwVersion = "3.4"
$glfwArchive = Join-Path $cacheRoot "glfw-$glfwVersion.bin.WIN64.zip"
$glfwUrl = "https://github.com/glfw/glfw/releases/download/$glfwVersion/glfw-$glfwVersion.bin.WIN64.zip"
$glfwSha256 = "54EFA829400F2A0537F742B2B3BDD74E437BB4F2F048E4B7D3C5557D11A611E6"

function Get-VerifiedArchive {
    param(
        [string]$Url,
        [string]$Destination,
        [string]$Sha256
    )

    if (-not (Test-Path -LiteralPath $Destination)) {
        Write-Host "Downloading $Url"
        Invoke-WebRequest -Uri $Url -OutFile $Destination -UseBasicParsing
    }

    $actualHash = (Get-FileHash -LiteralPath $Destination -Algorithm SHA256).Hash
    if ($actualHash -ne $Sha256) {
        throw "SHA256 mismatch for ${Destination}. Expected ${Sha256}, got ${actualHash}."
    }
}

$cudaBuildScript = Join-Path $CodeCudaProject "build_dist.ps1"
if (-not (Test-Path -LiteralPath $cudaBuildScript)) {
    throw "CodeCudaEngine build script not found: $cudaBuildScript. Clone CodeCudaEngine beside CodeVkEngine or pass -CodeCudaProject."
}

$cudaLibrary = Join-Path $dependencyRoot "CodeCudaEngine\lib\codeCudaLib.lib"
if ($Force -or -not (Test-Path -LiteralPath $cudaLibrary)) {
    Write-Host "Building and installing CodeCudaEngine"
    & $cudaBuildScript -DistDir (Join-Path $dependencyRoot "CodeCudaEngine")
}

$slangLibrary = Join-Path $dependencyRoot "Slang\lib\slang.lib"
$slangDll = Join-Path $dependencyRoot "Slang\bin\slang.dll"
$slangGlslangDll = Join-Path $dependencyRoot "Slang\bin\slang-glslang.dll"
$glfwLibrary = Join-Path $dependencyRoot "glfw\lib-vc2022\glfw3.lib"

if ($Force -or
    -not (Test-Path -LiteralPath $slangLibrary) -or
    -not (Test-Path -LiteralPath $slangDll) -or
    -not (Test-Path -LiteralPath $slangGlslangDll) -or
    -not (Test-Path -LiteralPath $glfwLibrary)) {
    New-Item -ItemType Directory -Force -Path $cacheRoot | Out-Null
    Get-VerifiedArchive -Url $slangUrl -Destination $slangArchive -Sha256 $slangSha256
    Get-VerifiedArchive -Url $glfwUrl -Destination $glfwArchive -Sha256 $glfwSha256

    $extractRoot = Join-Path ([System.IO.Path]::GetTempPath()) "CodeVkEngine-dependencies-$PID-$([guid]::NewGuid().ToString('N'))"
    New-Item -ItemType Directory -Path $extractRoot | Out-Null

    try {
        $slangExtract = Join-Path $extractRoot "slang"
        $glfwExtract = Join-Path $extractRoot "glfw"
        Expand-Archive -LiteralPath $slangArchive -DestinationPath $slangExtract
        Expand-Archive -LiteralPath $glfwArchive -DestinationPath $glfwExtract

        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $slangLibrary) | Out-Null
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $slangDll) | Out-Null
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $glfwLibrary) | Out-Null

        Copy-Item -LiteralPath (Join-Path $slangExtract "lib\slang.lib") -Destination $slangLibrary -Force
        Copy-Item -LiteralPath (Join-Path $slangExtract "bin\slang.dll") -Destination $slangDll -Force
        Copy-Item -LiteralPath (Join-Path $slangExtract "bin\slang-glslang.dll") -Destination $slangGlslangDll -Force
        Copy-Item -LiteralPath (Join-Path $glfwExtract "glfw-$glfwVersion.bin.WIN64\lib-vc2022\glfw3.lib") -Destination $glfwLibrary -Force
    }
    finally {
        if (Test-Path -LiteralPath $extractRoot) {
            Remove-Item -LiteralPath $extractRoot -Recurse -Force
        }
    }
}

Write-Host "Dependencies are ready."
Write-Host "Configure with: cmake -S . -B cmake-build-msvc -G `"Visual Studio 17 2022`" -A x64"
