const fs = require('fs');
const path = require('path');

const packageJson = JSON.parse(fs.readFileSync('package.json', 'utf8'));
const rnVersion = packageJson.dependencies['react-native'] || packageJson.devDependencies['react-native'];

let permissionsVersion = '4.1.5'; // default for RN 0.73+

if (rnVersion.includes('0.72')) {
  permissionsVersion = '4.0.6';
} else if (rnVersion.includes('0.71')) {
  permissionsVersion = '3.10.1';
} else if (rnVersion.includes('0.70')) {
  permissionsVersion = '3.9.7';
}

// Update the actual dependency version
if (packageJson.dependencies && packageJson.dependencies['react-native-permissions']) {
  packageJson.dependencies['react-native-permissions'] = permissionsVersion;
  console.log(`Updated react-native-permissions to ${permissionsVersion} in dependencies`);
}

if (packageJson.devDependencies && packageJson.devDependencies['react-native-permissions']) {
  packageJson.devDependencies['react-native-permissions'] = permissionsVersion;
  console.log(`Updated react-native-permissions to ${permissionsVersion} in devDependencies`);
}

// Write back to package.json
fs.writeFileSync('package.json', JSON.stringify(packageJson, null, 2));
console.log(`Set react-native-permissions version to ${permissionsVersion} based on React Native version ${rnVersion}`);