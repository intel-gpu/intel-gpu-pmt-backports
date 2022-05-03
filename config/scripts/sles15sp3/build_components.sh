#!/usr/bin/env bash
set -ex

# Zero ccache stats
ccache -z

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
$DIR/copy_firmware.sh
$DIR/build_module.sh
$DIR/build_pmt.sh
$DIR/patch_agama.sh
$DIR/install_pmt.sh

# Print ccache stats
ccache -s
