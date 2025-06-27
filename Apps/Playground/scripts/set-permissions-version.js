const fs = require('fs');
const path = require('path');

const packageJson = JSON.parse(fs.readFileSync('package.json', 'utf8'));
const rnVersion = packageJson.dependencies['react-native'] || packageJson.devDependencies['react-native'];

let permissionsVersion = '4.1.5'; // default

if (rnVersion.includes('0.72')) {
  permissionsVersion = '4.0.6';
} else if (rnVersion.includes('0.71')) {
  permissionsVersion = '3.10.1';
} else if (rnVersion.includes('0.70')) {
  permissionsVersion = '3.9.7';
}

// Update package.json with correct version
if (!packageJson.overrides) packageJson.overrides = {};
packageJson.overrides['react-native-permissions'] = permissionsVersion;

fs.writeFileSync('package.json', JSON.stringify(packageJson, null, 2));
