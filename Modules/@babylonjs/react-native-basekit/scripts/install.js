const https = require('https');
const zlib = require('zlib');
const tar = require('tar');
const path = require('path');
const fs = require('fs');

function getCachePathCandidates() {
  return [
    process.env.npm_config_sass_binary_cache,
    process.env.npm_config_cache,
  ].filter(function(_) { return _; });
}

function getCached(package, binaryFilename) {
  return [getCachePathCandidates(), 'BabylonReactNative', package.version, binaryFilename].join('/');
}

function useCachedFile(filePath) {
  if (fs.existsSync(filePath)) {
    console.log(`Using cached file ${filePath}`);
    const gunzip = zlib.createGunzip();
    const untar = tar.extract({ cwd: process.cwd() }); // Extract to the current working directory

    const sourceStream = fs.createReadStream(filePath); 
    sourceStream
    .pipe(gunzip) // Decompress the .tar.gz file
    .pipe(untar) // Extract the tar contents to the cwd
    .on('error', (err) => {
      console.error('Error extracting .tar.gz file:', err);
      process.exit(1);
    })
    .on('end', () => {
      console.log('Extraction completed successfully.');
    });
    return true;
  }
  return false;
}

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

  return [site, package.version, binaryFilename].join('/');
}

const downloadExtractAndCache = (url, cachedFilePath) => {
  const options = {
    followRedirects: true, // Follow HTTP 3xx redirects
  };

  https.get(url, options, (response) => {
    if ([301, 302, 303, 307, 308].includes(response.statusCode)) {
      // If the response is a redirect, recursively call downloadExtractAndCache with the new URL
      downloadExtractAndCache(response.headers.location, cachedFilePath);
    } else if (response.statusCode === 200) {
      const gunzip = zlib.createGunzip();
      const untar = tar.extract({ cwd: process.cwd() }); // Extract to the current working directory

      // create cache directory to store download file
      try {
        fs.mkdirSync(path.dirname(cachedFilePath), {recursive: true});
      } catch (err) {
        console.error('Unable to create directory for cache', err);
        process.exit(1);
      }

      const chunks = [];
      response.on('data', (chunk) =>{
        chunks.push(chunk);
      })
      .on('end', (err) => {
        const fileData = Buffer.concat(chunks);
        fs.writeFile(cachedFilePath, fileData, (err) => {
          if (err) {
            console.error('Error writing BabylonReactNative cache file:', err);
            process.exit(1);
          }
        });
      })
      .pipe(gunzip) // Unzip the response
      .pipe(untar)  // Extract the tar file
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
    console.log(`Using Babylon React Native version ${reactNativePostfix} from Package version ${packageJson.version}.`);

    const jsArchive = 'react-native.tar.gz';
    const iosAndroidArchive = `iOSAndroid${reactNativePostfix}.tar.gz`;
    const windowsArchive = `Windows${reactNativePostfix}.tar.gz`;

    const jsArchiveCachedPath = getCached(packageJson, jsArchive);
    const iosAndroidArchiveCachedPath = getCached(packageJson, iosAndroidArchive);
    const windowsArchiveCachedPath = getCached(packageJson, windowsArchive);

    if (!useCachedFile(jsArchiveCachedPath)) {
      const reactNative = getBinaryUrl(packageJson, jsArchive);
      downloadExtractAndCache(reactNative, jsArchiveCachedPath);
    }

    if (!useCachedFile(iosAndroidArchiveCachedPath)) {
      const reactNativeiOSAndroid = getBinaryUrl(packageJson, iosAndroidArchive);
      downloadExtractAndCache(reactNativeiOSAndroid, iosAndroidArchiveCachedPath);
    }

    // check and download Windows binary if react-native-windows is found in project package
    const reactNativeWindowsVersion = projectPackageJson.dependencies['react-native-windows'];
    if (reactNativeWindowsVersion) {
      console.log("react-native-windows detected.");
      if (!useCachedFile(windowsArchiveCachedPath)) {
        const reactNativeiOSWindows = getBinaryUrl(packageJson, windowsArchive);
        downloadExtractAndCache(reactNativeiOSWindows, windowsArchiveCachedPath);
      }
    }
  } else {
    console.error("No react-native version found for BabylonReactNative.");
    process.exit(1);
  }
}

Install();