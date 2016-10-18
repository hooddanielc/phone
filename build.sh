export PATH=/home/developer/ib:$PATH
export PATH=/opt/cross/x-tools/arm-unknown-linux-gnueabi/bin/:$PATH

cd /data && mkdir

echo 'building raspi-phone-tools/raspi-phone-tools'
ib raspi-phone-tools/raspi-phone-tools --force --out_root out

echo 'building raspi-phone-tools/util-test'
ib raspi-phone-tools/util-test  --force --out_root out

echo 'building phone-controller'
cd phone-controller
./scripts/build.sh
