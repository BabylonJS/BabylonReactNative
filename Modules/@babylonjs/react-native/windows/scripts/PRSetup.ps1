Import-Module $PSScriptRoot\Utils.psm1

git submodule update --init "$PSScriptRoot\..\..\submodules\BabylonNative"

Push-Location "$PSScriptRoot\..\..\submodules\BabylonNative"
git submodule update --init --recursive "Dependencies\SPIRV-Cross"
git submodule update --init --recursive "Dependencies\arcana.cpp"
git submodule update --init --recursive "Dependencies\base-n"
git submodule update --init --recursive "Dependencies\bgfx.cmake"
git submodule update --init --recursive "Dependencies\glslang"
git submodule update --init --recursive "Dependencies\xr\Dependencies\OpenXR-SDK"
git submodule update --init --recursive "Dependencies\xr\Dependencies\OpenXR-SDK"
Pop-Location

Restore-CMakeProject -Platform "x64"