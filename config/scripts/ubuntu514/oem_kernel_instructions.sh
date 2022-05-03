#!/usr/bin/env bash
set -ex

VERSION=$(cat oem_kernel_version.txt)

#Build instructions for installing oem full kernel
cat << EOF | tee oem_kernel_instructions.txt
### Install instructions for the full oem kernel ###
# Install this before installing dkms
sudo apt-get update && \
sudo apt-get install -y --install-suggests $VERSION
EOF

cp -v oem_kernel_instructions.txt /opt/src/output/.