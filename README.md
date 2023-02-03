# Intel® Platform Monitoring Technology Backports

This repo holds backports of the pmt driver.

The driver is provided as a DKMS module targeting distinct
versions of various distros. Each backport is on a topic 
branch. Current support status is documented on the topic branch.

## Branching and tagging strategy

We will no longer update branches and add tags for specific operating systems. 
At the moment, the driver can be built for all supported operating systems from the branches listed below. 
For each branch there will be created corresponding release tag with the branch name suffix.

- main
  - SLES 15SP4
  - Ubuntu 20.04 (kernel 5.15)
  - Ubuntu 22.04 (kernel 5.17)
  - RHEL 8.6

- legacy
  - SLES 15SP3

## Dependencies
This driver is part of a collection of kernel mode drivers 
that together enable support for Intel graphics. The backports 
collection within https://github.com/intel-gpu includes:

  - [i915](https://github.com/intel-gpu/intel-gpu-i915-backports) - The main graphics driver (includes a compatible DRM subsystem and dmabuf if necessary).
  - [cse](https://github.com/intel-gpu/intel-gpu-cse-backports) - Converged Security Engine
  - [pmt](https://github.com/intel-gpu/intel-gpu-pmt-backports) - Intel Platform Telemetry
  - [firmware](https://github.com/intel-gpu/intel-gpu-firmware) - Contains firmware required by i915.  

Each project is tagged consistently so when pulling these repos pull the same tag.

## How to generate the DKMS module

### SLES 15SP3

#### Install dependencies:

```
sudo zypper install \
   dkms \
   make \
   linux-glibc-devel \
   lsb-release \
   rpm-build
```

#### Build and install dkms package
```
make -f Makefile.dkms BUILD_VERSION=1 dkmsrpm-pkg
```

The resulting rpm package will be located in:
/usr/src/packages/RPMS/x86_64/

Install with:

```
sudo zypper install --allow-unsigned-rpm \
    /usr/src/packages/RPMS/x86_64/intel-platform-pmt-dkms-*.rpm
```

## How to generate the binary package

### SLES 15SP3

#### Install dependencies:

```
sudo zypper install \
   make \
   linux-glibc-devel \
   lsb-release \
   rpm-build
```

#### Build and install binary package
```
make -f Makefile.dkms BUILD_VERSION=1 binrpm-pkg
```

The rpm package will be placed in driver's directory in $HOME/rpmbuild/RPMS/x86_64/.
For example:

```
/home/user/rpmbuild/RPMS/x86_64/intel-platform-pmt-kmp-default-2022.42_k5.3.18_150300.59.93-1.x86_64.rpm
```

Install with:

```
cp $HOME/rpmbuild/RPMS/x86_64/*.rpm .
sudo rpm -ivh intel-platform-pmt*.rpm
```