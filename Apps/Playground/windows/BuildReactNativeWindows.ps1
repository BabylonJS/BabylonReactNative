Push-Location "$PSScriptRoot\..\..\..\submodules\react-native-windows"

yarn install
yarn build

cd "$PSScriptRoot\..\..\..\submodules"
npm install --global verdaccio
invoke-expression 'cmd /c start powershell -Command { verdaccio }'
Start-Sleep -s 3

# Note: Don't use a real account here, just set email and username as test, then use any password
cd "$PSScriptRoot\..\..\..\submodules\react-native-windows\vnext"
yarn publish --registry http://localhost:4873/

cd "$PSScriptRoot\.."
yarn install --registry http://localhost:4873/

Pop-Location