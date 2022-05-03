#!/usr/bin/env bash
set -ex

OS_PATH="agama/linux/$OS_VERSION"

rm $OS_PATH/intel-platform-pmt-*.rpm
cp output/pmt/*.rpm $OS_PATH/.