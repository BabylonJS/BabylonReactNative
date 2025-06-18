const path = require("path");

const { makeMetroConfig } = require("@rnx-kit/metro-config");
module.exports = makeMetroConfig({
  transformer: {
    getTransformOptions: async () => ({
      transform: {
        experimentalImportSupport: false,
        inlineRequires: false,
      },
    }),
  },
  resolver: {
    extraNodeModules: {
      '@babylonjs/core': path.resolve(__dirname, './node_modules/@babylonjs/core'),
      'base-64': path.resolve(__dirname, './node_modules/base-64'),
      'semver': path.resolve(__dirname, './node_modules/semver'),
      'react-native-permissions':  path.resolve(__dirname, './node_modules/semver/react-native-permissions')
    },
  },
  watchFolders: [
    path.dirname(require.resolve("@babylonjs/react-native/package.json")),
  ],
});
