FROM dhoodlum/phone
CMD cd /data && \
  ib raspi-phone-tools/raspi-phone-tools --out_root out && \
  ib raspi-phone-tools/file-test --out_root out && \
  cd phone-controller && ./scripts/build.sh && cd ..
