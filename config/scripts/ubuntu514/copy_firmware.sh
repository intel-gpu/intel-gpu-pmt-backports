#!/usr/bin/env bash
set -ex

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
OS_VERSION="ubuntu/20.04"

#Copy fw package
mkdir -p output
cp -pR firmware/linux/${OS_VERSION}/intel-fw-gpu*.deb output/.

#Test install
sudo dpkg -i --force-depends output/intel-fw-gpu*.deb