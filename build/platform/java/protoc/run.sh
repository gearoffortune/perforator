#!/bin/sh

set -uex

version="$1"
wget https://github.com/protocolbuffers/protobuf/releases/download/v${version}/protoc-${version}-osx-x86_64.zip
wget https://github.com/protocolbuffers/protobuf/releases/download/v${version}/protoc-${version}-osx-aarch_64.zip
wget https://github.com/protocolbuffers/protobuf/releases/download/v${version}/protoc-${version}-win64.zip
wget https://github.com/protocolbuffers/protobuf/releases/download/v${version}/protoc-${version}-linux-x86_64.zip
wget https://github.com/protocolbuffers/protobuf/releases/download/v${version}/protoc-${version}-linux-aarch_64.zip

for x in osx-x86_64 osx-aarch_64 win64 linux-x86_64 linux-aarch_64; do (
    unzip protoc-${version}-${x}.zip -d $x
    rm -f protoc-${version}-${x}.zip
    cd $x
    tar czf ../protoc-${x}.tgz .
    cd ..
    rm -rf $x
) done

echo "{ \"version\": \"${version}\" }" > version.json
