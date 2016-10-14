FROM dhoodlum/phone:v1.0.0
CMD cd /data && \
  ib raspi-phone-tools/raspi-phone-tools --out_root out && \
  ib raspi-phone-tools/file-test --out_root out && \
  cd phone-controller && ./scripts/build.sh && cd ..
