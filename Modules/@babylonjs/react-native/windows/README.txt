# Build Steps:
1. Run `npm install` in Apps\Playground
2. Run Modules\@babylonjs\react-native\windows\setup.bat
3. Open Modules\@babylonjs\react-native\windows\BabylonNative.sln and build BabylonNative for the flavors you care about (x86, x64, debug/release, etc)
>> Note: this is currently needed to populate BabylonNative static libs. We should be able to improve these build steps in the future have all build logic executed from Apps\Playground\windows\Playground.sln
4. In Apps\Playground, run 'npm run windows-verbose"