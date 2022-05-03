FROM dockerv2-gfx-build.gfx-assets.intel.com/upstream/debian:buster-slim

RUN apt-get update && apt-get install -y \
    python3     \
    python3-pip \
    wget        \
    zip

RUN pip3 install \
  pyyaml

RUN \
  mkdir /opt/ipldt && \
  wget --quiet https://gfx-assets.fm.intel.com/artifactory/gbi-build-assets-fm/infra/ipldt/v3.10/ipldt3_lin_intel64.tgz && \
  tar -xzf ipldt3_lin_intel64.tgz --directory /opt/ipldt && \
  rm -rf ipldt3_lin_intel64.tgz

ENTRYPOINT ["/bin/bash", "-c"]
