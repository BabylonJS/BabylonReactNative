# Build Steps:
1. Run `npm install` in Apps\Playground
2. Run `Modules\@babylonjs\react-native\windows\setup.bat`
3. Open Modules\@babylonjs\react-native\windows\BabylonReactNative.sln and build BabylonReactNative for the flavors you care about (x86, x64, debug/release, etc)
>> Note: this is currently needed to populate BabylonNative static libs. We should be able to improve these build steps in the future have all build logic executed from Apps\Playground\windows\Playground.sln
4. In Apps\Playground, run 'npm run windows-verbose"
>> Note: if you experience build issues for Apps\Playground related to autolinking, try running `npx react-native autolink-windows -logging` in the Apps\Playground folder