import { Engine } from '@babylonjs/core';
import compareVersions from 'compare-versions';
import packageDefinition from './package.json';

export function validateBabylonJSVersion(): string | undefined {
    if (__DEV__) {
        const babylonJSPackageName = "@babylonjs/core";
        const requiredVersion = packageDefinition.peerDependencies[babylonJSPackageName].replace('^', ''); 
        const currentVersion = Engine.Version;
        const comparision = compareVersions(currentVersion, requiredVersion);
        if (comparision < 0) {
            const message = `${packageDefinition.name}@${packageDefinition.version} requires ${babylonJSPackageName} version ${requiredVersion} or higher but version ${currentVersion} is currently installed. Update to the required version by running 'npm install ${babylonJSPackageName}@${requiredVersion}'.`;
            console.error(message);
            return message;
        }
    }
}