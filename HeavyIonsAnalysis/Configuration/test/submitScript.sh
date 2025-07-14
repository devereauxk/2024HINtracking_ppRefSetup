#!/bin/bash

# Move the emap to FileInPath search path
cp emap_2023_newZDC_v3.txt $CMSSW_BASE/src
cp CentralityTable_HFtowers200_DataPbPb_periHYDJETshape_run3v1302x04_offline_374289.db $CMSSW_BASE/src

# Run the code
cmsRun -j FrameworkJobReport.xml -p PSet.py
