const shelljs = require('shelljs');
const output = shelljs.exec('npm list -g npm');
const parts = output.split("npm@");
const version = parts[1].trim();

// Check if using npm version 7+
if (version.charAt(0) == '7')
{
  throw `Error: BabylonReactNative Playground development requires npm version 6.13.*, Your current npm version is ${version}. Run npm install -g npm@6.13 to update your npm version.`;
}