const util = require('util');
const fs = require('fs');
const exec = require('child_process').exec;
const spawn = require('child_process').spawn;
const execAsync = util.promisify(exec);
const readdirAsync = util.promisify(fs.readdir);
const log = require('fancy-log');
const del = require('del');
const gulp = require('gulp');
const parseArgsStringToArgv = require('string-argv').parseArgsStringToArgv

async function runCommand(command, options = null, logCommand = true) {
  if (logCommand) {
    log(command);
  }

  const { stdout, stderr } = await execAsync(command, options);
  if (stderr) {
    log.error(stderr);
  }
  if (stdout) {
    log(stdout);
  }
}

async function runProcess(command, options = {}, logCommand = true) {
  if (logCommand) {
    log(command);
  }

  const arguments = parseArgsStringToArgv(command);
  command = arguments.shift();

  await new Promise((resolve, reject) => {
    const process = spawn(command, arguments, options);

    process.on('close', code => code ? reject(code) : resolve());

    process.stdout.on('data', data => {
      const message = data.toString();
      if (message !== '\n') {
        log(message);
      }
    });

    process.stderr.on('data', data => {
      log.error(data.toString());
    })
  });
}

const clean = async () => {
  await del('Assembled', {force:true});
}

const makeXCodeProj = async () => {
  await runCommand('mkdir -p iOS/Build');
  await runProcess('cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../../Apps/Playground/node_modules/@babylonjs/react-native/submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DDEPLOYMENT_TARGET=12 -DENABLE_GLSLANG_BINARIES=OFF -DSPIRV_CROSS_CLI=OFF ..', {cwd: 'iOS/Build'});
};

const buildIphoneOS = async () => {
  await runProcess('xcodebuild -sdk iphoneos -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO', {cwd: 'iOS/Build'});
};

const buildIphoneSimulator = async () => {
  await runProcess('xcodebuild -sdk iphonesimulator -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO', {cwd: 'iOS/Build'});
};

const buildIOS = gulp.series(makeXCodeProj, buildIphoneOS, buildIphoneSimulator);

const buildAndroid = async () => {
  await runProcess('./gradlew babylonjs_react-native:assembleRelease', {cwd: '../Apps/Playground/android'});
};

const copyCommonFiles = async () => {
  await runCommand('mkdir -p Assembled');
  await runCommand('cp ../Apps/Playground/node_modules/@babylonjs/react-native/package.json Assembled');
  await runCommand('cp ../Apps/Playground/node_modules/@babylonjs/react-native/README.md Assembled');
  await runCommand('cp ../Apps/Playground/node_modules/@babylonjs/react-native/*.ts* Assembled');
  await runCommand('cp react-native-babylon.podspec Assembled');
};

const copyIOSFiles = async () => {
  await runCommand('mkdir -p Assembled/ios');
  await runCommand('cp ../Apps/Playground/node_modules/@babylonjs/react-native/ios/*.h Assembled/ios');
  await runCommand('cp ../Apps/Playground/node_modules/@babylonjs/react-native/ios/*.mm Assembled/ios');
  // This xcodeproj is garbage that we don't need in the package, but `pod install` ignores the package if it doesn't contain at least one xcodeproj. 🤷🏼‍♂️
  await runCommand('cp -R iOS/Build/ReactNativeBabylon.xcodeproj Assembled/ios');
};

const createIOSUniversalLibs = async () => {
  await runCommand('mkdir -p Assembled/ios/libs');
  const libs = await readdirAsync('iOS/Build/Release-iphoneos');
  await Promise.all(libs.map(lib => runCommand(`lipo -create iOS/Build/Release-iphoneos/${lib} iOS/Build/Release-iphonesimulator/${lib} -output Assembled/ios/libs/${lib}`)));
};

const copyAndroidFiles = async () => {
  await runCommand('mkdir -p Assembled/android');
  await runCommand('cp Android/build.gradle Assembled/android');

  await runCommand('mkdir -p Assembled/android/src/main');
  await runCommand('cp ../Apps/Playground/node_modules/@babylonjs/react-native/android/src/main/AndroidManifest.xml Assembled/android/src/main');

  await runCommand('mkdir -p Assembled/android/src/main/java');
  await runCommand('rsync -a ../Apps/Playground/node_modules/@babylonjs/react-native/android/src/main/java/ Assembled/android/src/main/java/');
  await runCommand('rsync -a ../Apps/Playground/node_modules/@babylonjs/react-native/android/build/intermediates/library_and_local_jars_jni/release/jni/ Assembled/android/src/main/jniLibs/');
};

const copyFiles = gulp.parallel(copyCommonFiles, copyIOSFiles, copyAndroidFiles);

const build = gulp.series(buildIOS, buildAndroid, createIOSUniversalLibs, copyFiles);

exports.buildIOS = buildIOS;
exports.buildAndroid = buildAndroid;
exports.createIOSUniversalLibs = createIOSUniversalLibs;
exports.copyFiles = copyFiles;

exports.clean = clean;
exports.build = build;
exports.rebuild = gulp.series(clean, build);

exports.default = build;