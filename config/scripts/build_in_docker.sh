#!/usr/bin/env bash
set -ex
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

OS_TYPE="${OS_TYPE:-ubuntu_20.04}"
. "$DIR/$OS_TYPE/defines.sh"
export PRODUCT_DIR="$( basename "$( dirname "$( dirname "$DIR" )" )" )"
export BUILD_TYPE="${BUILD_TYPE:-Release}"
export BUILD_NUMBER="${BUILD_NUMBER:-1}"
export BUILD_ID="${BUILD_ID:-2}"
export WORKSPACE="${WORKSPACE:-$DIR/../..}"
export CCACHE_DIR="${CCACHE_DIR:-$DIR/../../ccache}"
export LOCAL_USER_ID=`id -u`
export LOCAL_GROUP_ID=`id -g`
if [[ ! -d "$CCACHE_DIR" ]]; then
        mkdir -p $CCACHE_DIR
fi
#Take snapshot of manifest file#
dt workspace snapshot -o ${WORKSPACE}/output/manifest.yml

# Extra parameters for KW scans if $KLOCWORK=1
KW_PARAMS=
if [ "$KLOCWORK" == "1" ]; then
  KW_PARAMS+=" -v $WORKSPACE/klocwork:/kw"
  if [ -e "$HOME/.klocwork" ]; then
    KW_PARAMS+=" -v $HOME/.klocwork:/home/ubit/.klocwork"
  fi
  KW_PARAMS+=" -e COMPONENT_BRANCH=$COMPONENT_BRANCH"
  KW_PARAMS+=" -e COMPONENT_PROJECT=$COMPONENT_PROJECT"
  KW_PARAMS+=" -e GIT_REVISION=$GIT_REVISION"
  KW_PARAMS+=" -e KLOCWORK=$KLOCWORK"
  KW_PARAMS+=" -e KW_PROJECT_BRANCH=$KW_PROJECT_BRANCH"
  KW_PARAMS+=" -e KW_URL=$KW_URL"
  KW_PARAMS+=" -e KW_LIC_HOST=$KW_LIC_HOST"
  KW_PARAMS+=" -e KW_LIC_PORT=$KW_LIC_PORT"
  KW_PARAMS+=" -e SYSTEM=$SYSTEM"
fi

docker run -w /opt/src \
    -v $WORKSPACE:/opt/src \
    -v $CCACHE_DIR:/opt/ccache \
    --net=host \
    --init \
    -e BUILD_TYPE=$BUILD_TYPE \
    -e BUILD_NUMBER=$BUILD_NUMBER \
    -e BUILD_VERSION=$BUILD_VERSION \
    -e BUILD_ID=$BUILD_ID \
    -e PRODUCT_DIR=$PRODUCT_DIR \
    -e OS_VERSION=$OS_VERSION \
    -e CCACHE_DIR=/opt/ccache \
    -e LOCAL_USER_ID=`id -u` \
    -e LOCAL_GROUP_ID=`id -g` \
    $KW_PARAMS \
    $DOCKER_IMAGE \
    exec_with_ccache.sh \
    /opt/src/$PRODUCT_DIR/config/scripts/$OS_TYPE/build_components.sh
