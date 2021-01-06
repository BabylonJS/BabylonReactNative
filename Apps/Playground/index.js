/**
 * @format
 */

debugger;
import {AppRegistry} from 'react-native';
import App from './App';
import {name as appName} from './app.json';

try {
    AppRegistry.registerComponent(appName, () => App);
} catch (error) {
    console.log(error);
}
