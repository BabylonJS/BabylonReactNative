const util = require('util');
const fs = require('fs');
const path = require('path');
const readdirAsync = util.promisify(fs.readdir);
const log = require('fancy-log');
const gulp = require('gulp');
const shelljs = require('shelljs');
const rename = require('gulp-rename');
const glob = require('glob');
const chalk = require('chalk');

const unzipper = require('unzipper');
const os = require('os');

let assembledWindowsDir = 'Assembled-Windows';
let basekitBuild = false;
let cmakeBasekitBuildDefinition = '';

function exec(command, workingDirectory = '.', logCommand = true) {
  if (logCommand) {
    log(command);
  }

  if (shelljs.exec(command, { fatal: true, cwd: workingDirectory }).code) {
    throw `'${command}' finished with non-zero exit code.`;
  }
}

function checkDirectory(actualList, expectedList, directoryName) {
  const extras = actualList.filter(path => !expectedList.includes(path));
  const missing = expectedList.filter(path => !actualList.includes(path));

  let isValid = true;

  if (extras.length !== 0) {
    console.error(chalk.white.bgRedBright(`The ${directoryName} directory contains unexpected files:`));
    console.log(extras);
    isValid = false;
  }

  if (missing.length !== 0) {
    console.error(chalk.white.bgRedBright(`The ${directoryName} directory is missing some expected files:`));
    console.log(missing);
    isValid = false;
  }

  if (!isValid) {
    console.log(chalk.black.bgCyan(`If the ${directoryName} directory is correct, update the file validation list in gulpfile.js with the following:`))
    console.log(actualList);
    throw `The ${directoryName} directory does not contain the expected files.`;
  }
}

const clean = async () => {
  if (shelljs.test('-d', 'Assembled')) {
    shelljs.rm('-r', 'Assembled');
  }

  if (shelljs.test('-d', `${assembledWindowsDir}`)) {
    shelljs.rm('-r', `${assembledWindowsDir}`);
  }
};

const buildTypeScript = async () => {
  exec('node_modules/typescript/bin/tsc --noEmit false --outDir ../../../Package/Assembled', '../Modules/@babylonjs/react-native');

  // Update the 'main' property in package.json to be 'index.js' instead of 'index.ts'
  const packageJson = JSON.parse(fs.readFileSync('Assembled/package.json'));
  packageJson.main = `${path.basename(packageJson.main, '.ts')}.js`;
  fs.writeFileSync('Assembled/package.json', JSON.stringify(packageJson, null, 4));
};

const makeXCodeProj = async () => {
  exec(`cmake react-native/ios -G Xcode -DBUILD_RNAPP_DIR=Playground -B Build/iOS -DFETCHCONTENT_FULLY_DISCONNECTED=ON -DBABYLON_NATIVE_BUILD_APPS=OFF -DBABYLON_NATIVE_INSTALL=OFF -DFETCHCONTENT_SOURCE_DIR_BGFX.CMAKE=./react-native/shared/BabylonNative/deps/bgfx.cmake-src -DFETCHCONTENT_SOURCE_DIR_ARCANA.CPP=./react-native/shared/BabylonNative/deps/arcana.cpp-src -DFETCHCONTENT_SOURCE_DIR_CMAKEEXTENSIONS=./react-native/shared/BabylonNative/deps/cmakeextensions-src -DFETCHCONTENT_SOURCE_DIR_JSRUNTIMEHOST=./react-native/shared/BabylonNative/deps/jsruntimehost-src -DFETCHCONTENT_SOURCE_DIR_URLLIB=./react-native/shared/BabylonNative/deps/urllib-src -DFETCHCONTENT_SOURCE_DIR_SPIRV-CROSS=./react-native/shared/BabylonNative/deps/spirv-cross-src -DFETCHCONTENT_SOURCE_DIR_LIBWEBP=./react-native/shared/BabylonNative/deps/libwebp-src -DFETCHCONTENT_SOURCE_DIR_GLSLANG=./react-native/shared/BabylonNative/deps/glslang-src -DFETCHCONTENT_SOURCE_DIR_BASE-N=./react-native/shared/BabylonNative/deps/base-n-src -DFETCHCONTENT_SOURCE_DIR_IOS-CMAKE=./react-native/shared/BabylonNative/deps/ios-cmake-src`, 
    '../Modules/@babylonjs');
};

const buildIphoneOS = async () => {
  exec('xcodebuild -sdk iphoneos -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO', '../Modules/@babylonjs/Build/iOS');
};

const buildIphoneSimulator = async () => {
  exec('xcodebuild -sdk iphonesimulator -arch x86_64 -configuration Release -project ReactNativeBabylon.xcodeproj -scheme BabylonNative build CODE_SIGNING_ALLOWED=NO', '../Modules/@babylonjs/Build/iOS');
};

const buildAndroid = async () => {
  const basekitBuildProp = basekitBuild ? "-PBASEKIT_BUILD=1" : "";
  exec(`./gradlew babylonjs_react-native:assembleRelease --stacktrace --info ${basekitBuildProp}`, '../Apps/Playground/android');
};

const makeUWPProjectPlatform = async (name, arch) => {
  shelljs.mkdir('-p', `./../Modules/@babylonjs/react-native/Build/uwp_${name}`);
  exec(`cmake -G "Visual Studio 16 2019" -D CMAKE_SYSTEM_NAME=WindowsStore -D CMAKE_SYSTEM_VERSION=10.0 -DCMAKE_UNITY_BUILD=true ${cmakeBasekitBuildDefinition} -A ${arch} ./../../../react-native/windows`, `./../Modules/@babylonjs/react-native/Build/uwp_${name}`);
};

const makeUWPProjectx86 = async () => makeUWPProjectPlatform('x86', 'Win32');
const makeUWPProjectx64 = async () => makeUWPProjectPlatform('x64', 'x64');
const makeUWPProjectARM64 = async () => makeUWPProjectPlatform('arm64', 'arm64');

const makeUWPProject = gulp.parallel(
  makeUWPProjectx86,
  makeUWPProjectx64,
  makeUWPProjectARM64
);

const buildUWPx86Debug = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\Build.bat -Platform Win32 -Configuration Debug');
}

const buildUWPx86Release = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\Build.bat -Platform Win32 -Configuration Release');
}

const buildUWPx64Debug = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\Build.bat -Platform x64 -Configuration Debug');
}

const buildUWPx64Release = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\Build.bat -Platform x64 -Configuration Release');
}

const buildUWPARM64Debug = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\Build.bat -Platform ARM64 -Configuration Debug');
}

const buildUWPARM64Release = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\Build.bat -Platform ARM64 -Configuration Release');
}

const buildUWPProject = gulp.parallel(
  buildUWPx86Debug,
  buildUWPx86Release,
  buildUWPx64Debug,
  buildUWPx64Release,
  buildUWPARM64Debug,
  buildUWPARM64Release
);

const nugetRestoreUWPPlayground = async () => {
  exec('nuget restore Playground.sln', './../Apps/Playground/Playground/windows');
}

const buildUWPPlaygroundx86Debug = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\BuildPlayground.bat -Platform x86 -Configuration Debug');
}

const buildUWPPlaygroundx86Release = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\BuildPlayground.bat -Platform x86 -Configuration Release');
}

const buildUWPPlaygroundx64Debug = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\BuildPlayground.bat -Platform x64 -Configuration Debug');
}

const buildUWPPlaygroundx64Release = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\BuildPlayground.bat -Platform x64 -Configuration Release');
}

const buildUWPPlaygroundARM64Debug = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\BuildPlayground.bat -Platform ARM64 -Configuration Debug');
}

const buildUWPPlaygroundARM64Release = async () => {
  exec('.\\..\\Modules\\@babylonjs\\react-native\\windows\\scripts\\BuildPlayground.bat -Platform ARM64 -Configuration Release');
}

const buildUWPPlayground = gulp.parallel(
  buildUWPPlaygroundx86Debug,
  buildUWPPlaygroundx86Release,
  buildUWPPlaygroundx64Debug,
  buildUWPPlaygroundx64Release,
  buildUWPPlaygroundARM64Debug,
  buildUWPPlaygroundARM64Release
);

const buildUWP = gulp.series(makeUWPProject, buildUWPProject);

const copyCommonFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/README.md')
    .pipe(gulp.src('../NOTICE.html'))
    .pipe(gulp.src(`../Modules/@babylonjs/react-native/react-native-babylon.podspec`))
    .pipe(gulp.dest('Assembled'));
};

const copySharedFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/shared/**')
    .pipe(gulp.dest('Assembled/shared'));
};

const copyIOSFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/ios/**')
    .pipe(gulp.dest(`Assembled/ios`));
};

const copyAndroidFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/android/**')
    .pipe(gulp.dest(`Assembled/android`));
};

const copyWindowsFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/windows/**')
    .pipe(gulp.dest(`Assembled/windows`));
};

const createUWPDirectories = async () => {
  shelljs.mkdir('-p', `${assembledWindowsDir}`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/arm64`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/arm64/Debug`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/arm64/Release`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/x86`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/x86/Debug`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/x86/Release`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/x64`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/x64/Debug`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/libs/x64/Release`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/BabylonReactNative`);
  shelljs.mkdir('-p', `${assembledWindowsDir}/windows/include`);
}

const copyCommonFilesUWP = () => {
  return gulp.src('../Modules/@babylonjs/react-native/package.json')
    .pipe(gulp.src('../Modules/@babylonjs/react-native/README.md'))
    .pipe(gulp.src('../NOTICE.html'))
    .pipe(gulp.src('../Modules/@babylonjs/react-native/*.ts*'))
    .pipe(gulp.dest(`${assembledWindowsDir}`));
}

const copyx86DebugUWPFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/Build/uwp_x86/**/Debug/**/*.{lib,pri}')
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/libs/x86/Debug`));
}

const copyx86ReleaseUWPFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/Build/uwp_x86/**/Release/**/*.{lib,pri}')
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/libs/x86/Release`));
}

const copyx64DebugUWPFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/Build/uwp_x64/**/Debug/**/*.{lib,pri}')
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/libs/x64/Debug`));
}

const copyx64ReleaseUWPFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/Build/uwp_x64/**/Release/**/*.{lib,pri}')
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/libs/x64/Release`));
}

const copyARM64DebugUWPFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/Build/uwp_arm64/**/Debug/**/*.{lib,pri}')
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/libs/arm64/Debug`));
}

const copyARM64ReleaseUWPFiles = () => {
  return gulp.src('../Modules/@babylonjs/react-native/Build/uwp_arm64/**/Release/**/*.{lib,pri}')
    .pipe(rename({ dirname: '' }))
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/libs/arm64/Release`));
}

const copyVCXProjUWPFiles = () => {
  const uwpFilesDir = '../Modules/@babylonjs/react-native/windows/BabylonReactNative';
  return gulp.src([`${uwpFilesDir}/*.*`, `!${uwpFilesDir}/*.pfx`])
    .pipe(gulp.dest(`${assembledWindowsDir}/windows/BabylonReactNative`));
}

const copyUWPFiles = gulp.series(
  createUWPDirectories,
  basekitBuild ? 
  gulp.parallel(
    copyCommonFilesUWP,
    copyx86DebugUWPFiles,
    copyx86ReleaseUWPFiles,
    copyx64DebugUWPFiles,
    copyx64ReleaseUWPFiles,
    copyARM64DebugUWPFiles,
    copyARM64ReleaseUWPFiles,
    copyVCXProjUWPFiles)
    :
  gulp.parallel(
    copyCommonFilesUWP,
    copyx86DebugUWPFiles,
    copyx86ReleaseUWPFiles,
    copyx64DebugUWPFiles,
    copyx64ReleaseUWPFiles,
    copyARM64DebugUWPFiles,
    copyARM64ReleaseUWPFiles,
    copyVCXProjUWPFiles));

const validateAssembled = async () => {
  // When the package contents are updated *and validated*, update the expected below from the output of the failed validation console output (run `gulp validateAssembled`).
  // This helps ensure a bad package is not accidentally published due to tooling changes, etc.
  const expected = [
    'Assembled/BabylonModule.d.ts',
    'Assembled/BabylonModule.js',
    'Assembled/BabylonModule.js.map',
    'Assembled/EngineHook.d.ts',
    'Assembled/EngineHook.js',
    'Assembled/EngineHook.js.map',
    'Assembled/EngineView.d.ts',
    'Assembled/EngineView.js',
    'Assembled/EngineView.js.map',
    'Assembled/index.d.ts',
    'Assembled/index.js',
    'Assembled/index.js.map',
    'Assembled/NativeCapture.d.ts',
    'Assembled/NativeCapture.js',
    'Assembled/NativeCapture.js.map',
    'Assembled/NativeEngineHook.d.ts',
    'Assembled/NativeEngineHook.js',
    'Assembled/NativeEngineHook.js.map',
    'Assembled/NativeEngineView.d.ts',
    'Assembled/NativeEngineView.js',
    'Assembled/NativeEngineView.js.map',
    'Assembled/FontFace.d.ts',
    'Assembled/FontFace.js',
    'Assembled/FontFace.js.map',
    'Assembled/package.json',
    'Assembled/ReactNativeEngine.d.ts',
    'Assembled/ReactNativeEngine.js',
    'Assembled/ReactNativeEngine.js.map',
    'Assembled/README.md',
    'Assembled/NOTICE.html',
    'Assembled/shared',
    'Assembled/shared/BabylonNative.h',
    'Assembled/shared/BabylonNative.cpp',
    'Assembled/shared/BabylonNative',
    'Assembled/shared/XrAnchorHelper.h',
    'Assembled/shared/XrContextHelper.h',
    'Assembled/VersionValidation.d.ts',
    'Assembled/VersionValidation.js',
    'Assembled/VersionValidation.js.map',
    `Assembled/android`,
    `Assembled/android/build.gradle`,
    `Assembled/android/CMakeLists.txt`,
    `Assembled/android/README.md`,
// TODO: Is following needed ?
//    `Assembled/android/include`, 
//    `Assembled/android/include/IXrContextARCore.h`,
    `Assembled/android/src`,
    `Assembled/android/src/main`,
    `Assembled/android/src/main/AndroidManifest.xml`,
    `Assembled/android/src/main/java`,
    `Assembled/android/src/main/java/com`,
    `Assembled/android/src/main/java/com/babylonreactnative`,
    `Assembled/android/src/main/java/com/babylonreactnative/BabylonModule.java`,
    `Assembled/android/src/main/java/com/babylonreactnative/BabylonNativeInterop.java`,
    `Assembled/android/src/main/java/com/babylonreactnative/BabylonPackage.java`,
    `Assembled/android/src/main/java/com/babylonreactnative/EngineView.java`,
    `Assembled/android/src/main/java/com/babylonreactnative/EngineViewManager.java`,
    `Assembled/android/src/main/java/com/babylonreactnative/SnapshotDataReturnedEvent.java`,
    `Assembled/ios`,
    `Assembled/ios/BabylonModule.mm`,
    `Assembled/ios/BabylonNativeInterop.h`,
    `Assembled/ios/BabylonNativeInterop.mm`,
    `Assembled/ios/EngineViewManager.mm`,
// TODO: Is following needed ?
//    `Assembled/ios/include`,
//    `Assembled/ios/include/IXrContextARKit.h`,
    `Assembled/package.json`,
    `Assembled/react-native-babylon.podspec`,

    'Assembled/android/src/main/cpp',
    'Assembled/android/src/main/cpp/BabylonNativeInterop.cpp',
    'Assembled/ios/CMakeLists.txt',
    'Assembled/shared/BabylonNative/CMakeLists.txt',
    'Assembled/shared/CMakeLists.txt',
    'Assembled/shared/DispatchFunction.h',
    'Assembled/windows',
    'Assembled/windows/BabylonReactNative',
    'Assembled/windows/BabylonReactNative/BabylonModule.cpp',
    'Assembled/windows/BabylonReactNative/BabylonModule.h',
    'Assembled/windows/BabylonReactNative/BabylonReactNative_TemporaryKey.pfx',
    'Assembled/windows/BabylonReactNative/BabylonReactNative.def',
    'Assembled/windows/BabylonReactNative/BabylonReactNative.rc',
    'Assembled/windows/BabylonReactNative/BabylonReactNative.vcxproj',
    'Assembled/windows/BabylonReactNative/BabylonReactNative.vcxproj.filters',
    'Assembled/windows/BabylonReactNative/EngineView.cpp',
    'Assembled/windows/BabylonReactNative/EngineView.h',
    'Assembled/windows/BabylonReactNative/EngineView.idl',
    'Assembled/windows/BabylonReactNative/EngineViewManager.cpp',
    'Assembled/windows/BabylonReactNative/EngineViewManager.h',
    'Assembled/windows/BabylonReactNative/packages.config',
    'Assembled/windows/BabylonReactNative/pch.cpp',
    'Assembled/windows/BabylonReactNative/pch.h',
    'Assembled/windows/BabylonReactNative/PropertySheet.props',
    'Assembled/windows/BabylonReactNative/ReactPackageProvider.cpp',
    'Assembled/windows/BabylonReactNative/ReactPackageProvider.h',
    'Assembled/windows/BabylonReactNative/ReactPackageProvider.idl',
    'Assembled/windows/BabylonReactNative/resource.h',
    'Assembled/windows/CMakeLists.txt'
  ];

  const actual = glob.sync('Assembled/**/*', {ignore: ['Assembled/shared/BabylonNative/BabylonNative-*/**', 'Assembled/shared/BabylonNative/deps/**']});
  checkDirectory(actual, expected, `Assembled`);
}

const createPackage = async () => {
  exec('npm pack', 'Assembled');
};

const COMMIT_ID = 'a736d2d675c4733e70186237d39412f187139b48';
const ZIP_URL = `https://github.com/CedricGuillemet/BabylonNative/archive/${COMMIT_ID}.zip`;
const TARGET_DIR = path.resolve(__dirname, '../Modules/@babylonjs/react-native/shared/BabylonNative');
const ZIP_PATH = path.join(TARGET_DIR, `${COMMIT_ID}.zip`);
const UNZIP_FOLDER = path.join(TARGET_DIR, `BabylonNative-${COMMIT_ID}`);
const CMAKE_LISTS_PATH = path.join(TARGET_DIR, 'CMakeLists.txt');
const TEMP_BUILD_DIR = path.join(TARGET_DIR, 'tempBuild');
const DEPS_OUTPUT_DIR = path.join(TARGET_DIR, 'deps');

async function downloadZip(url, dest) {
  const res = await fetch(url);
  if (!res.ok) {
    throw new Error(`Failed to download zip: ${res.status} ${res.statusText}`);
  }

  const fileStream = fs.createWriteStream(dest);
  const reader = res.body.getReader();

  return new Promise((resolve, reject) => {
    function pump() {
      reader.read().then(({ done, value }) => {
        if (done) {
          fileStream.end();
          resolve();
          return;
        }
        fileStream.write(Buffer.from(value), pump);
      }).catch(reject);
    }

    fileStream.on('error', reject);
    pump();
  });
}

async function unzipFile(zipPath, destDir) {
  await fs.createReadStream(zipPath)
    .pipe(unzipper.Extract({ path: destDir }))
    .promise();
}

function deleteFile(filePath) {
  return fs.promises.unlink(filePath);
}

function runCMake(buildDir) {
  let cmakeCommand = `cmake -S . -B ../tempBuild -DBABYLON_NATIVE_BUILD_SOURCETREE=ON -DBABYLON_NATIVE_BUILD_APPS=OFF`;

  exec(cmakeCommand, buildDir);
}

function writeCMakeListsFile(commitId, cmakePath) {
  const content = `add_subdirectory(\${CMAKE_CURRENT_LIST_DIR}/BabylonNative-${commitId})\n`;
  fs.writeFileSync(cmakePath, content, 'utf8');
}

function deleteFolderRecursive(folderPath, excludeSubFolders = []) {
  if (!fs.existsSync(folderPath)) return;

  const entries = fs.readdirSync(folderPath, { withFileTypes: true });

  for (const entry of entries) {
    const fullPath = path.join(folderPath, entry.name);

    if (entry.isDirectory()) {
      if (excludeSubFolders.includes(entry.name)) {
        continue; // Skip this folder
      }
      deleteFolderRecursive(fullPath, excludeSubFolders);
    } else {
      fs.unlinkSync(fullPath);
    }
  }

  // After deleting contents, delete the root folder if it's not in the excluded list
  const folderName = path.basename(folderPath);
  if (!excludeSubFolders.includes(folderName)) {
    const remaining = fs.readdirSync(folderPath);
    if (remaining.length === 0) {
      fs.rmdirSync(folderPath);
      console.log(`Deleted folder: ${folderPath}`);
    }
  }
}

function copyRecursiveExcludingGit(src, dest) {
  if (!fs.existsSync(src)) return;

  fs.mkdirSync(dest, { recursive: true });
  const entries = fs.readdirSync(src, { withFileTypes: true });

  for (const entry of entries) {
    if (entry.name === '.git') continue;

    const srcPath = path.join(src, entry.name);
    const destPath = path.join(dest, entry.name);

    if (entry.isDirectory()) {
      copyRecursiveExcludingGit(srcPath, destPath);
    } else {
      fs.copyFileSync(srcPath, destPath);
    }
  }
}

function copyDepsFolders() {
  const depsSrc = path.join(TEMP_BUILD_DIR, '_deps');
  if (!fs.existsSync(depsSrc)) return;

  fs.mkdirSync(DEPS_OUTPUT_DIR, { recursive: true });

  const entries = fs.readdirSync(depsSrc, { withFileTypes: true });

  for (const entry of entries) {
    if (entry.isDirectory() && entry.name.endsWith('-src')) {
      const fullSrcPath = path.join(depsSrc, entry.name);
      const fullDestPath = path.join(DEPS_OUTPUT_DIR, entry.name);
      console.log(`Copying ${entry.name} to deps/`);
      copyRecursiveExcludingGit(fullSrcPath, fullDestPath);
    }
  }
}
const buildBabylonNativeSourceTree = async () => {
  fs.mkdirSync(TARGET_DIR, { recursive: true });

  console.log(`Downloading BabylonNative commit ${COMMIT_ID}...`);
  await downloadZip(ZIP_URL, ZIP_PATH);

  console.log('Unzipping...');
  await unzipFile(ZIP_PATH, TARGET_DIR);

  console.log('Deleting ZIP...');
  await deleteFile(ZIP_PATH);

  console.log('Creating CMakeLists.txt...');
  writeCMakeListsFile(COMMIT_ID, CMAKE_LISTS_PATH);

  console.log('Running CMake...');
  await runCMake(UNZIP_FOLDER);

  console.log('Copying *-src folders to deps/ (excluding .git)...');
  copyDepsFolders();

  console.log('Deleting tempBuild folder...');
  deleteFolderRecursive(TEMP_BUILD_DIR);

  console.log('Remove unnecessary sources');
  deleteFolderRecursive(`${UNZIP_FOLDER}/.github`);
  deleteFolderRecursive(`${UNZIP_FOLDER}/Apps`);
  deleteFolderRecursive(`${UNZIP_FOLDER}/Documentation`);
  deleteFolderRecursive(`${UNZIP_FOLDER}/Install`);

  // dependencies cleanup
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/bgfx.cmake-src/bgfx/3rdparty`, ['renderdoc']);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/bgfx.cmake-src/bgfx/examples`,['common']);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/bgfx.cmake-src/bgfx/bindings`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/bgfx.cmake-src/bgfx/docs`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/bgfx.cmake-src/bgfx/scripts`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/bgfx.cmake-src/bgfx/tools`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/spirv-cross-src/reference`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/spirv-cross-src/shaders`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/spirv-cross-src/shaders-msl`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/spirv-cross-src/shaders-hlsl`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/spirv-cross-src/shaders-hlsl-no-opt`);
  deleteFolderRecursive(`${DEPS_OUTPUT_DIR}/glslang-src/Test`);
 
}

const copyFiles = gulp.parallel(copyCommonFiles, copySharedFiles, copyIOSFiles, copyAndroidFiles, copyWindowsFiles);
const buildAssembled = gulp.series(buildBabylonNativeSourceTree, copyFiles, buildTypeScript, validateAssembled);
const buildIOS = gulp.series(makeXCodeProj,  buildIphoneOS, buildIphoneSimulator);

exports.buildAssembled = buildAssembled;
exports.buildTypeScript = buildTypeScript;
exports.validateAssembled = validateAssembled;
exports.buildIOS = buildIOS;
exports.buildAndroid = buildAndroid;
exports.copyFiles = copyFiles;

exports.clean = clean;
/*exports.build = build;
exports.rebuild = rebuild;
exports.pack = pack;*/

const copyPackageFilesUWP = gulp.series(copyUWPFiles);
const buildUWPPublish = gulp.series(buildUWP, copyPackageFilesUWP, buildUWP, copyPackageFilesUWP);
const packUWP = gulp.series(clean, buildUWP, copyPackageFilesUWP, createPackage);
const packUWPNoBuild = gulp.series(clean, copyPackageFilesUWP, createPackage);

//exports.buildTS = buildTS;
exports.makeUWPProjectx86 = makeUWPProjectx86;
exports.makeUWPProjectx64 = makeUWPProjectx64;
exports.makeUWPProjectARM64 = makeUWPProjectARM64;
exports.makeUWPProject = makeUWPProject;

exports.buildUWPx86Debug = buildUWPx86Debug;
exports.buildUWPx86Release = buildUWPx86Release;
exports.buildUWPx64Debug = buildUWPx64Debug;
exports.buildUWPx64Release = buildUWPx64Release;
exports.buildUWPARM64Debug = buildUWPARM64Debug;
exports.buildUWPARM64Release = buildUWPARM64Release;
exports.buildUWPProject = buildUWPProject;

exports.nugetRestoreUWPPlayground = nugetRestoreUWPPlayground;
exports.buildUWPPlaygroundx86Debug = buildUWPPlaygroundx86Debug;
exports.buildUWPPlaygroundx86Release = buildUWPPlaygroundx86Release;
exports.buildUWPPlaygroundx64Debug = buildUWPPlaygroundx64Debug;
exports.buildUWPPlaygroundx64Release = buildUWPPlaygroundx64Release;
exports.buildUWPPlaygroundARM64Debug = buildUWPPlaygroundARM64Debug;
exports.buildUWPPlaygroundARM64Release = buildUWPPlaygroundARM64Release;
exports.buildUWPPlayground = buildUWPPlayground;

exports.buildUWP = buildUWP;
exports.buildUWPPublish = buildUWPPublish;

exports.copyUWPFiles = copyUWPFiles;
exports.packUWP = packUWP;
exports.packUWPNoBuild = packUWPNoBuild;

exports.default = buildAssembled;


exports.buildBabylonNativeSourceTree = buildBabylonNativeSourceTree;