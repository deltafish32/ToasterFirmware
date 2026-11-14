# Builds and runs the host unit tests with MSVC (Visual Studio C++ build tools).
# Usage: powershell -ExecutionPolicy Bypass -File test\host\run.ps1 [--dump FILE]
$ErrorActionPreference = 'Stop'

$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$lib = Join-Path $root 'src\lib'
$out = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force $out | Out-Null

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) { throw 'vswhere.exe not found. Install Visual Studio with the C++ build tools, or use run.sh with g++/clang.' }
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'No Visual Studio installation with the C++ build tools was found.' }
$vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
$env:PATH = "$(Split-Path $vswhere);$env:PATH"  # vcvars looks for vswhere on PATH

$sources = @(
  (Join-Path $PSScriptRoot 'test_main.cpp'),
  (Join-Path $lib 'yaml_array.cpp'),
  (Join-Path $lib 'yaml_node.cpp'),
  (Join-Path $lib 'strutil.cpp')
) | ForEach-Object { "`"$_`"" }

$compat = Join-Path $PSScriptRoot 'host_compat.h'
$exe = Join-Path $out 'host_test.exe'
$cl = "cl /nologo /std:c++17 /EHsc /utf-8 /W3 /wd4267 /wd4996 /D_CRT_SECURE_NO_WARNINGS /FI`"$compat`" /I`"$lib`" /Fo`"$out\\`" /Fe`"$exe`" $($sources -join ' ')"

cmd /c "`"$vcvars`" >nul && $cl"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $exe @args
exit $LASTEXITCODE
