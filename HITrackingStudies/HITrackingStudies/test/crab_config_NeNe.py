###For a description of the crabConfig.py parameters. See:
###https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile

import CRABClient
from WMCore.Configuration import Configuration
config = Configuration() ###create a Configuration object

config.section_('General')###add a new section of type "General"
###General: In this section, the user specifies generic parameters about the request (e.g. request name).
config.General.workArea     = 'NeNe_Efficiency' ###fixed name for projects dir in my area

config.General.requestName  = 'MinBias_NeNe_5p36TeV_hijing_Nominal'

config.General.transferLogs = False 
config.General.transferOutputs = True

################################

config.section_('JobType')###add a new section of type "JobType"
###JobType: This section aims to contain all the parameters of the user job type and 
###related configurables (e.g. CMSSW parameter-set configuration file, additional input files, etc.).
config.JobType.pluginName     = 'Analysis'
#config.JobType.psetName       = 'run_NeNe_cfg_byPdgId.py'
config.JobType.psetName       = 'run_NeNe_cfg.py'

#config.JobType.allowNonProductionCMSSW = True
#config.JobType.maxMemoryMB    = 2500

################################

config.section_('Data')###add a new section of type "Data"
###Data: This section contains all the parameters related to the data to be analyzed, 
###including the splitting parameters.

# private NeNe Hijing minbias
config.Data.inputDataset      = '/MinBias_Hijing_NeNe_5362GeV/cbaldene-AOD_250716_el8_Run3_2025_NeNe_1509p1-3b764edcebe61584fa2cd371a8b58648/USER'

config.Data.splitting         = 'FileBased'
#config.Data.unitsPerJob       = X ###files per job (but not impose)
#config.Data.totalUnits        = Y ###how many files to analyze
config.Data.unitsPerJob       = 10
config.Data.totalUnits        = -1
config.Data.inputDBS          = 'phys03'
#config.Data.inputDBS          = 'global'
#config.Data.publishDBS = 'phys03'
#config.Data.publishDataName = 'TestingJobs_RecoWithNewPixelTracks'
#config.Data.outputDatasetTag = 'HBTAnalysispp13TeV_MC_Test01' ###change for each sample(task) --only if publish

#config.Data.outLFN            = '/store/user/caber/TESTJOBS_RecoWithNewPixelTracks'

#config.Data.outLFNDirBase            = '/store/user/caber/EventSizeSubDetectors_FlowCorrDataset'
config.Data.outLFNDirBase            = '/store/group/phys_heavyions/kdeverea/Tracking_NeNe_5p36TeV/'


################################

config.section_('Site')###add a new section of type "Site"
###Site: Grid site parameters are defined in this section, including the stage out information 
###(e.g. stage out destination site, white/black lists, etc.).
###config.Site.storageSite       = 'T2_BR_SPRACE'
config.Site.storageSite       = 'T2_CH_CERN'
#config.Site.whitelist         = ['T2_US_MIT']
