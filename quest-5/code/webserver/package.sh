#!/bin/bash
pushd server 

npx tsc --outDir ../dist/server
cp package.json ../dist/server
cp package-lock.json ../dist/server

cd ../client

npx webpack --output-path=../dist/client/dist
cp index.html ../dist/client
cp package.json ../dist/client
cp package-lock.json ../dist/client

cd ../

mv dist security-central
zip -r security-central.zip security-central
rm -r security-central

popd

