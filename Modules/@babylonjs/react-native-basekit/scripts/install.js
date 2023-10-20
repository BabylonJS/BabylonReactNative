const https = require('https');
const zlib = require('zlib');
const tar = require('tar');

function getArgument(name) {
  const flags = process.argv.slice(2), index = flags.lastIndexOf(name);

  if (index === -1 || index + 1 >= flags.length) {
    return null;
  }

  return flags[index + 1];
}

function getBinaryUrl(package, binaryFilename) {
  var site = getArgument('--brn-binary-site') ||
              process.env.BRN_BINARY_SITE  ||
              process.env.npm_config_brn_binary_site ||
              (package.nodeBabylonConfig && package.nodeBabylonConfig.binarySite) ||
              'https://github.com/CedricGuillemet/BabylonReactNative-1/releases/download';

  return [site, /*'v' +*/ package.version, binaryFilename].join('/');
}

const downloadAndExtract = (url) => {
  const options = {
    followRedirects: true, // Follow HTTP 3xx redirects
  };

  https.get(url, options, (response) => {
    if ([301, 302, 303, 307, 308].includes(response.statusCode)) {
      // If the response is a redirect, recursively call downloadAndExtract with the new URL
      downloadAndExtract(response.headers.location);
    } else if (response.statusCode === 200) {
      const gunzip = zlib.createGunzip();
      const untar = tar.extract({ cwd: process.cwd() }); // Extract to the current working directory

      response
        .pipe(gunzip) // Unzip the response
        .pipe(untar)  // Extract the tar file
        .on('finish', () => {
          //done
        })
        .on('error', (err) => {
          console.error('Error extracting the tar.gz file:', err);
          process.exit(1);
        });
    } else {
      console.error('Failed to download the file. Status code:', response.statusCode);
      process.exit(1);
    }
  })
  .on('error', (err) => {
    console.error('Error downloading the file:', err);
    process.exit(1);
  });
};

function Install() {
  // get Babylon React Native package version
  const packageJsonPath = process.env.npm_package_json;
  const packageJson = require(packageJsonPath);
  if (!packageJson.version) {
    console.err("Babylon React Native version not found");
    process.exit(1);
  }
  // get user project react-native version
  const projectPackageJsonPath = process.env.npm_config_local_prefix + '/package.json';

  // Read the project's package.json file
  const projectPackageJson = require(projectPackageJsonPath);
  // Check if the project's package.json has a 'react-native' dependency
  if (projectPackageJson.dependencies && projectPackageJson.dependencies['react-native']) {
    const reactNativeVersion = projectPackageJson.dependencies['react-native'];
    console.log(`Detected React Native version: ${reactNativeVersion}`);
    reactNativePostfix = "";
    if (reactNativeVersion >= '0.71') {
      reactNativePostfix = "0.71";
    } else if (reactNativeVersion >= '0.70') {
      reactNativePostfix = "0.70";
    } else if (reactNativeVersion >= '0.69') {
      reactNativePostfix = "0.69";
    } else {
      console.error("Unsupported react native version.");
      process.exit(1);
    }
    console.log(`Downloading Babylon React Native version ${reactNativePostfix} from Package version ${packageJson.version}.`);

    const reactNative = getBinaryUrl(packageJson, 'react-native.tar.gz');
    const reactNativeiOSAndroid = getBinaryUrl(packageJson, `iOSAndroid${reactNativePostfix}.tar.gz`);

    // Start the download and extraction process
    downloadAndExtract(reactNative);
    downloadAndExtract(reactNativeiOSAndroid);

    // check and download Windows binary if react-native-windows is found in project package
    const reactNativeWindowsVersion = projectPackageJson.dependencies['react-native-windows'];
    if (reactNativeWindowsVersion) {
      console.log("react-native-windows detected.");
      const reactNativeiOSWindows = getBinaryUrl(packageJson, `Windows${reactNativePostfix}.tar.gz`);
      downloadAndExtract(reactNativeiOSWindows);
    }
  } else {
    console.error("No react-native version found for BabylonReactNative.");
    process.exit(1);
  }
}

Install();