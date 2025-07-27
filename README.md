# Track correction table production

Kyle Devereaux

Forked from Raghunath Pradhan's repository available [here](https://github.com/RAGHUMATAPITA/2024HINtracking_ppRefSetup/tree/ptHatWeightandVzWeight).

## Downloading the code for the first time

First get the correct CMSSW version for the AOD we will be processing. For example we use `14_1_7` for official ppref pythia sample, `15_0_8` for official OO Hijing sample.

```
ssh username@lxplus8.cern.ch
cmsrel CMSSW_15_0_8
cd CMSSW_15_0_8
```

Replace the `src` folder with our tracking correction repo, set up the envirnoment, and build the repository, and staring voms proxy

```
rm -r src

git clone git@github.com:devereauxk/2024HINtracking_ppRefSetup.git src

cd src
cmsenv

scram b -j8

voms-proxy-init -rfc -voms cms
```
The workspace is now ready for producing track corrections.

### Setting up the environment subsequent times
After everything is setup, upon returning to the working directory you just need to run
```
cd CMSSW_15_0_8/src
cmsenv
voms-proxy-init -rfc -voms cms
```

## Producing track correction tables

```
cd HITrackingStudies/HITrackingStudies/test
```

The pp reference correction tables are produced with the config file `run_PPRef_cfg.py` and the OO corrections are with the `run_OO_cfg.py` file. These files call on the module `HITrackCorrectionAnalyzer_cfi` defined at
```
HITrackingStudies/HITrackingStudies/src/HITrackCorrectionAnalyzer.cc
```
for calculating the corrections. Note that modifications to this module need to be recompiled using `scram b -j8`.

To test the functionality of your config file, for example the pp file, it can be ran with `cmsRun` on a debug files defined in `ppref.py` via
```
cmsRun run_PPRef_cfg.py sample="MC_RecoDebug" n=100 usePixelTrks=False  runOverStreams=False
```
Once you are happy with the functionality of your config file, you should go through `crab_config_pp.py`, edit the options as needed, and then submit a crab job to run over your entire dataset via
```
crab submit -c crab_config_pp.py
```
The outputed files have the necessary histograms to compute the Absolute Efficiency, Fake Rate, Multiple Reconstruction, and Secondary reconstruction tables.

### Plotting
Navigate to the plotting directory
```
cd HITrackingStudies/HITrackingStudies/test/plottingMacro
```
Modify `plotHist2D.C` with the path to the outputed correction table path (be sure to merge the files if it was ran as a crab job). Run the plotting macro
```
root -l -q -b plotHist2D.C
```
Output plots will be written to `/files` as well as a `.root` file with the 2D (eta, pT) histograms
* `rEff` : absolute efficiency
* `rFak` : fake rate
* `rSec` : secondary reconstructions
* `rMul` : multiple reconstructions

### Calculating a correction
```
(1 - fake) * (1 - secondary) / (efficiency * (1 + multipleReco))
```


## Species-by-species tables

The `HITrackCorrectionAnalyzer_byPdgId_cfi` module produces the absolute efficiency table for several particle species. It is defined at
```
HITrackingStudies/HITrackingStudies/src/HITrackCorrectionAnalyzer_byPdgId.cc
```
modifications to the structure should be made in that file and recompiled.

The pp config file is at `run_PPRef_cfg_byPdgId.py`.
The OO config file is at `run_OO_cfg_byPdgId.py`. The `config.JobType.psetName` flag in crab files for pp and OO should be modified as needed to use the desired config file.

### Plotting
```
cd HITrackingStudies/HITrackingStudies/test/plottingMacro
root -l -q -b plotHist2D_byPdgId.C
```
modify `plotHist2D_byPdgId.C` with the path for outputted tables. Similar to `plotHist2D.C`, this macro will produce plots and a `.root` file and save them to `/files_byPdgId`. The 2D absolute efficiency tables include
* `rEff_0` : inclusive particles
* `rEff_2212` : $p + \bar{p}$
* `rEff_211` : $\pi^{+} + \pi^{-}$
* `rEff_321` : $K^+ + K^-$
* `rEff_3222` : $\Sigma^+ + \bar{\Sigma}^+$
* `rEff_3112` : $\Sigma^- + \bar{\Sigma}^-$
* `rEff_9999` : all remaining particles not specifically selected

Additionally there are the $\eta$-integrated 1D histograms (histograms over $p_T$) named `rEff_1D_<PdgId>`. We integrate over $|\eta|<1$.

## Tunable parameters

Change tracking cuts
```
    process.HITrackCorrections.dxyErrMax = 3.0 #default 3.0
    process.HITrackCorrections.dzErrMax = 3.0 # default 3.0
```
```
   if(fabs(dxy/dxysigma) > dxyErrMax_) return false;
   if(fabs(dz/dzsigma) > dzErrMax_) return false;
   //if(fabs(track.ptError()) / track.pt() > ptErrMax_) return false; // original

   if(track.pt() > 10)
     {
       if(fabs(track.ptError()) / track.pt() > 0.1) return false;
     }
   else
     {
       if(fabs(track.ptError()) / track.pt() > ptErrMax_) return false;
     }
```

Change flag of VZ and pTHat weight
```
    process.HITrackCorrections.doVtxReweighting = True
    process.HITrackCorrections.dopthatWeight = True
```

Change global tag
```
from Configuration.AlCa.GlobalTag import GlobalTag
if (options.sample == "MC_MiniAOD" or options.sample == "MC_RecoDebug" or options.sample == "MC_Reco_AOD"):
    process.GlobalTag = GlobalTag(process.GlobalTag, '141X_mcRun3_2024_realistic_ppRef5TeV_v7', '')
    process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
    
if (options.sample == "Data_Reco_AOD" or options.sample == "Data_MiniAOD"):
    process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Express_v3', '') # 2024 ppref
    process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
```

Change MC trigger 
```
    process.hltMB.HLTPaths = ["HLT_PPRefZeroBias_v6*"] # for 2024 ppref #
```
