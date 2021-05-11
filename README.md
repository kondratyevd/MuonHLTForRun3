
# CMS Run 3 Muon HLT

## CMSSW_11_2_0 (without Patatrack)

### Setup
```shell
cmsrel CMSSW_11_2_0
cd CMSSW_11_2_0/src
cmsenv
git cms-init

# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideL1TStage2Instructions
git remote add cms-l1t-offline git@github.com:cms-l1t-offline/cmssw.git
git fetch cms-l1t-offline l1t-integration-CMSSW_11_2_0
git cms-merge-topic -u cms-l1t-offline:l1t-integration-v105.5
git cms-addpkg L1Trigger/Configuration
git cms-addpkg L1Trigger/L1TMuon
git clone https://github.com/cms-l1t-offline/L1Trigger-L1TMuon.git L1Trigger/L1TMuon/data
git cms-addpkg L1Trigger/L1TCalorimeter
git clone https://github.com/cms-l1t-offline/L1Trigger-L1TCalorimeter.git L1Trigger/L1TCalorimeter/data

git cms-checkdeps -A -a

# temporary fix to fill BMTF muons
git remote add panoskatsoulis https://github.com/panoskatsoulis/cmssw.git
git cms-merge-topic panoskatsoulis:fix_empty_bmtf_ntuples

# Customizer for Muon HLT
git cms-addpkg HLTrigger/Configuration
git clone https://github.com/khaosmos93/MuonHLTForRun3.git HLTrigger/Configuration/python/MuonHLTForRun3

scram b -j 8
```

### Obtaining HLT menu
1. hltGetConfiguration (only worked at lxplus for me)
```shell
hltGetConfiguration /dev/CMSSW_11_2_0/GRun --type GRun \
--path HLTriggerFirstPath,HLT_IsoMu24_v*,HLT_Mu50_v*,HLTriggerFinalPath,HLTAnalyzerEndpath \
--unprescale --cff >$CMSSW_BASE/src/HLTrigger/Configuration/python/HLT_MuonHLT_cff.py
```

2. cmsDriver
* Run 3 MC
```shell
cmsDriver.py hlt_muon \
--python_filename=hlt_muon_Run3_mc.py \
--step RAW2DIGI,HLT:MuonHLT \
--process MYHLT --era=Run3 \
--mc --conditions=112X_mcRun3_2021_realistic_v15 \
--customise=L1Trigger/Configuration/customiseReEmul.L1TReEmulMCFromRAW \
--customise=L1Trigger/Configuration/customiseSettings.L1TSettingsToCaloParams_2018_v1_4 \
--customise=HLTrigger/Configuration/MuonHLTForRun3/customizeMuonHLTForRun3.customizeMuonHLTForDoubletRemoval \
--customise=HLTrigger/Configuration/MuonHLTForRun3/customizeMuonHLTForRun3.customizeMuonHLTForCscSegment \
--customise=HLTrigger/Configuration/MuonHLTForRun3/customizeMuonHLTForRun3.customizeMuonHLTForGEM \
--filein=/store/mc/Run3Winter20DRMiniAOD/DYToLL_M-50_TuneCP5_14TeV-pythia8/GEN-SIM-RAW/DRFlatPU30to80_110X_mcRun3_2021_realistic_v6-v2/230001/4A5403D5-F9A2-204D-ADA3-ABF92F6AA673.root \
-n 100 --no_output --no_exec
```
and manually modify
```python
process.GlobalTag = GlobalTag(process.GlobalTag, '112X_mcRun3_2021_realistic_v15', '')
```

to

```python
process.GlobalTag = GlobalTag(process.GlobalTag, '112X_mcRun3_2021_realistic_v15', '')

import os
base = os.environ["CMSSW_BASE"]
process.GlobalTag.toGet = cms.VPSet(
    cms.PSet(record = cms.string("GEMeMapRcd"),
        tag = cms.string("GEMeMapDummy"),
        connect = cms.string("sqlite_file:" + base + "/src/L1Trigger/Configuration/test/GEMeMapDummy.db")
    )
)
process.muonGEMDigis.useDBEMap = True

from RecoMuon.TrackingTools.MuonServiceProxy_cff import *

process.muonNtuples = cms.EDAnalyzer("MuonNtuples",
                   MuonServiceProxy,
                   offlineVtx               = cms.InputTag("offlinePrimaryVertices"),
                   offlineMuons             = cms.InputTag("muons"),
                   triggerResult            = cms.untracked.InputTag("TriggerResults::MYHLT"),
                   triggerSummary           = cms.untracked.InputTag("hltTriggerSummaryAOD::MYHLT"),
                   tagTriggerResult         = cms.untracked.InputTag("TriggerResults::HLT"),
                   tagTriggerSummary        = cms.untracked.InputTag("hltTriggerSummaryAOD::HLT"),
                   triggerProcess   = cms.string("TEST"),
                   L3Candidates             = cms.untracked.InputTag("hltIterL3MuonCandidates"),
                   L3CandidatesNoID         = cms.untracked.InputTag("hltIterL3MuonsNoID"),
                   L2Candidates             = cms.untracked.InputTag("hltL2MuonCandidates"),
                   L1Candidates             = cms.untracked.InputTag('hltGtStage2Digis','Muon'),
                   TkMuCandidates           = cms.untracked.InputTag("hltIterL3OIL3MuonCandidates"),
                   L3OIMuCandidates         = cms.untracked.InputTag("hltIterL3OIL3MuonCandidates"),
                   L3IOMuCandidates         = cms.untracked.InputTag("hltIterL3IOFromL2MuonCandidates"),
                   MuonLinksTag = cms.untracked.InputTag("hltIterL3MuonsFromL2LinksCombination"),
                   globalMuons = cms.InputTag("globalMuons"),
                   #theTrackOI               = cms.untracked.InputTag("hltIterL3OIMuonTrackSelectionHighPurity"),
                   theTrackOI               = cms.untracked.InputTag("hltIterL3OIMuCtfWithMaterialTracks"),
                   theTrackIOL2             = cms.untracked.InputTag("hltIter3IterL3MuonMerged"),
                   theTrackIOL1             = cms.untracked.InputTag("hltIter3IterL3FromL1MuonMerged"),
                   l3filterLabel    = cms.string("hltL3fL1sMu22Or25L1f0L2f10QL3Filtered27Q"),
                   lumiScalerTag            = cms.untracked.InputTag("scalersRawToDigi"),
                   puInfoTag                = cms.untracked.InputTag("addPileupInfo"),
                   genParticlesTag          = cms.untracked.InputTag("genParticles"),
                   doOffline                = cms.untracked.bool(True),
                   seedsForOIFromL2         = cms.InputTag("hltIterL3OISeedsFromL2Muons"),
                   theTrajOI                = cms.untracked.InputTag("hltIterL3OITrackCandidates"),
                   simTracks            = cms.untracked.InputTag("mix","MergedTrackTruth", "HLT"),
                   propagatorName       = cms.string('PropagatorWithMaterialParabolicMf'),
)

process.TFileService = cms.Service("TFileService",
                               fileName = cms.string("muonNtuple_run3_MC.root"),
                               closeFileFast = cms.untracked.bool(False)
)
process.HLTValidation = cms.EndPath(
    process.muonNtuples
)
```

then add the following to "Schedule definition":
```shell
process.schedule.extend([process.HLTValidation])
```


* 2018 data
```shell
cmsDriver.py hlt_muon \
--python_filename=hlt_muon_Run3_data.py \
--step HLT:MuonHLT \
--process MYHLT --era=Run3 \
--data --conditions=auto:run3_data_GRun \
--customise=HLTrigger/Configuration/customizeHLTforCMSSW.customisePixelGainForRun2Input \
--customise=HLTrigger/Configuration/customizeHLTforCMSSW.synchronizeHCALHLTofflineRun3on2018data \
--customise=HLTrigger/Configuration/MuonHLTForRun3/customizeMuonHLTForRun3.customizeMuonHLTForDoubletRemoval \
--customise=HLTrigger/Configuration/MuonHLTForRun3/customizeMuonHLTForRun3.customizeMuonHLTForCscSegment \
--filein=/store/data/Run2018D/EphemeralHLTPhysics1/RAW/v1/000/323/775/00000/0244D183-F28D-2741-9DBF-1638BEDC734E.root \
-n 100 --no_output --no_exec
```

* optionally, add the following lines at the end of the configuration file
```python
process.options.wantSummary = cms.untracked.bool( True )
if 'MessageLogger' in process.__dict__:
    process.MessageLogger.categories.append('TriggerSummaryProducerAOD')
    process.MessageLogger.categories.append('L1GtTrigReport')
    process.MessageLogger.categories.append('L1TGlobalSummary')
    process.MessageLogger.categories.append('HLTrigReport')
    process.MessageLogger.categories.append('FastReport')
    process.MessageLogger.cerr.FwkReport.reportEvery = 1000
```


