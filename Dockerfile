FROM dhoodlum/phone:v1.0.0
CMD cd /data && \
  ib raspi-phone-tools/raspi-phone-tools --force --out_root out && \
  ib raspi-phone-tools/util-test  --force --out_root out && \
  cd phone-controller && ./scripts/build.sh && cd .. && bash
