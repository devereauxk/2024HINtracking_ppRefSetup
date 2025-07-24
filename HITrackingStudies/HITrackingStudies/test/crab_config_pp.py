###For a description of the crabConfig.py parameters. See:
###https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile

import CRABClient
from WMCore.Configuration import Configuration
config = Configuration() ###create a Configuration object

config.section_('General')###add a new section of type "General"
###General: In this section, the user specifies generic parameters about the request (e.g. request name).
config.General.workArea     = 'ppref_Efficiency' ###fixed name for projects dir in my area

config.General.requestName  = 'QCD_pThat-15to1200_TuneCP5_5p36TeV_pythia8_vzpthatWeight_vzcut_Nominal_byPdgId'

config.General.transferLogs = False 
config.General.transferOutputs = True

################################

config.section_('JobType')###add a new section of type "JobType"
###JobType: This section aims to contain all the parameters of the user job type and 
###related configurables (e.g. CMSSW parameter-set configuration file, additional input files, etc.).
config.JobType.pluginName     = 'Analysis'
#config.JobType.psetName       = 'run_PPRef_cfg.py'
config.JobType.psetName       = 'run_PPRef_cfg_byPdgId.py'

#config.JobType.allowNonProductionCMSSW = True
#config.JobType.maxMemoryMB    = 2500

################################

config.section_('Data')###add a new section of type "Data"
###Data: This section contains all the parameters related to the data to be analyzed, 
###including the splitting parameters.

# unofficial pp MC datasets
#config.Data.inputDataset      = '/Pythia8_LowPU_0p5MEvents_TuneCP5_5360GeV/rpradhan-PrivatePythia8_LowPU_0p5MEvents_RecoDebug-7d0fd9a913ab722b2353469d41166019/USER'
#config.Data.inputDataset      = '/Pythia8_NoPU_0p5MEvents_TuneCP5_QCD_ptHat15_5360GeV/rpradhan-PrivatePythia8_NoPU_0p5MEvents_TuneCP5_QCD_ptHat15_RecoDebug-7d0fd9a913ab722b2353469d41166019/USER'

# official pp MC datasets
config.Data.inputDataset      = '/QCD_pThat-15to1200_TuneCP5_5p36TeV_pythia8/RunIIIpp5p36Winter24DR-NoPU_FEVTDEBUGHLT_forTracking_141X_mcRun3_2024_realistic_ppRef5TeV_v7-v4/GEN-SIM-RECODEBUG'

config.Data.splitting         = 'FileBased'
#config.Data.unitsPerJob       = X ###files per job (but not impose)
#config.Data.totalUnits        = Y ###how many files to analyze
config.Data.unitsPerJob       = 10
config.Data.totalUnits        = -1
#config.Data.inputDBS          = 'phys03'
config.Data.inputDBS          = 'global'
config.Data.outLFNDirBase      = '/store/group/phys_heavyions/kdeverea/Tracking_ppref_5p36TeV/'
#config.Data.outputDatasetTag   = 'Pythia_NoPU_NoTrigger_TuneCP5_QCD_ptHat15_5p36TeV_vzpthatWeight_vzcut_Nominal'
################################

config.section_('Site')###add a new section of type "Site"
###Site: Grid site parameters are defined in this section, including the stage out information 
###(e.g. stage out destination site, white/black lists, etc.).
###config.Site.storageSite       = 'T2_BR_SPRACE'
config.Site.storageSite       = 'T2_CH_CERN'
#config.Site.storageSite       = 'T2_US_MIT'
#config.Site.whitelist         = ['T2_US_MIT']
