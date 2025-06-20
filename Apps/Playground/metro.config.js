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
      '@babylonjs/loaders': path.resolve(__dirname, './node_modules/@babylonjs/loaders'),
      '@babylonjs/react-native': path.resolve(__dirname, './node_modules/@babylonjs/tract-native'),
      'base-64': path.resolve(__dirname, './node_modules/base-64'),
      'semver': path.resolve(__dirname, './node_modules/semver'),
      'react-native-permissions':  path.resolve(__dirname, './node_modules/semver/react-native-permissions')
    },
  },
  watchFolders: [
    path.dirname(require.resolve("@babylonjs/react-native/package.json")),
  ],
});
