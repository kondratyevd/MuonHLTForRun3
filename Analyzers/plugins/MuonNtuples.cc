/** \class MuonNtuples
 */
      
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/Luminosity/interface/LumiDetails.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/RecoCandidate/interface/IsoDeposit.h"
#include "DataFormats/RecoCandidate/interface/IsoDepositFwd.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidate.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateFwd.h"
#include "DataFormats/RecoCandidate/interface/RecoChargedCandidateIsolation.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Scalers/interface/LumiScalers.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "HLTrigger/HLTcore/interface/HLTEventAnalyzerAOD.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include <map>
#include <string>
#include <iomanip>
#include "TTree.h"
#include "HLTrigger/Analyzers/src/MuonTree.h"
#include "DataFormats/MuonReco/interface/MuonTrackLinks.h"
#include "DataFormats/TrackerRecHit2D/interface/BaseTrackerRecHit.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit1D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2D.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"
#include "DataFormats/TrackerRecHit2D/interface/OmniClusterRef.h"

#include "TrackingTools/PatternTools/interface/Trajectory.h"

//#include "SimTracker/TrackAssociation/interface/TrackAssociatorBase.h"

#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticle.h"
#include "SimDataFormats/TrackingAnalysis/interface/TrackingParticleFwd.h"

#include "TrackingTools/GeomPropagators/interface/StateOnTrackerBound.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"

class MuonServiceProxy;

enum HLTCollectionType { 
  iL2muons=0,
  iL3muons,
  iL3NoIDmuons,
  itkmuons,
  iL3OImuons,
  iL3IOmuons,
};


//*********INCLUDED*********// 
enum TrackCollectionType {
  ihltTrackOI=0,
  ihltTrackIOL1,
  ihltTrackIOL2,
};
//**************************//


const double NOMATCH = 999.;
const double NOMATCHITS =  0.;



class MuonNtuples : public edm::EDAnalyzer {

 public:
  MuonNtuples(const edm::ParameterSet& cfg);
  virtual ~MuonNtuples() {};

  virtual void analyze (const edm::Event& event, const edm::EventSetup & eventSetup);
  virtual void beginJob();
  virtual void endJob();
  virtual void beginEvent();
  virtual void beginRun(const edm::Run & run,    const edm::EventSetup & eventSetup);
  virtual void endRun  (const edm::Run & run,    const edm::EventSetup & eventSetup);


 private:

  void fillHlt(const edm::Handle<edm::TriggerResults> &, 
               const edm::Handle<trigger::TriggerEvent> &,
               const edm::TriggerNames &,
               const edm::Event &,
               bool 
              );

  //*****************************INCLUDED*****************************//
  void fillHltTrack(const edm::Handle<reco::TrackCollection>  &,
		    const edm::Event    &,
                    TrackCollectionType type
		    );
  //******************************************************************//

  void fillHltMuons(const edm::Handle<reco::RecoChargedCandidateCollection> &,
                    const edm::Event   &,
                    HLTCollectionType type,
                    const edm::EventSetup &
                   );

  void fillHltMuons(const edm::Handle<reco::MuonCollection> &,
                    const edm::Event   &,
		    HLTCollectionType type
                   );

  void fillL1Muons(const edm::Handle<l1t::MuonBxCollection> &,
                    const edm::Event   &
                   );

  void MonteCarloStudies(const edm::Event&);
  

//   virtual void endEvent();

  edm::InputTag offlinePVTag_;
  edm::EDGetTokenT<reco::VertexCollection> offlinePVToken_;
  edm::InputTag offlineMuonTag_;
  edm::EDGetTokenT<std::vector<reco::Muon>> offlineMuonToken_;
  /// file service
  edm::Service<TFileService> outfile_;

//*****************************INCLUDED*****************************//
  edm::InputTag theTrackOITag_;
  edm::EDGetTokenT<reco::TrackCollection> theTrackOIToken_;
  edm::InputTag theTrajOITag_;
  edm::EDGetTokenT<std::vector<Trajectory>> theTrajOIToken_;
  edm::InputTag theTrackIOL2Tag_;
  edm::EDGetTokenT<reco::TrackCollection> theTrackIOL2Token_;
  edm::InputTag theTrackIOL1Tag_;
  edm::EDGetTokenT<reco::TrackCollection> theTrackIOL1Token_;
  edm::InputTag simTrackTag_;
  edm::EDGetTokenT<std::vector<TrackingParticle>> simTrackToken_;
//******************************************************************//


  // Trigger process
  edm::InputTag triggerResultTag_;
  edm::EDGetTokenT<edm::TriggerResults>   triggerResultToken_;
  edm::InputTag triggerSummTag_;
  edm::EDGetTokenT<trigger::TriggerEvent> triggerSummToken_;
  edm::InputTag tagTriggerResultTag_;
  edm::EDGetTokenT<edm::TriggerResults>   tagTriggerResultToken_;
  edm::InputTag tagTriggerSummTag_;
  edm::EDGetTokenT<trigger::TriggerEvent> tagTriggerSummToken_;

  // Input tags
  edm::InputTag l3candTag_;
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> l3candToken_;
  edm::InputTag l3candNoIDTag_;
  edm::EDGetTokenT<std::vector<reco::Muon>> l3candNoIDToken_; 
  edm::InputTag l2candTag_;
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> l2candToken_; 
  edm::InputTag l1candTag_;
  edm::EDGetTokenT<l1t::MuonBxCollection> l1candToken_; 
  edm::InputTag tkMucandTag_;
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> tkMucandToken_; 
  edm::InputTag l3OIcandTag_;
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> l3OIcandToken_; 
  edm::InputTag l3IOcandTag_;
  edm::EDGetTokenT<reco::RecoChargedCandidateCollection> l3IOcandToken_; 

  edm::EDGetTokenT<reco::MuonTrackLinksCollection>  theLinksToken_;
  edm::EDGetTokenT<reco::TrackExtraCollection>  theMuonsWithHitsToken_;
 


  //trigger process
  std::string triggerProcess_;
  std::string triggerName_;
  std::string l1filterLabel_;
  std::string l2filterLabel_;
  std::string l3filterLabel_;



  edm::InputTag lumiScalerTag_;
  edm::EDGetTokenT<LumiScalersCollection> lumiScalerToken_;

  edm::InputTag puTag_;
  edm::EDGetTokenT<std::vector< PileupSummaryInfo>> puToken_;

  edm::InputTag genTag_;
  edm::EDGetTokenT<reco::GenParticleCollection> genToken_;


  //edm::EDGetTokenT<std::vector<SeedCandidate>>  theSeedLabel;


  bool doOffline_;
  std::string propagatorName_;
  MuonEvent event_;
  std::map<std::string,TTree*> tree_;
  
  unsigned int nGoodVtx; 
  MuonServiceProxy *theService;

};

/// default constructor
MuonNtuples::MuonNtuples(const edm::ParameterSet& cfg): 
  offlinePVTag_           (cfg.getParameter<edm::InputTag>("offlineVtx")), 
  offlinePVToken_         (consumes<reco::VertexCollection>(offlinePVTag_)), 
  offlineMuonTag_         (cfg.getParameter<edm::InputTag>("offlineMuons")),
  offlineMuonToken_       (consumes<std::vector<reco::Muon>>(offlineMuonTag_)),

//********************************************INCLUDED********************************************//
  theTrackOITag_          (cfg.getUntrackedParameter<edm::InputTag>("theTrackOI")),
    theTrackOIToken_        (consumes<reco::TrackCollection>(theTrackOITag_)),
  theTrajOITag_          (cfg.getUntrackedParameter<edm::InputTag>("theTrajOI")),
   theTrajOIToken_        (consumes<std::vector<Trajectory>>(theTrajOITag_)),
  theTrackIOL2Tag_          (cfg.getUntrackedParameter<edm::InputTag>("theTrackIOL2")),
    theTrackIOL2Token_        (consumes<reco::TrackCollection>(theTrackIOL2Tag_)),
  theTrackIOL1Tag_          (cfg.getUntrackedParameter<edm::InputTag>("theTrackIOL1")),
    theTrackIOL1Token_        (consumes<reco::TrackCollection>(theTrackIOL1Tag_)),

  simTrackTag_          (cfg.getUntrackedParameter<edm::InputTag>("simTracks")),
   simTrackToken_        (consumes<std::vector<TrackingParticle>>(simTrackTag_)),
//************************************************************************************************//


  triggerResultTag_       (cfg.getUntrackedParameter<edm::InputTag>("triggerResult")), 
    triggerResultToken_     (consumes<edm::TriggerResults>(triggerResultTag_)),
  triggerSummTag_         (cfg.getUntrackedParameter<edm::InputTag>("triggerSummary")), 
    triggerSummToken_       (consumes<trigger::TriggerEvent>(triggerSummTag_)),
   //l3filterLabel_          (cfg.getParameter<std::string>("l3filterLabel")),

  tagTriggerResultTag_    (cfg.getUntrackedParameter<edm::InputTag>("tagTriggerResult")), 
    tagTriggerResultToken_  (consumes<edm::TriggerResults>(tagTriggerResultTag_)),
  tagTriggerSummTag_      (cfg.getUntrackedParameter<edm::InputTag>("tagTriggerSummary")), 
    tagTriggerSummToken_    (consumes<trigger::TriggerEvent>(tagTriggerSummTag_)),
  // triggerProcess_         (cfg.getParameter<std::string>("triggerProcess")),

  l3candTag_              (cfg.getUntrackedParameter<edm::InputTag>("L3Candidates")),
    l3candToken_            (consumes<reco::RecoChargedCandidateCollection>(l3candTag_)),
  l3candNoIDTag_              (cfg.getUntrackedParameter<edm::InputTag>("L3CandidatesNoID")),
    l3candNoIDToken_            (consumes<std::vector<reco::Muon>>(l3candNoIDTag_)),
  l2candTag_              (cfg.getUntrackedParameter<edm::InputTag>("L2Candidates")),
    l2candToken_            (consumes<reco::RecoChargedCandidateCollection>(l2candTag_)),
  l1candTag_              (cfg.getUntrackedParameter<edm::InputTag>("L1Candidates")),
    l1candToken_            (consumes<l1t::MuonBxCollection>(l1candTag_)),
  tkMucandTag_            (cfg.getUntrackedParameter<edm::InputTag>("TkMuCandidates")),
    tkMucandToken_          (consumes<reco::RecoChargedCandidateCollection>(tkMucandTag_)),
  l3OIcandTag_            (cfg.getUntrackedParameter<edm::InputTag>("L3OIMuCandidates")),
    l3OIcandToken_          (consumes<reco::RecoChargedCandidateCollection>(l3OIcandTag_)),
  l3IOcandTag_          (cfg.getUntrackedParameter<edm::InputTag>("L3IOMuCandidates")),
    l3IOcandToken_        (consumes<reco::RecoChargedCandidateCollection>(l3IOcandTag_)),

  theLinksToken_          (consumes<reco::MuonTrackLinksCollection>(cfg.getUntrackedParameter<edm::InputTag>("MuonLinksTag"))),
  theMuonsWithHitsToken_  (consumes<reco::TrackExtraCollection>(edm::InputTag("globalMuons"))),


  lumiScalerTag_          (cfg.getUntrackedParameter<edm::InputTag>("lumiScalerTag")),
    lumiScalerToken_        (consumes<LumiScalersCollection>(lumiScalerTag_)), 
  puTag_                  (cfg.getUntrackedParameter<edm::InputTag>("puInfoTag")),
    puToken_                (consumes<std::vector< PileupSummaryInfo>>(puTag_)), 

  genTag_                 (cfg.getUntrackedParameter<edm::InputTag>("genParticlesTag")),
    genToken_               (consumes<reco::GenParticleCollection>(genTag_)), 

  doOffline_                 (cfg.getUntrackedParameter<bool>("doOffline")),
    propagatorName_        (cfg.getParameter<std::string>("propagatorName"))
{

  theService = new MuonServiceProxy(cfg.getParameter<edm::ParameterSet>("ServiceParameters"), consumesCollector());

}

void MuonNtuples::beginJob() {

  TH1::SetDefaultSumw2() ;
  tree_["muonTree"] = outfile_-> make<TTree>("muonTree","muonTree");
  tree_["muonTree"] -> Branch("event" ,&event_, 64000,2);

}    

void MuonNtuples::endJob() {}

void MuonNtuples::beginRun(const edm::Run & run, const edm::EventSetup & eventSetup) {
}

void MuonNtuples::endRun  (const edm::Run & run, const edm::EventSetup & eventSetup) {}
 
void MuonNtuples::analyze (const edm::Event &event, const edm::EventSetup &eventSetup) {

  theService->update(eventSetup);

  beginEvent();

  // Fill general info
  event_.runNumber             = event.id().run();
  event_.luminosityBlockNumber = event.id().luminosityBlock();
  event_.eventNumber           = event.id().event();

  if (doOffline_){

    // Fill bx and inst lumi info
    if (event.isRealData()) {
      event_.bxId  = event.bunchCrossing();

      if (lumiScalerTag_.label() != "none")
      {
        edm::Handle<LumiScalersCollection> lumiScaler;
        event.getByToken(lumiScalerToken_, lumiScaler);

        if (lumiScaler->begin() != lumiScaler->end())
          event_.instLumi = lumiScaler->begin()->instantLumi();
      } 
    }

  // Fill PU info
  if (!event.isRealData()) {
    edm::Handle<std::vector< PileupSummaryInfo > > puInfo;
    if ( event.getByToken(puToken_,puInfo)){
      std::vector<PileupSummaryInfo>::const_iterator PVI;
      for(PVI = puInfo->begin(); PVI != puInfo->end(); ++PVI) 
      {
        if(PVI->getBunchCrossing()==0){
          event_.trueNI   = PVI->getTrueNumInteractions();
          continue;
        }
      }
    } 
    else  
      edm::LogError("") << "PU collection not found !!!";
  }

  
  // Fill MC GEN info
  if (!event.isRealData()) 
    MonteCarloStudies(event);

  //Track Outside-In
  edm::Handle<reco::TrackCollection> trackOI;
  event.getByToken(theTrackOIToken_, trackOI);
  
  if (event.getByToken(theTrackOIToken_, trackOI))
    fillHltTrack(trackOI, event, TrackCollectionType::ihltTrackOI);

  //Trajectory outside-in
    edm::Handle<std::vector<Trajectory>> trajOI;
    event.getByToken(theTrajOIToken_, trajOI);

  //Track Inside Out from L2
  edm::Handle<reco::TrackCollection> trackIOL2;
  if (event.getByToken(theTrackIOL2Token_, trackIOL2))
    fillHltTrack(trackIOL2, event, TrackCollectionType::ihltTrackIOL2);
  
  //Track Outside In from L1
  edm::Handle<reco::TrackCollection> trackIOL1;
  if (event.getByToken(theTrackIOL1Token_, trackIOL1))
    fillHltTrack(trackIOL1, event, TrackCollectionType::ihltTrackIOL1);
  
  
  // Fill trigger information for probe muon
  edm::Handle<edm::TriggerResults>   triggerResults;
  edm::Handle<trigger::TriggerEvent> triggerEvent;

  if (event.getByToken(triggerResultToken_, triggerResults) &&
      event.getByToken(triggerSummToken_  , triggerEvent)) {
      
    edm::TriggerNames triggerNames_ = event.triggerNames(*triggerResults);
    fillHlt(triggerResults, triggerEvent, triggerNames_, event, false);
  }
  else 
    edm::LogError("") << "Trigger collection for probe muon not found !!!";

  // Fill trigger information for tag muon
  edm::Handle<edm::TriggerResults>   tagTriggerResults;
  edm::Handle<trigger::TriggerEvent> tagTriggerEvent;
      
  if (event.getByToken(tagTriggerResultToken_, tagTriggerResults) &&
      event.getByToken(tagTriggerSummToken_  , tagTriggerEvent)) {
      
    edm::TriggerNames tagTriggerNames_ = event.triggerNames(*tagTriggerResults);
    fillHlt(tagTriggerResults, tagTriggerEvent, tagTriggerNames_, event, true);
  }
  else 
    edm::LogError("") << "Trigger collection for tag muon not found !!!";

 // Handle the online muon collection and fill online muons //the hltmuons branch
  edm::Handle<reco::RecoChargedCandidateCollection> l3cands;
  if (event.getByToken(l3candToken_, l3cands))
    fillHltMuons(l3cands, event, HLTCollectionType::iL3muons, eventSetup);

 // Handle the 2nd online muon collection and fill online muons //the hltmuons branch
  edm::Handle<reco::MuonCollection> l3candsNoID;
  if (event.getByToken(l3candNoIDToken_, l3candsNoID))
    fillHltMuons(l3candsNoID, event, HLTCollectionType::iL3NoIDmuons);

  // Handle the online muon collection and fill L2 muons //the l2muosn branch
  edm::Handle<reco::RecoChargedCandidateCollection> l2cands;
  if (event.getByToken(l2candToken_, l2cands))
    fillHltMuons(l2cands, event, HLTCollectionType::iL2muons, eventSetup);
//  else
//    edm::LogWarning("") << "Online L2 muon collection not found !!!";

  // Handle the online muon collection and fill L1 muons
  edm::Handle<l1t::MuonBxCollection> l1cands;
  if (event.getByToken(l1candToken_, l1cands))
    fillL1Muons(l1cands, event);
//  else
//    edm::LogWarning("") << "Online L1 muon collection not found !!!";
  
  // Handle the 2nd online muon collection and fill online muons
  edm::Handle<reco::RecoChargedCandidateCollection> tkMucands;
  if (event.getByToken(tkMucandToken_, tkMucands))
    fillHltMuons(tkMucands, event, HLTCollectionType::itkmuons, eventSetup);
//  else
//    edm::LogWarning("") << "Online tracker muon collection not found !!!";
//

  // Handle the online muon collection and fill online muons //the hltmuons branch
  edm::Handle<reco::RecoChargedCandidateCollection> l3OIcands;
  event.getByToken(l3OIcandToken_, l3OIcands);
  if (event.getByToken(l3OIcandToken_, l3OIcands))
    fillHltMuons(l3OIcands, event, HLTCollectionType::iL3OImuons, eventSetup);

   //if (l3OIcands.isValid())std::cout<<"OI cand size: "<<l3OIcands->size()<<std::endl;
//  else
//    edm::LogWarning("") << "Online OI muon collection not found !!!"; 

  edm::Handle<reco::RecoChargedCandidateCollection> l3IOcands;
  event.getByToken(l3IOcandToken_, l3IOcands);
  if (event.getByToken(l3IOcandToken_, l3IOcands))
    fillHltMuons(l3IOcands, event, HLTCollectionType::iL3IOmuons, eventSetup);
//  else
//    edm::LogWarning("") << "Online IO muon collection not found !!!"; 

  edm::Handle<reco::MuonTrackLinksCollection> links;
  event.getByToken(theLinksToken_, links);

  //edm::Handle<reco::TrackExtraCollection> trackMuons;
  //event.getByToken(theMuonsWithHitsToken_, trackMuons);

  edm::Handle<trigger::TriggerEvent> triggerSummary;
  event.getByToken(triggerSummToken_, triggerSummary);

  size_t L3MuonFilterIndex = (*triggerSummary).filterIndex(edm::InputTag("Mu","","TEST"));
  
  trigger::TriggerObjectCollection L3MuonTrigObjects;
  trigger::TriggerObjectCollection allTriggerObjects = triggerSummary->getObjects();


     if(L3MuonFilterIndex < (*triggerSummary).sizeFilters()) { //check if the trigger object is present
    const trigger::Keys &keysMuons = (*triggerSummary).filterKeys(L3MuonFilterIndex);
    for(size_t j = 0; j < keysMuons.size(); j++) {
      trigger::TriggerObject foundObject = (allTriggerObjects)[keysMuons[j]];
      L3MuonTrigObjects.push_back(foundObject);
    }
  }

  } // close if(doOffline)

  if(!event.isRealData()) { // for MC GEN-SIM-RAW

  //Track Outside-In
    edm::Handle<reco::TrackCollection> trackOI;
    bool trackOIflag = false;
    if (event.getByToken(theTrackOIToken_, trackOI))
      trackOIflag = true;

    if (trackOIflag)
      fillHltTrack(trackOI, event, TrackCollectionType::ihltTrackOI);

  //Trajectory outside-in
    edm::Handle<std::vector<Trajectory>> trajOI;
    if(event.getByToken(theTrajOIToken_, trajOI)){
      for(std::vector<Trajectory>::const_iterator t=trajOI->begin(); t!=trajOI->end(); t++){
      }
    }

    // Handle the online muon collection and fill L2 muons //the l2muosn branch
    edm::Handle<reco::RecoChargedCandidateCollection> l2cands;
    if (event.getByToken(l2candToken_, l2cands))
      fillHltMuons(l2cands, event, HLTCollectionType::iL2muons, eventSetup);
  }

  // endEvent();
  tree_["muonTree"] -> Fill();
}



//------------------------------------------------------------------------
void MuonNtuples::MonteCarloStudies(const edm::Event& event)
{
  edm::Handle<reco::GenParticleCollection> genParticles;
  event.getByToken(genToken_, genParticles);
  int muId  =    13;

  for ( size_t i=0; i< genParticles->size(); ++i) 
  { 
    const reco::GenParticle &p = (*genParticles)[i];
    // only save muons
    if(fabs(p.pdgId()) != muId )     continue; 
      
    GenParticleCand theGen;
    theGen.pdgId  = p.pdgId();
    theGen.pt     = p.pt() ;
    theGen.eta    = p.eta();
    theGen.phi    = p.phi();
    theGen.energy = p.energy();
    theGen.status = p.status();
    
    unsigned int n_moms = p.numberOfMothers();
    if (n_moms == 0 ){
      theGen.pdgMother.push_back(0);
      theGen.pdgRealMother.push_back(0);
    }
    else {
      for (unsigned int im=0; im < n_moms; ++im){
        theGen.pdgMother.push_back(p.motherRef(im)->pdgId());
        // if coming from a muon, go back one step ** to be improved **
        if(n_moms == 1 && fabs(p.motherRef(0)->pdgId()) == muId){
          for (unsigned int igm = 0; igm < p.motherRef(0)->numberOfMothers(); igm++){
            theGen.pdgRealMother.push_back(p.motherRef(0)->motherRef(igm)->pdgId());
          }
        }
        else
          theGen.pdgRealMother.push_back(0);
      }
    }

    event_.genParticles.push_back(theGen);

  }  // end for genParticles
}



// --------------------------------------------------------------------
void MuonNtuples::fillHlt(const edm::Handle<edm::TriggerResults>   & triggerResults, 
                          const edm::Handle<trigger::TriggerEvent> & triggerEvent  ,
                          const edm::TriggerNames                  & triggerNames  ,
                          const edm::Event                         & event         ,
                          bool                                       isTag         )
{    
   
  for (unsigned int itrig=0; itrig < triggerNames.size(); ++itrig) 
  {
    LogDebug ("triggers") << triggerNames.triggerName(itrig) ;
    if (triggerResults->accept(itrig)) 
    {
      std::string pathName = triggerNames.triggerName(itrig);
      
//      if ( pathName.find ("HLT_IsoMu"  ) !=std::string::npos ||
//           pathName.find ("HLT_Mu"     ) !=std::string::npos ||
//           pathName.find ("HLT_Mu5"    ) !=std::string::npos ||
//           pathName.find ("HLT_TkMu5"  ) !=std::string::npos ||
//           pathName.find ("HLT_IsoTkMu") !=std::string::npos ||
//           pathName.find ("HLT_Mu17"   ) !=std::string::npos ||
//           pathName.find ("HLT_Mu8_T"  ) !=std::string::npos 
//      ){
      if (isTag) event_.hltTag.triggers.push_back(pathName);
      else       event_.hlt   .triggers.push_back(pathName);
//      }
    }
  }
     
     
  const trigger::size_type nFilters(triggerEvent->sizeFilters());
  for (trigger::size_type iFilter=0; iFilter!=nFilters; ++iFilter) 
  {
    std::string filterTag = triggerEvent->filterTag(iFilter).encode();

    if ( ( filterTag.find ("Mu"     ) !=std::string::npos)
	 && 
	 filterTag.find ("Tau"       ) ==std::string::npos   &&
	 filterTag.find ("EG"        ) ==std::string::npos   &&
	 filterTag.find ("MultiFit"  ) ==std::string::npos  
	 )   
    {
      trigger::Keys objectKeys = triggerEvent->filterKeys(iFilter);
      const trigger::TriggerObjectCollection& triggerObjects(triggerEvent->getObjects());


      
      for (trigger::size_type iKey=0; iKey<objectKeys.size(); ++iKey) 
      {  
        trigger::size_type objKey = objectKeys.at(iKey);
        const trigger::TriggerObject& triggerObj(triggerObjects[objKey]);
        
        HLTObjCand hltObj;
        
        hltObj.filterTag = filterTag;
  
        hltObj.pt  = triggerObj.pt();
        hltObj.eta = triggerObj.eta();
        hltObj.phi = triggerObj.phi();
        
        if (isTag)       event_.hltTag.objects.push_back(hltObj);
        else             event_.hlt   .objects.push_back(hltObj);
      }  
    }       
  }
}


// ---------------------------------------------------------------------
//**********************************************INCLUDED*********************************************//

void MuonNtuples::fillHltTrack(const edm::Handle<reco::TrackCollection>  & trackm ,
			       const edm::Event                          & tevent ,
			       TrackCollectionType type)
{
  int t_mu=0;
  for(reco::TrackCollection::const_iterator t=trackm->begin(); t!=trackm->end(); t++){

    t_mu++;
    HltTrackCand MuTrack;

    MuTrack.pt   = t -> pt() ;
    MuTrack.eta  = t -> eta();
    MuTrack.phi  = t -> phi();

    MuTrack.chi2           = t  ->  normalizedChi2();
    MuTrack.dxy            = t  ->  dxy();
    MuTrack.dz             = t  ->  dz() ;
    MuTrack.pixelHits      = t  ->  hitPattern().numberOfValidPixelHits();
    MuTrack.validHits      = t  ->  numberOfValidHits();

    MuTrack.layerHits         = t -> hitPattern().trackerLayersWithMeasurement(); 
    MuTrack.pixelLayers       = t -> hitPattern().pixelLayersWithMeasurement(); 
    MuTrack.fracValidTrackhit = t -> validFraction();  
  
    if (type == TrackCollectionType::ihltTrackOI)   {
        MuTrack.covMat_00 = t -> covariance(0, 0);
        MuTrack.covMat_01 = t -> covariance(0, 1);
        MuTrack.covMat_02 = t -> covariance(0, 2);
        MuTrack.covMat_03 = t -> covariance(0, 3);
        MuTrack.covMat_04 = t -> covariance(0, 4);
        
        MuTrack.covMat_10 = t -> covariance(1, 0);
        MuTrack.covMat_11 = t -> covariance(1, 1);
        MuTrack.covMat_12 = t -> covariance(1, 2);
        MuTrack.covMat_13 = t -> covariance(1, 3);
        MuTrack.covMat_14 = t -> covariance(1, 4);
        
        MuTrack.covMat_20 = t -> covariance(2, 0);
        MuTrack.covMat_21 = t -> covariance(2, 1);
        MuTrack.covMat_22 = t -> covariance(2, 2);
        MuTrack.covMat_23 = t -> covariance(2, 3);
        MuTrack.covMat_24 = t -> covariance(2, 4);
        
        MuTrack.covMat_30 = t -> covariance(3, 0);
        MuTrack.covMat_31 = t -> covariance(3, 1);
        MuTrack.covMat_32 = t -> covariance(3, 2);
        MuTrack.covMat_33 = t -> covariance(3, 3);
        MuTrack.covMat_34 = t -> covariance(3, 4);

        MuTrack.covMat_40 = t -> covariance(4, 0);
        MuTrack.covMat_41 = t -> covariance(4, 1);
        MuTrack.covMat_42 = t -> covariance(4, 2);
        MuTrack.covMat_43 = t -> covariance(4, 3);
        MuTrack.covMat_44 = t -> covariance(4, 4);

        event_.hltTrackOI.push_back(MuTrack)  ;  continue; }
    if (type == TrackCollectionType::ihltTrackIOL1) {event_.hltTrackIOL1.push_back(MuTrack);  continue; }
    if (type == TrackCollectionType::ihltTrackIOL2) {event_.hltTrackIOL2.push_back(MuTrack);  continue; }

  }
}
//****************************************************************************************************//


// ---------------------------------------------------------------------
void MuonNtuples::fillHltMuons(const edm::Handle<reco::RecoChargedCandidateCollection> & l3cands , //candidates to HLT 
                               const edm::Event                                        & event   , 
                               HLTCollectionType type,
                               const edm::EventSetup & iSetup
                               )
{

  for( unsigned int il3 = 0; il3 < l3cands->size(); ++il3) {
    HLTMuonCand theL3Mu;
    reco::RecoChargedCandidateRef candref(l3cands, il3);
    reco::TrackRef candTrackRef = candref->track();
    theL3Mu.pt      = candref -> pt();
    theL3Mu.eta     = candref -> eta();
    theL3Mu.phi     = candref -> phi();
    theL3Mu.charge  = candref -> charge();

    reco::TrackRef trkmu = candref->track();
    theL3Mu.trkpt   = trkmu -> pt();
    if (type == HLTCollectionType::iL2muons){
        if (trkmu -> ndof() != 0){ 
            theL3Mu.chi2 = trkmu -> chi2() / trkmu -> ndof();
        } else {
            theL3Mu.chi2 = -1;
        }
        theL3Mu.validHits = trkmu -> found();
        theL3Mu.lostHits = trkmu -> lost();
        
        Plane::PlanePointer dummyPlane = Plane::build(Plane::PositionType(), Plane::RotationType());        
        edm::ESHandle<MagneticField> magfieldH;
        edm::ESHandle<Propagator> propagatorAlongH;
        edm::ESHandle<Propagator> propagatorOppositeH; 
        edm::ESHandle<GlobalTrackingGeometry> geometryH;
        
        iSetup.get<IdealMagneticFieldRecord>().get(magfieldH);
        iSetup.get<TrackingComponentsRecord>().get(propagatorName_, propagatorOppositeH);
        iSetup.get<TrackingComponentsRecord>().get(propagatorName_, propagatorAlongH);
        iSetup.get<GlobalTrackingGeometryRecord>().get(geometryH);
        
        std::unique_ptr<Propagator> propagatorAlong = SetPropagationDirection(*propagatorAlongH, alongMomentum);
        std::unique_ptr<Propagator> propagatorOpposite = SetPropagationDirection(*propagatorOppositeH, oppositeToMomentum);
        edm::ESHandle<Propagator> SHPOpposite;
        iSetup.get<TrackingComponentsRecord>().get("hltESPSteppingHelixPropagatorOpposite", SHPOpposite);
        
        FreeTrajectoryState fts = trajectoryStateTransform::initialFreeState(*candTrackRef, magfieldH.product());

        dummyPlane->move(fts.position() - dummyPlane->position());
        TrajectoryStateOnSurface tsosAtIP = TrajectoryStateOnSurface(fts, *dummyPlane);
        TrajectoryStateOnSurface tsosAtMuonSystem = trajectoryStateTransform::innerStateOnSurface(
            *candTrackRef, *geometryH, magfieldH.product());
        trajectoryStateTransform::innerStateOnSurface(*candTrackRef, *geometryH, magfieldH.product());
        StateOnTrackerBound fromInside(propagatorAlong.get());
        TrajectoryStateOnSurface outerTkStateInside = fromInside(fts);
        StateOnTrackerBound fromOutside(&*SHPOpposite);
        TrajectoryStateOnSurface outerTkStateOutside = fromOutside(tsosAtMuonSystem);
        
        if (tsosAtIP.isValid()){
            theL3Mu.tsos_IP_valid = 1;
            AlgebraicSymMatrix55 matrix_IP = tsosAtIP.curvilinearError().matrix();
            theL3Mu.err0_IP = sqrt(matrix_IP[0][0]);
            theL3Mu.err1_IP = sqrt(matrix_IP[1][1]);
            theL3Mu.err2_IP = sqrt(matrix_IP[2][2]);
            theL3Mu.err3_IP = sqrt(matrix_IP[3][3]);
            theL3Mu.err4_IP = sqrt(matrix_IP[4][4]);

            theL3Mu.covMat_00 = matrix_IP[0][0];
            theL3Mu.covMat_01 = matrix_IP[0][1];
            theL3Mu.covMat_02 = matrix_IP[0][2];
            theL3Mu.covMat_03 = matrix_IP[0][3];
            theL3Mu.covMat_04 = matrix_IP[0][4];
            
            theL3Mu.covMat_10 = matrix_IP[1][0];
            theL3Mu.covMat_11 = matrix_IP[1][1];
            theL3Mu.covMat_12 = matrix_IP[1][2];
            theL3Mu.covMat_13 = matrix_IP[1][3];
            theL3Mu.covMat_14 = matrix_IP[1][4];
            
            theL3Mu.covMat_20 = matrix_IP[2][0];
            theL3Mu.covMat_21 = matrix_IP[2][1];
            theL3Mu.covMat_22 = matrix_IP[2][2];
            theL3Mu.covMat_23 = matrix_IP[2][3];
            theL3Mu.covMat_24 = matrix_IP[2][4];

            theL3Mu.covMat_30 = matrix_IP[3][0];
            theL3Mu.covMat_31 = matrix_IP[3][1];
            theL3Mu.covMat_32 = matrix_IP[3][2];
            theL3Mu.covMat_33 = matrix_IP[3][3];
            theL3Mu.covMat_34 = matrix_IP[3][4];

            theL3Mu.covMat_40 = matrix_IP[4][0];
            theL3Mu.covMat_41 = matrix_IP[4][1];
            theL3Mu.covMat_42 = matrix_IP[4][2];
            theL3Mu.covMat_43 = matrix_IP[4][3];
            theL3Mu.covMat_44 = matrix_IP[4][4];
            
            theL3Mu.tsos_IP_eta = tsosAtIP.globalPosition().eta();
            theL3Mu.tsos_IP_phi = tsosAtIP.globalPosition().phi();
            theL3Mu.tsos_IP_pt = tsosAtIP.globalMomentum().perp();
            theL3Mu.tsos_IP_pt_eta = tsosAtIP.globalMomentum().eta();
            theL3Mu.tsos_IP_pt_phi = tsosAtIP.globalMomentum().phi();
        } else {
            theL3Mu.tsos_IP_valid = 0;
        }
        if (outerTkStateOutside.isValid()){
            theL3Mu.tsos_MuS_valid = 1;
            AlgebraicSymMatrix55 matrix_MuS = outerTkStateOutside.curvilinearError().matrix();
            theL3Mu.err0_MuS = sqrt(matrix_MuS[0][0]);
            theL3Mu.err1_MuS = sqrt(matrix_MuS[1][1]);
            theL3Mu.err2_MuS = sqrt(matrix_MuS[2][2]);
            theL3Mu.err3_MuS = sqrt(matrix_MuS[3][3]);
            theL3Mu.err4_MuS = sqrt(matrix_MuS[4][4]);
            theL3Mu.tsos_MuS_eta = outerTkStateOutside.globalPosition().eta();
            theL3Mu.tsos_MuS_phi = outerTkStateOutside.globalPosition().phi();
            theL3Mu.tsos_MuS_pt = outerTkStateOutside.globalMomentum().perp();
            theL3Mu.tsos_MuS_pt_eta = outerTkStateOutside.globalMomentum().eta();
            theL3Mu.tsos_MuS_pt_phi = outerTkStateOutside.globalMomentum().phi();
        } else {
            theL3Mu.tsos_MuS_valid = 0;
        }

        
    }
    if (type == HLTCollectionType::iL3muons)     { event_.hltmuons    .push_back(theL3Mu);  continue; }
    if (type == HLTCollectionType::iL3OImuons)   { event_.hltOImuons  .push_back(theL3Mu);  continue; }
    if (type == HLTCollectionType::iL3IOmuons)   { event_.hltIOmuons  .push_back(theL3Mu);  continue; }
    if (type == HLTCollectionType::itkmuons)     { event_.tkmuons     .push_back(theL3Mu);  continue; }
    if (type == HLTCollectionType::iL2muons)     { event_.L2muons     .push_back(theL3Mu);  continue; }
  }
}

void MuonNtuples::fillHltMuons(const edm::Handle<reco::MuonCollection> & l3cands , //candidates to HLT 
                               const edm::Event                                        & event   , 
                               HLTCollectionType type
                               )
{
  for(std::vector<reco::Muon>::const_iterator mu1=l3cands->begin(); mu1!=l3cands->end(); ++mu1) 
  {



    HLTMuonCand theL3Mu;

    theL3Mu.pt      = mu1 -> pt();
    theL3Mu.eta     = mu1 -> eta();
    theL3Mu.phi     = mu1 -> phi();
    theL3Mu.charge  = mu1 -> charge();


    theL3Mu.trkpt   = mu1 -> pt();
    if (type == HLTCollectionType::iL3NoIDmuons) { event_.hltNoIDmuons.push_back(theL3Mu);  continue; }
  }
}



// ---------------------------------------------------------------------
void MuonNtuples::fillL1Muons(const edm::Handle<l1t::MuonBxCollection> & l1cands ,
                              const edm::Event                         & event    
                              )
{

  for (int ibx = l1cands->getFirstBX(); ibx <= l1cands->getLastBX(); ++ibx) {
    if (ibx != 0) continue;
    for (auto it = l1cands->begin(ibx); it != l1cands->end(ibx); it++){

      l1t::MuonRef muon(l1cands, distance(l1cands->begin(l1cands->getFirstBX()),it) );

      L1MuonCand theL1Mu;

      theL1Mu.pt       = muon -> pt();
      theL1Mu.eta      = muon -> eta();
      theL1Mu.phi      = muon -> phi();
      theL1Mu.charge   = muon -> charge();
      theL1Mu.quality  = muon -> hwQual();

      event_.L1muons.push_back(theL1Mu);
    }
  }
}




//---------------------------------------------
void MuonNtuples::beginEvent()
{

  event_.hlt.triggers.clear();
  event_.hlt.objects.clear();


  event_.hltTag.triggers.clear();
  event_.hltTag.objects.clear();


  event_.genParticles.clear();

//*******************INCLUDED*******************//
  event_.hltTrackOI.clear();
  event_.hltTrackIOL1.clear();
  event_.hltTrackIOL2.clear();
//**********************************************//
  event_.muons.clear();
  event_.hltmuons.clear();
  event_.hltNoIDmuons.clear();
  event_.L2muons.clear();
  event_.L2muonsTSG.clear();
  event_.L1muons.clear();
  event_.tkmuons.clear();
  event_.hltOImuons.clear();
  event_.hltIOmuons.clear();

  for (unsigned int ix=0; ix<3; ++ix) {
    event_.primaryVertex[ix] = 0.;
    for (unsigned int iy=0; iy<3; ++iy) {
      event_.cov_primaryVertex[ix][iy] = 0.;
    }
  }
  event_.nVtx       = -1;
  event_.trueNI     = -1;
 
  event_.bxId       = -1;
  event_.instLumi   = -1;
  
  nGoodVtx = 0; 
}





// define this as a plug-in
DEFINE_FWK_MODULE(MuonNtuples);
