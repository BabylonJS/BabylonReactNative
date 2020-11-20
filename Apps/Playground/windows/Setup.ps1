Push-Location "$PSScriptRoot\..\..\..\Modules\@babylonjs\react-native\submodules\BabylonNative"

$BuildDir = "Build_uwp_x64"
mkdir $BuildDir
cd $BuildDir
cmake -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 ..

Pop-Location