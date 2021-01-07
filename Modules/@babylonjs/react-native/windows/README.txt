# Build Steps:
1. Run `yarn install` in Apps\Playground
2. Run Modules\@babylonjs\react-native\windows\setup.bat
3. Open Modules\@babylonjs\react-native\windows\BabylonNative.sln and build BabylonNative for the flavors you care about (x86, x64, debug/release, etc)
>> Note: this is currently needed to populate BabylonNative static libs. We should be able to improve these build steps in the future to all be executed from Apps\Playground\windows\Playground.sln
4. Run `yarn install --force` in Apps\Playground
>> Note: this hard copies the Modules\@babylonjs\react-native package content compared to npm that generates symbolic links. Be sure to use --force to ensure that the built BabylonNative.dll and static lib dependencies are recopied to the Apps\Playgroud's node_modules folder.
5. Run 'npm run windows-verbose"