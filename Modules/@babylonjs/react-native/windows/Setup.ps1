Push-Location "$PSScriptRoot\..\submodules\BabylonNative"

Write-Output "`nSetting up x86 BabylonNative dependencies..."
$BuildDir = "Build_uwp_x86"
if (!(Test-Path $BuildDir)) {
  mkdir $BuildDir
}

cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -D NAPI_JAVASCRIPT_ENGINE=JSI -A Win32 ..\..\..\windows
cd ..

Write-Output "`nSetting up x64 BabylonNative dependencies..."
$BuildDir = "Build_uwp_x64"
if (!(Test-Path $BuildDir)) {
  mkdir $BuildDir
}

cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -D NAPI_JAVASCRIPT_ENGINE=JSI ..\..\..\windows
cd ..

Write-Output "`nSetting up ARM BabylonNative dependencies..."
$BuildDir = "Build_uwp_arm"
if (!(Test-Path $BuildDir)) {
  mkdir $BuildDir
}

cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -D NAPI_JAVASCRIPT_ENGINE=JSI -A arm ..\..\..\windows
cd ..

Write-Output "`nSetting up ARM64 BabylonNative dependencies..."
$BuildDir = "Build_uwp_arm64"
if (!(Test-Path $BuildDir)) {
  mkdir $BuildDir
}

cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -D NAPI_JAVASCRIPT_ENGINE=JSI -A arm64 ..\..\..\windows

Pop-Location