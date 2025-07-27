###Options to run
'''
To run it, please, do e.g.:
cmsRun run_OO_cfg_byPdgId.py sample="MC_RecoDebug" n=100 usePixelTrks=False  runOverStreams=False 

sample="MC_RecoDebug","MC_Reco_AOD","MC_MiniAOD","Data_Reco_AOD","Data_MiniAOD"
n=integer number of events
runOverStreams=False or True
usePixelTrks=False or True

IMPORTANT: only run runOverStreams=True together with sample="Data_Reco_AOD". FIXME: this option is not working for now

To change input files, please, look at oxyoxy.py file


for Quick change between 2023 and 2024 data, check the the follwoing items

process.GlobalTag --> Needs to change for 2023 and 2024
process.hltMB.HLTPaths --> Needs to change for 2023 and 2024
process.TFileService --> This is just the name of produced root file, you can change or not
'''

import FWCore.ParameterSet.Config as cms

process = cms.Process('TRACKANA')
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('HITrackingStudies.HITrackingStudies.HITrackCorrectionAnalyzer_byPdgId_cfi')

import FWCore.ParameterSet.VarParsing as VarParsing
options = VarParsing.VarParsing('analysis')

options.register ('sample',
                  "MC_RecoDebug", # default value
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.string,          # string, int, bool or float
                  "sample")
options.register ('n',
                  50, # default value
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.int,          # string, int, bool or float
                  "n")
options.register ('runOverStreams',
                  False, # default value
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.bool,          # string, int, bool or float
                  "runOverStreams")
options.register ('usePixelTrks',
                  False, # default value
                  VarParsing.VarParsing.multiplicity.singleton, # singleton or list
                  VarParsing.VarParsing.varType.bool,          # string, int, bool or float
                  "usePixelTrks")
options.parseArguments()

from oxyoxy import oxyoxy_mc_recodebug as oxyoxy
if options.sample == "MC_Reco_AOD":
    from oxyoxy import oxyoxy_mc_reco_aod as oxyoxy
if options.sample == "Data_Reco_AOD":
    from oxyoxy import oxyoxy_data_reco_aod as oxyoxy
if options.sample == "Data_Reco_AOD" and options.runOverStreams==True :
    from oxyoxy import oxyoxy_data_reco_aod_streams as oxyoxy
if options.sample == "Data_MiniAOD":
    from oxyoxy import oxyoxy_data_miniaod as oxyoxy
if options.sample == "Data_MiniAOD" and options.runOverStreams==True :
    from oxyoxy import oxyoxy_data_miniaod_streams as oxyoxy
if options.sample == "MC_MiniAOD":
    from oxyoxy import oxyoxy_mc_miniaod as oxyoxy


process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True),
)

process.TFileService = cms.Service("TFileService", fileName = cms.string('OfficialOOMC_Hijing_5p36TeV_byPdgId.root'))

process.load("SimTracker.TrackAssociation.trackingParticleRecoTrackAsssociation_cfi")
process.tpRecoAssocGeneralTracks = process.trackingParticleRecoTrackAsssociation.clone()
process.tpRecoAssocGeneralTracks.label_tr = cms.InputTag("generalTracks")

if options.usePixelTrks == True:
    process.tpRecoAssocGeneralTracks.label_tr = cms.InputTag("hiConformalPixelTracks")

process.load("SimTracker.TrackAssociatorProducers.quickTrackAssociatorByHits_cfi")
process.quickTrackAssociatorByHits.SimToRecoDenominator = cms.string('reco')

process.load("SimTracker.TrackerHitAssociation.tpClusterProducerDefault_cfi")
process.tpClusterProducer  = process.tpClusterProducerDefault.clone()

# Input source
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames =  cms.untracked.vstring(oxyoxy),
    skipEvents = cms.untracked.uint32(0),
    secondaryFileNames = cms.untracked.vstring()
)
if options.runOverStreams == True :
    process.source = cms.Source("NewEventStreamFileReader",
            fileNames =  cms.untracked.vstring(oxyoxy),
            skipEvents = cms.untracked.uint32(0)
    )

### centrality ###
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

### Track cuts ###
# input collections
process.HITrackCorrections_byPdgId.centralitySrc = cms.InputTag("centralityBin","HFtowers")
process.HITrackCorrections_byPdgId.trackSrc = cms.InputTag("generalTracks")
if options.usePixelTrks == True:
    process.HITrackCorrections_byPdgId.trackSrc = cms.InputTag("hiConformalPixelTracks")
process.HITrackCorrections_byPdgId.vertexSrc = cms.InputTag("offlinePrimaryVertices")
process.HITrackCorrections_byPdgId.qualityString = cms.string("highPurity")
process.HITrackCorrections_byPdgId.pfCandSrc = cms.InputTag("particleFlow")
process.HITrackCorrections_byPdgId.jetSrc = cms.InputTag("ak4CaloJets")
# options
process.HITrackCorrections_byPdgId.useCentrality = False
process.HITrackCorrections_byPdgId.applyTrackCuts = True
if options.usePixelTrks == True:
    process.HITrackCorrections_byPdgId.applyTrackCuts = False
process.HITrackCorrections_byPdgId.fillNTuples = False
process.HITrackCorrections_byPdgId.applyVertexZCut = False
process.HITrackCorrections_byPdgId.doVtxReweighting = False
process.HITrackCorrections_byPdgId.doCaloMatched = False
# cut values
process.HITrackCorrections_byPdgId.dxyErrMax = 3.0 #default 3.0
process.HITrackCorrections_byPdgId.dzErrMax = 3.0 # default 3.0
process.HITrackCorrections_byPdgId.ptErrMax = 9999999999999.0 # no cut basically
process.HITrackCorrections_byPdgId.nhitsMin = 0 # default 0 
process.HITrackCorrections_byPdgId.chi2nMax = 9999999999999.0 # no cut basically
process.HITrackCorrections_byPdgId.reso = 0.5 # not applying, just ignore

#process.HITrackCorrections_byPdgId.crossSection = 1.0 #1.0 is no reweigh
#algo
process.HITrackCorrections_byPdgId.algoParameters = cms.vint32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46)
# vertex reweight parameters
process.HITrackCorrections_byPdgId.vtxWeightParameters = cms.vdouble(0.0306789, 0.427748, 5.16555, 0.0228019, -0.02049, 7.01258 )
###
from Configuration.AlCa.GlobalTag import GlobalTag
if (options.sample == "MC_MiniAOD" or options.sample == "MC_RecoDebug" or options.sample == "MC_Reco_AOD"):
    #process.GlobalTag = GlobalTag(process.GlobalTag, '132X_mcRun3_2023_realistic_HI_v9', '') # for 2023 
    process.GlobalTag = GlobalTag(process.GlobalTag, '141X_mcRun3_2024_realistic_HI_v5', '') # for 2024
    process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")

if (options.sample == "Data_Reco_AOD" or options.sample == "Data_MiniAOD"):
    #process.GlobalTag = GlobalTag(process.GlobalTag, '132X_dataRun3_Prompt_v7', '') # 2023
    #process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '') # 2024
    #process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Express_v3', '') # 2024
    process.GlobalTag = GlobalTag(process.GlobalTag, '141X_dataRun3_Prompt_v3', '')
    process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
    
#forest style analyzers (anaTrack module) (not affected by HITrackCorrections code)
process.load('HITrackingStudies.AnalyzerCode.trackAnalyzer_cff')
process.anaTrack.useCentrality = False
process.anaTrack.trackSrc = 'generalTracks'
process.anaTrack.mvaSrc = cms.InputTag("generalTracks","MVAValues")
if options.usePixelTrks == True:
    process.anaTrack.trackSrc = 'hiConformalPixelTracks'
    process.anaTrack.doMVA = cms.bool(False)
process.anaTrack.doSimTrack = True
process.anaTrack.doSimVertex = True
process.anaTrack.fillSimTrack = True
process.anaTrack.doPFMatching = False
process.anaTrack.doHighestPtVertex = False
process.anaTrack.doTrackVtxWImpPar = False
process.anaSeq = cms.Sequence(process.anaTrack)
if (options.sample == "MC_Reco_AOD" or options.sample == "MC_MiniAOD" or options.sample == "Data_Reco_AOD" or options.sample == "Data_MiniAOD"):
    process.anaTrack.doSimTrack = False
    process.anaTrack.doSimVertex = False
    process.anaTrack.fillSimTrack = False
    if (options.sample == "MC_MiniAOD" or options.sample == "Data_MiniAOD"):
        process.load('HITrackingStudies.AnalyzerCode.unpackedTracksAndVertices_cfi')
        process.anaTrack.trackSrc = 'unpackedTracksAndVertices'
        process.anaTrack.vertexSrc = cms.vstring(['unpackedTracksAndVertices'])
        process.anaTrack.doMVA = cms.bool(False)
        process.hiPixelTracks = process.unpackedTracksAndVertices.clone(
                packedCandidates = cms.VInputTag("hiPixelTracks"),
                packedCandidateNormChi2Map = cms.VInputTag(""),
                secondaryVertices = ""
        )
        if options.usePixelTrks == True:
           process.anaTrack.trackSrc = 'hiPixelTracks'
           process.anaTrack.vertexSrc = cms.vstring(['hiPixelTracks'])
        process.anaSeq = cms.Sequence(process.unpackedTracksAndVertices * process.hiPixelTracks * process.anaTrack)

###trigger selection
import HLTrigger.HLTfilters.hltHighLevel_cfi
process.hltMB = HLTrigger.HLTfilters.hltHighLevel_cfi.hltHighLevel.clone()
#process.hltMB.HLTPaths = ["HLT_HIMinimumBiasHF1AND*"] # for 2023
process.hltMB.HLTPaths = ["HLT_HIMinimumBiasHF1AND*"] # for 2024 #
#process.hltMB.HLTPaths = ["HLT_HIMinimumBiasHF1ANDZDC1nOR_v4"] # for 2024
process.hltMB.andOr = cms.bool(True)  # True = OR, False = AND between the HLT paths
process.hltMB.throw = cms.bool(False) # throw exception on unknown path names

# __________________ Event selection _________________                                                                                                
# Add PbPb collision event selection

# event analysis
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.load('HeavyIonsAnalysis.EventAnalysis.hffilter_cfi')

# Define the event selection sequence
process.eventFilter = cms.Sequence(
    process.phfCoincFilter2Th4 *
    process.primaryVertexFilter *
    process.clusterCompatibilityFilter
)

process.p = cms.Path(
    process.tpClusterProducer *
    process.quickTrackAssociatorByHits *
    process.tpRecoAssocGeneralTracks *
#    process.centralityBin *
#    process.hltMB *
    process.HITrackCorrections_byPdgId
#    process.anaSeq ## comment if you want to save only histograms
)

if (options.sample == "MC_Reco_AOD" or options.sample == "Data_Reco_AOD" or options.sample == "MC_MiniAOD" or options.sample == "Data_MiniAOD"):
    process.p = cms.Path(
#        process.eventFilter*
#        process.hltMB *
#        process.centralityBin *
        process.anaSeq
    )
