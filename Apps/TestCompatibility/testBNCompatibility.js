const fs = require('fs');
const shelljs = require('shelljs');

let compatibility = [];
function execute(command, workingDirectory, callback){
    const res = shelljs.exec(command, { fatal: false, cwd: workingDirectory, async: false });
    callback(res.code, res.stdout, res.stderr);
};

function copy(source, destination) {
    fs.copyFileSync(source, destination);
    console.log(`${source} was copied to ${destination}`);
}

const protocolChecker = "//BRNCHECK\r\nconsole.log(' * Checking Babylon.js / BabylonNative protocols');\r\n"+
    "if (_native.Engine.PROTOCOL_VERSION !== BABYLON.NativeEngine.PROTOCOL_VERSION) {\r\n"+
    "console.log(` * Protocol version mismatch: ${_native.Engine.PROTOCOL_VERSION} (Native) !== ${BABYLON.NativeEngine.PROTOCOL_VERSION} (JS)`);\r\n"+
    "TestUtils.exit(-1); } else { console.log(' * Protocol version OK! '); }\r\n"+
    "console.error = function(errorMsg) { console.log(errorMsg); TestUtils.exit(-1);}\r\n";

const exeFolder = './BabylonNative/build/Apps/ValidationTests/Release';
const filesToCopy = [
    {source:'babylonjs', files:['babylon.max.js', 'babylon.max.js.map']},
    {source:'babylonjs-gui', files:['babylon.gui.js', 'babylon.gui.js.map']},
    {source:'babylonjs-loaders', files:['babylonjs.loaders.js', 'babylonjs.loaders.js.map']},
    {source:'babylonjs-materials', files:['babylonjs.materials.js', 'babylonjs.materials.js.map']}];

const BRNVersions = [
    {tag:'1.4.3', hash:'301ab90'},
    {tag:'1.4.4', hash:'75954f4'},
    {tag:'1.5.0', hash:'75954f4'},
    {tag:'1.5.1', hash:'a2cf1c7'},
    {tag:'1.6.0', hash:'409bc4b'}];

function patchTestScript() {
    const filePath = `${exeFolder}/Scripts/validation_native.js`;
    console.log("Patching test script.");
    const testScript = fs.readFileSync(filePath);
    if (testScript.includes("//BRNCHECK")) {
        // already patched
        return;
    }
    const newContent = protocolChecker + testScript;
    fs.writeFileSync(filePath, newContent);
}

function checkoutAndBuildBN(tag, hash, callback) {
    console.log("Git clone.");
    execute('git clone https://github.com/BabylonJS/BabylonNative.git', './', (error, stdout, stderr) => {
        console.log("Submodule update.");
        execute('git submodule update --init --recursive', './BabylonNative', (error, stdout, stderr) => {
            //if (error) throw error;
            console.log(`Checkout tag ${hash}.`);
            execute(`git checkout ${hash}`, "./BabylonNative", (error, stdout, stderr) => {
                if (error) throw error;
                execute('git submodule update --recursive', './BabylonNative', (error, stdout, stderr) => {
                    console.log('Making build directory.');
                    fs.rmSync('./BabylonNative/build', { recursive: true, force: true });
                    execute(`mkdir build`, "./BabylonNative", (error, stdout, stderr) => {
                        if (error) throw error;
                        execute(`npm install`, "./BabylonNative/Apps", (error, stdout, stderr) => {
                            if (error) throw error;
                            console.log('Building win32 project.');

                            execute(`cmake -G "Visual Studio 16 2019" -A x64 ..`, "./BabylonNative/build", (error, stdout, stderr) => {
                                if (error) throw error;
                                console.log('Building win32 apps.');
                                execute(`cmake --build build --config Release`, "./BabylonNative", (error, stdout, stderr) => {
                                    if (error) throw error;
                                    patchTestScript();
                                    callback(tag, hash);
                                });
                            });
                        });
                    });
                });
            });
        });
    });
}

function testPackages(tag, hash) {
    let compatiblePackageVersions = [];

    let bjsMask = '5.*.*';
    const [bjsVersion] = process.argv.slice(2);
    if (bjsVersion != '') {
        bjsMask = bjsVersion;
    }

    console.log("Getting NPM versions ...");
    execute(`npm show babylonjs@${bjsMask} version --json`, "./", (error, stdout, stderr) => {
        if (error) throw error;
    
        const versions = JSON.parse(stdout);
    
        versions.forEach(version => {
            console.log("-".repeat(30));
            console.log(`Installing packages for ${version}`);
            execute(`npm install babylonjs@${version} babylonjs-gui@${version} babylonjs-loaders@${version} babylonjs-materials@${version}`, "./", (error, stdout, stderr) => {
                if (error) throw error;
                    
                // copy package .js
                console.log("Copying packages files.");
                filesToCopy.forEach(entry => {
                    entry.files.forEach(file => {
                        copy(`node_modules/${entry.source}/${file}`, `${exeFolder}/Scripts/${file}`);
                    });
                });
    
                // run validation test
                console.log("Running validation tests.");
                execute("ValidationTests", exeFolder, (error, stdout, stderr) => {
                    console.log(`Error Code ${error}`);
                    if (!!error) {
                        console.log(`${version} Failed!`);
                    } else {
                        compatiblePackageVersions.push(version);
                        console.log(`${version} OK!`);
                    }
                });
            });
        });
    });
    console.log(`Compatible versions for BabylonReactNative ${tag} using BabylonNative ${hash}:`, compatiblePackageVersions);
    compatibility.push({tag:tag, hash:hash, npms:compatiblePackageVersions});
}

BRNVersions.forEach((versionToTest) =>{
    checkoutAndBuildBN(versionToTest.tag, versionToTest.hash,(tag, hash) => { testPackages(tag, hash); });
});

// and out!
fs.writeFileSync("compatibility.json", JSON.stringify(compatibility));