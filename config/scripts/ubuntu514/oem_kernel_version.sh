#!/usr/bin/env bash
set -ex

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
MANIFEST_DIR="$DIR/../../config/manifests/linux.yml"

grep -i comment ${MANIFEST_DIR} | awk -F"first " '{ print $2 }' >> oem_kernel_version.txt
cp -v oem_kernel_version.txt /opt/src/output/.
