Push-Location "$PSScriptRoot\..\submodules\BabylonNative"

$BuildDir = "Build_uwp_x64"
if (!(Test-Path $BuildDir)) {
  mkdir $BuildDir
}

cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -D NAPI_JAVASCRIPT_ENGINE=JSI ..\..\..\windows
cd ..

$BuildDir = "Build_uwp_arm64"
if (!(Test-Path $BuildDir)) {
  mkdir $BuildDir
}

cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -D NAPI_JAVASCRIPT_ENGINE=JSI -A arm64 ..\..\..\windows

Pop-Location