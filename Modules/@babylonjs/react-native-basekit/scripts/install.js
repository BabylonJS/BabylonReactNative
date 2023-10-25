const https = require("https");
const zlib = require("zlib");
const tar = require("tar");
const path = require("path");
const fs = require("fs");

function getCachePathCandidates() {
  return [
    process.env.npm_config_babylon_binary_cache,
    process.env.npm_config_cache,
  ].filter(function(_) {
    return _;
  });
}

function getCached(package, binaryFilename) {
  return [
    getCachePathCandidates(),
    "BabylonReactNative",
    package.version,
    binaryFilename,
  ].join("/");
}

function useCachedFile(filePath) {
  return new Promise((resolve, reject) => {
    if (fs.existsSync(filePath)) {
      const gunzip = zlib.createGunzip();
      const untar = tar.extract({ cwd: process.cwd() }); // Extract to the current working directory

      const sourceStream = fs.createReadStream(filePath);
      sourceStream
        .pipe(gunzip) // Decompress the .tar.gz file
        .pipe(untar) // Extract the tar contents to the cwd
        .on("error", (err) => {
          reject(err);
        })
        .on("end", () => {
          resolve(true);
        });
    } else {
      resolve(false);
    }
  });
}

function getArgument(name) {
  const flags = process.argv.slice(2),
    index = flags.lastIndexOf(name);

  if (index === -1 || index + 1 >= flags.length) {
    return null;
  }

  return flags[index + 1];
}

function getBinaryUrl(package, binaryFilename) {
  var site =
    getArgument("--brn-binary-site") ||
    process.env.BRN_BINARY_SITE ||
    process.env.npm_config_brn_binary_site ||
    (package.nodeBabylonConfig && package.nodeBabylonConfig.binarySite) ||
    "https://github.com/CedricGuillemet/BabylonReactNative-1/releases/download";

  return [site, package.version, binaryFilename].join("/");
}

function downloadExtractAndCache(url, cachedFilePath, cb) {
  const options = {
    followRedirects: true, // Follow HTTP 3xx redirects
  };

  https
    .get(url, options, (response) => {
      if ([301, 302, 303, 307, 308].includes(response.statusCode)) {
        // If the response is a redirect, recursively call downloadExtractAndCache with the new URL
        downloadExtractAndCache(response.headers.location, cachedFilePath, cb);
      } else if (response.statusCode === 200) {
        const gunzip = zlib.createGunzip();
        const untar = tar.extract({ cwd: process.cwd() }); // Extract to the current working directory

        // create cache directory to store download file
        const dir = path.dirname(cachedFilePath);
        try {
          if (!fs.existsSync(dir)) {
            fs.mkdirSync(dir, { recursive: true });
          }
        } catch (err) {
          reject(err);
        }

        var file = fs.createWriteStream(cachedFilePath);

        response
          .pipe(gunzip) // Unzip the response
          .pipe(untar) // Extract the tar file
          .on("error", (err) => {
            reject(err);
          });
        response.pipe(file);
        file.on("finish", () => {
          file.close((err) => {
            if (err) {
              reject(err);
            } else {
              cb();
            }
          }); // close() is async, call cb after close completes.
        });
      } else {
        reject(
          `Failed to download the file. Status code: ${response.statusCode}`
        );
      }
    })
    .on("error", (err) => {
      cb(err);
    });
}

function installZip(filePath, fileURL) {
  return new Promise((resolve, reject) => {
    useCachedFile(filePath)
      .then((result) => {
        if (result) {
          resolve(true);
        } else {
          // File does not exist in cache
          downloadExtractAndCache(fileURL, filePath, (err) => {
            if (err) {
              reject(err);
            } else {
              resolve(true);
            }
          });
        }
      })
      .catch((err) => {
        reject(err);
      });
  });
}

function install() {
  // get Babylon React Native package version
  const packageJsonPath = process.env.npm_package_json;
  const packageJson = require(packageJsonPath);
  if (!packageJson.version) {
    console.err("Babylon React Native version not found");
    process.exit(1);
  }
  // get user project react-native version
  const projectPackageJsonPath =
    process.env.npm_config_local_prefix + "/package.json";

  // Read the project's package.json file
  const projectPackageJson = require(projectPackageJsonPath);
  // Check if the project's package.json has a 'react-native' dependency
  if (
    projectPackageJson.dependencies &&
    projectPackageJson.dependencies["react-native"]
  ) {
    const reactNativeVersion = projectPackageJson.dependencies["react-native"];
    console.log(`Detected React Native version: ${reactNativeVersion}`);

    const [_, packageMinor] = reactNativeVersion.split('.').map(Number);

    reactNativePostfix = "";
    if (packageMinor >= "71") {
      reactNativePostfix = "0.71";
    } else if (packageMinor >= "70") {
      reactNativePostfix = "0.70";
    } else if (packageMinor >= "69") {
      reactNativePostfix = "0.69";
    } else {
      console.error("Unsupported react native version.");
      process.exit(1);
    }
    console.log(
      `Using Babylon React Native version ${reactNativePostfix} from Package version ${packageJson.version}.`
    );

    const jsArchive = "react-native.tar.gz";
    const iosAndroidArchive = `iOSAndroid${reactNativePostfix}.tar.gz`;
    const windowsArchive = `Windows${reactNativePostfix}.tar.gz`;

    const jsArchiveCachedPath = getCached(packageJson, jsArchive);
    const iosAndroidArchiveCachedPath = getCached(
      packageJson,
      iosAndroidArchive
    );
    const windowsArchiveCachedPath = getCached(packageJson, windowsArchive);

    const jsArchiveURL = getBinaryUrl(packageJson, jsArchive);
    const iOSAndroidArchiveURL = getBinaryUrl(packageJson, iosAndroidArchive);
    const windowsArchiveURL = getBinaryUrl(packageJson, windowsArchive);

    const reactNativeWindowsVersion =
      projectPackageJson.dependencies["react-native-windows"];
    installZip(jsArchiveCachedPath, jsArchiveURL)
      .then((result) => {
        installZip(iosAndroidArchiveCachedPath, iOSAndroidArchiveURL)
          .then((result) => {
            if (reactNativeWindowsVersion) {
              installZip(windowsArchiveCachedPath, windowsArchiveURL)
                .then((result) => {})
                .catch((err) => {
                  reject(err);
                });
            }
          })
          .catch((err) => {
            reject(err);
          });
      })
      .catch((err) => {
        console.error("Error install BabylonReactNative archive:", err);
        process.exit(1);
      });
  } else {
    console.error("No react-native version found for BabylonReactNative.");
    process.exit(1);
  }
}

install();
