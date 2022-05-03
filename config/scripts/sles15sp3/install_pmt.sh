set -ex

(
cd output/pmt
sudo rpm -i intel-platform-pmt-*.rpm --noscripts
)

ls -t /usr/src
pmt_package_location=$(ls -t /usr/src | head -n 1)
kernel_ver=$(ls /lib/modules | grep default)

(
cd /usr/src/${pmt_package_location}
sudo rm -rf /lib/modules/${kernel_ver}/updates
sudo dkms install . -k ${kernel_ver}
)

cd $PRODUCT_DIR/bin
pmt_modules=$(ls *.ko)
pmt_dkms=$(ls /lib/modules/${kernel_ver}/updates)

missing_modules=""
for module in ${pmt_modules}; do
    if [[ "${pmt_dkms}" != *"$module"* ]]; then
        missing_modules+="${module} "
    fi
done

if [[ ! -z $missing_modules ]]; then
    echo "Error: missing modules: ${missing_modules}"
    exit 1
fi

dkms status
