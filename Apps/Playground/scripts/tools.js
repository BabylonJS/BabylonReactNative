const os = require('os');
const shelljs = require('shelljs');
const chalk = require('chalk');

function iosCmake() {
  console.log(chalk.black.bgCyan('Running CMake for iOS...'));
  shelljs.exec('cmake -G Xcode -DCMAKE_TOOLCHAIN_FILE=../submodules/BabylonNative/Dependencies/ios-cmake/ios.toolchain.cmake -DPLATFORM=OS64COMBINED -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DDEPLOYMENT_TARGET=12 -DENABLE_PCH=OFF .', {cwd: 'node_modules/@babylonjs/react-native-iosandroid/ios'});
}

function macosCmake() {
  console.log(chalk.black.bgCyan('Running CMake for macOS...'));
  shelljs.exec('cmake -G Xcode', {cwd: 'node_modules/@babylonjs/react-native-macos/macos'});
}

function postInstall() {
  const version = shelljs.exec('npm --version', {silent: true});

  console.log(chalk.black.bgCyan('Installing Babylon React Native npm packages...'));
  shelljs.exec('npm install --legacy-peer-deps', {cwd: '../../../Modules/@babylonjs/react-native'});

  console.log(chalk.black.bgCyan('Updating submodules...'));
  shelljs.exec('git submodule update --init --recursive', {cwd: '../../../'});

  if (os.platform() === 'darwin') {
    iosCmake();
    macosCmake();

    console.log(chalk.black.bgCyan('Installing iOS pods...'));
    shelljs.exec('pod install', {cwd: 'ios'});

    console.log(chalk.black.bgCyan('Installing macOS pods...'));
    shelljs.exec('pod install', {cwd: 'macos'});
  }
}

// First arg will be 'node', second arg will be 'tools.js'
const [command] = process.argv.slice(2);

if (command === 'postinstall') {
  postInstall();
} else if (command === 'iosCMake') {
  iosCmake();
} else if (command === 'macosCMake') {
  macosCmake();
} else {
  console.error(chalk.black.bgRedBright(`Unkown command: ${command}`));
  process.exit(1);
}