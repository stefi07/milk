#!/usr/bin/env bash

source ./fetch_milk_dev.sh


for mod in AOloopControl AOloopControl_DM AOloopControl_IOtools AOloopControl_PredictiveControl AOloopControl_acquireCalib AOloopControl_compTools AOloopControl_computeCalib AOloopControl_perfTest FPAOloopControl
do
echo "Module ${mod}"
if [ -d "${mod}" ]; then
  echo "	Already installed -> skipping"
else
  git clone -b dev https://github.com/cacao-org/${mod}
  cat >> ../.gitmodules << EOF
[submodule "extra/${mod}"]
	path = extra/${mod}
	url = ""
EOF
  echo ""
fi
done
