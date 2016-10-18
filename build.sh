export PATH=/home/developer/ib:$PATH

cd /data

echo 'building raspi-phone-tools/raspi-phone-tools'
ib raspi-phone-tools/raspi-phone-tools --force --out_root out

echo 'building raspi-phone-tools/util-test'
ib raspi-phone-tools/util-test  --force --out_root out

echo 'building phone-controller'
cd phone-controller
./scripts/build.sh
