Import-Module $PSScriptRoot\Utils.psm1

git submodule update --init "$PSScriptRoot\..\..\submodules\BabylonNative"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\SPIRV-Cross"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\arcana.cpp"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\base-n"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\bgfx.cmake"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\glslang"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\xr\Dependencies\OpenXR-SDK"
git submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative\Dependencies\xr\Dependencies\OpenXR-SDK"

Restore-CMakeProject -Platform "x64"