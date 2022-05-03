#!/usr/bin/env bash
set -ex

BUILD_NUMBER="${BUILD_NUMBER:-1}"
FW_PKG_NAME="intel-fw-gpu-prerelease"
TMP_VERSION="$(ls output/${FW_PKG_NAME}*.deb | awk -F"output/${FW_PKG_NAME}_" '{ print $2 }' | awk -F".deb" '{ print $1 }')"
VERSION="${TMP_VERSION//[$'\t\r\n']}"

(
cd module
sed -i -e "/^Provides.*/i \ ${FW_PKG_NAME} \(\>\=\ ${VERSION}\)" debian/control
dch -l "+i${BUILD_NUMBER}-" -m "build ${BUILD_NUMBER}"
dpkg-buildpackage -j`nproc --all` -us -uc -b -rfakeroot
)
mkdir -p output
cp *.deb output/
cp module/versions output/
rm -f *.deb