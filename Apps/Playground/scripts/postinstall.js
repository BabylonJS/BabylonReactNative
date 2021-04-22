const shelljs = require('shelljs');
let output = shelljs.exec('npm list -g npm');

if (!output.includes("npm@")) {
    shelljs.echo("global npm install not found");
    output = shelljs.exec('npm list npm');
}

if (!output.includes("npm@")) {
    shelljs.echo("local npm version not found");

}
else {
    const parts = output.split("npm@");
    const version = parts[1].trim();

    // Check if using npm version 7+
    if (version.charAt(0) == '7') {
        throw `Error: BabylonReactNative Playground development requires npm version 6.13.*, Your current npm version is ${version}. Run npm install -g npm@6.13 to update your npm version.`;
    }
}