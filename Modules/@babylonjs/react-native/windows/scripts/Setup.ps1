Import-Module $PSScriptRoot\Utils.psm1

# windows build agents don't support the path lengths required for initializing arcore dependencies,
# so we manually initialize the submodules we need here.
git -c submodule."Dependencies/xr/Dependencies/arcore-android-sdk".update=none submodule update --init --recursive "$PSScriptRoot\..\..\submodules\BabylonNative"
            
Restore-CMakeProject -Platform "x86"
Restore-CMakeProject -Platform "x64"
Restore-CMakeProject -Platform "arm"
Restore-CMakeProject -Platform "arm64"