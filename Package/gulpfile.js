const util = require('util');
const fs = require('fs');
const spawn = require('child_process').spawn;
const readdirAsync = util.promisify(fs.readdir);
const log = require('fancy-log');
const del = require('del');
const gulp = require('gulp');
const parseArgsStringToArgv = require('string-argv').parseArgsStringToArgv;

function exec(command, options = {}, logCommand = true) {
  if (logCommand) {
    log(command);
  }

  const arguments = parseArgsStringToArgv(command);
  command = arguments.shift();

  return new Promise((resolve, reject) => {
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
};

const makeXCodeProj = async () => {
  await exec('mkdir -p iOS/Build');
  await exec('cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../../Apps/Playground/node_modules/@babylonjs/react-native/submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DDEPLOYMENT_TARGET=12 -DENABLE_GLSLANG_BINARIES=OFF -DSPIRV_CROSS_CLI=OFF ..', {cwd: 'iOS/Build'});
};

const buildIphoneOS = async () => {
  await exec('xcodebuild -sdk iphoneos -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO', {cwd: 'iOS/Build'});
};

const buildIphoneSimulator = async () => {
  await exec('xcodebuild -sdk iphonesimulator -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO', {cwd: 'iOS/Build'});
};

const buildIOS = gulp.series(makeXCodeProj, buildIphoneOS, buildIphoneSimulator);

const buildAndroid = async () => {
  await exec('./gradlew babylonjs_react-native:assembleRelease', {cwd: '../Apps/Playground/android'});
};

const copyCommonFiles = () => {
  return  gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/package.json')
    .pipe(gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/README.md'))
    .pipe(gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/*.ts*'))
    .pipe(gulp.src('react-native-babylon.podspec'))
    .pipe(gulp.dest('Assembled'));
};

const copyIOSFiles = () => {
  return  gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/ios/*.h')
    .pipe(gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/ios/*.mm'))
    // This xcodeproj is garbage that we don't need in the package, but `pod install` ignores the package if it doesn't contain at least one xcodeproj. 🤷🏼‍♂️
    .pipe(gulp.src('iOS/Build/ReactNativeBabylon.xcodeproj**/**/*'))
    .pipe(gulp.dest('Assembled/ios'));
};

const createIOSUniversalLibs = async () => {
  await exec('mkdir -p Assembled/ios/libs');
  const libs = await readdirAsync('iOS/Build/Release-iphoneos');
  await Promise.all(libs.map(lib => exec(`lipo -create iOS/Build/Release-iphoneos/${lib} iOS/Build/Release-iphonesimulator/${lib} -output Assembled/ios/libs/${lib}`)));
};

const copyAndroidFiles = async () => {
  await new Promise(resolve => {
          gulp.src('Android/build.gradle')
    .pipe(gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/android/src**/main/AndroidManifest.xml'))
    .pipe(gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/android/src**/main/java/**/*'))
    .pipe(gulp.dest('Assembled/android'))
    .on('end', resolve);
  });

  await new Promise(resolve => {
          gulp.src('../Apps/Playground/node_modules/@babylonjs/react-native/android/build/intermediates/library_and_local_jars_jni/release/jni/**/*')
    .pipe(gulp.dest('Assembled/android/src/main/jniLibs/'))
    .on('end', resolve);
  });
};

const createPackage = async () => {
  await exec('npm pack', {cwd: 'Assembled'});
};

const copyFiles = gulp.parallel(copyCommonFiles, copyIOSFiles, copyAndroidFiles);

const build = gulp.series(buildIOS, buildAndroid, createIOSUniversalLibs, copyFiles);
const rebuild = gulp.series(clean, build);
const pack = gulp.series(rebuild, createPackage);

exports.buildIOS = buildIOS;
exports.buildAndroid = buildAndroid;
exports.createIOSUniversalLibs = createIOSUniversalLibs;
exports.copyFiles = copyFiles;

exports.clean = clean;
exports.build = build;
exports.rebuild = rebuild;
exports.pack = pack;

exports.default = build;