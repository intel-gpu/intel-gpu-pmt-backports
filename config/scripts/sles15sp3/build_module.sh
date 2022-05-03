#!/usr/bin/env bash
set -ex
BUILD_NUMBER="${BUILD_NUMBER:-1}"
BUILD_DIR=/opt/src/build_module
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PKG_NAME="intel-fw-gpu-prerelease"
VERSION="$(ls output/${PKG_NAME}*.rpm | awk -F"output/${PKG_NAME}-" '{ print $2 }' | awk -F"-" '{ print $1 }')"
DMA_BUF_NAME=intel-dmabuf-dkms-prerelease

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
cp -pr module/* $BUILD_DIR/.
cp -pr module/.git $BUILD_DIR/

(
cd $BUILD_DIR
sed -i -e "/[[:space:]]Requires.*/i Requires\: ${PKG_NAME} \>\=\ ${VERSION}" scripts/backport-mkdmabufdkmsspec
cp defconfigs/dmabuf .config
make BUILD_VERSION="$BUILD_NUMBER" dmadkmsrpm-pkg
#Insert dependency into dkms i915 pkg
DMA_BUF_VERSION="$(rpm -qi  ~/rpmbuild/RPMS/x86_64/${DMA_BUF_NAME}-*.rpm | grep Version | awk -F": " '{print $2}')"
sed -i -e "/[[:space:]]Requires: intel-dmabuf-dkms-prerelease/s/$/ = ${DMA_BUF_VERSION}/" scripts/backport-mki915dkmsspec
cp defconfigs/i915 .config
make BUILD_VERSION="$BUILD_NUMBER" i915dkmsrpm-pkg
)

mkdir -p output
cp ~/rpmbuild/RPMS/x86_64/*.rpm output/

cp module/versions output/