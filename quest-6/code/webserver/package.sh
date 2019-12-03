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

mv dist crawler-rollup
zip -r crawler-rollup.zip crawler-rollup
rm -r crawler-rollup

popd

