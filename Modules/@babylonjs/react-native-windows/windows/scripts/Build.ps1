param (
    [Parameter(Mandatory = $false)]
    [string]$Platform,
    [Parameter(Mandatory = $false)]
    [string]$Configuration
)

Import-Module $PSScriptRoot\Utils.psm1

if ((!$Platform -And $Configuration) -Or
    ($Platform -And !$Configuration)) {
    Write-Error "Platform and Configuration flags must be provided together"
    exit 1;
}

if (!$Platform -And !$Configuration) {
    Compile-Solution -Platform "Win32" -Configuration "Debug" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_x86\ReactNativeBabylon.sln"
    Compile-Solution -Platform "Win32" -Configuration "Release" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_x86\ReactNativeBabylon.sln"
    Compile-Solution -Platform "x64" -Configuration "Release" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_x64\ReactNativeBabylon.sln"
    Compile-Solution -Platform "x64" -Configuration "Debug" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_x64\ReactNativeBabylon.sln"
    Compile-Solution -Platform "ARM" -Configuration "Debug" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_arm\ReactNativeBabylon.sln"
    Compile-Solution -Platform "ARM" -Configuration "Release" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_arm\ReactNativeBabylon.sln"
    Compile-Solution -Platform "ARM64" -Configuration "Release" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_arm64\ReactNativeBabylon.sln"
    Compile-Solution -Platform "ARM64" -Configuration "Debug" -Solution "$PSScriptRoot\..\..\..\react-native\Build\uwp_arm64\ReactNativeBabylon.sln"
}
else {
    $DirectoryMap = @{ "Win32"="uwp_x86"; "x64" = "uwp_x64"; "ARM"="uwp_arm"; "ARM64"="uwp_arm64"; };
    $Directory = $DirectoryMap[$Platform];
    Compile-Solution -Platform $Platform -Configuration $Configuration -Solution "$PSScriptRoot\..\..\..\react-native\Build\$Directory\ReactNativeBabylon.sln"
}

