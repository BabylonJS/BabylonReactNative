# Build Steps:
1. Run `yarn install` in Apps\Playground
2. Run Modules\@babylonjs\react-native\windows\setup.bat
3. Open BabylonNative.sln and build the x86, x64, arm, arm64 dependencies you care about
>> Note: BabylonNative won't resolve dependencies correctly from this location and needs to be built with Apps\Playground
4. Run `yarn install --force` in Apps\Playground
>> Note: this recopies the Modules\@babylonjs\react-native package content and is needed to insure lib dependencies are populated
5. Run 'npm run windows-verbose"