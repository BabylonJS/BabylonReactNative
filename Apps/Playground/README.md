BabylonReactNative supports different versions of react-native.
For each major version (0.64, 0.65,...) there is a different Playground App.
Each version is built using the react-native template with different dependencies version.

# How to add a new react-native supported version

Use the react native template to generate the app:

```
npx react-native init Playground --version 0.65.0 --template react-native-template-typescript
```

This will create a new Playground app using the version passed a parameter.
Rename the `Playground` folder as the react-native version. Here : `0.65`
Add or change the dependencies version in `package.json`, also, add this line in dependencies:
`"@babylonjs/playground-shared": "file:../playground-shared",`
remove `App.tsx` and `App.json` and change `index.js` to use playground-shared package:

```
import {AppRegistry} from 'react-native';
import App from '../playground-shared/App';
import {name as appName} from '../playground-shared/app.json';

AppRegistry.registerComponent(appName, () => App);
```

