#ifndef  MuonTree_h
#define  MuonTree_h

#include "TROOT.h"
#include "TMath.h"
#include <vector>
#include <string>
//#include "DataFormats/TrajectorySeed/interface/SeedCandidate.h"


class GenParticleCand {
public:
  Int_t   pdgId; 
  Int_t   status; 
  Float_t energy; 
  Float_t pt; 
  Float_t eta; 
  Float_t phi; 
  std::vector<Int_t>  pdgMother; 
  std::vector<Int_t>  pdgRealMother; 

  GenParticleCand(){};
  virtual ~GenParticleCand(){};
  
  ClassDef(GenParticleCand,1)
};


//INCLUDED2//

class SeedCand {
 public:
  std::string layerId;
  Int_t layerNum;
  Int_t l2_idx;
  Int_t hitBased;
  Float_t pt;
  Float_t eta;
  Float_t phi;

  Int_t evNum; 

  Float_t tsos_q_p_err;
  Float_t tsos_lambda_err;
  Float_t tsos_phi_err;
  Float_t tsos_xT_err;
  Float_t tsos_yT_err;

  Float_t tsosIP_q_p_err;
  Float_t tsosIP_lambda_err;
  Float_t tsosIP_phi_err;
  Float_t tsosIP_xT_err;
  Float_t tsosIP_yT_err;

  Float_t dR_pos;
  Float_t dR_mom;

  SeedCand(){};
  virtual ~SeedCand(){};

  ClassDef(SeedCand,1)
    };

class HltTrackCand {
 public:

  Float_t pt;
  Float_t eta;
  Float_t phi;

  Double_t  chi2;

  Float_t dxy;
  Float_t dz;
  Float_t fracValidTrackhit;

  Int_t validHits;
  Int_t pixelHits;
  Int_t layerHits;
  Int_t pixelLayers;

  Float_t covMat_00 = -999.;
  Float_t covMat_01 = -999.;
  Float_t covMat_02 = -999.;
  Float_t covMat_03 = -999.;
  Float_t covMat_04 = -999.;
  
  Float_t covMat_10 = -999.;
  Float_t covMat_11 = -999.;
  Float_t covMat_12 = -999.;
  Float_t covMat_13 = -999.;
  Float_t covMat_14 = -999.;

  Float_t covMat_20 = -999.;
  Float_t covMat_21 = -999.;
  Float_t covMat_22 = -999.;
  Float_t covMat_23 = -999.;
  Float_t covMat_24 = -999.;
    
  Float_t covMat_30 = -999.;
  Float_t covMat_31 = -999.;
  Float_t covMat_32 = -999.;
  Float_t covMat_33 = -999.;
  Float_t covMat_34 = -999.;

  Float_t covMat_40 = -999.;
  Float_t covMat_41 = -999.;
  Float_t covMat_42 = -999.;
  Float_t covMat_43 = -999.;
  Float_t covMat_44 = -999.;
    
  HltTrackCand(){};
  virtual ~HltTrackCand(){};

  ClassDef(HltTrackCand,1)
    };


class MuonCand {
public:

  Float_t pt;  
  Float_t eta; 
  Float_t phi; 
  Int_t   charge;    

  Int_t   isGlobal;
  Int_t   isTracker;

  Int_t   isLoose;
  Int_t   isMedium;
  Int_t   isTight;
  std::vector<double> SharedHitFrac;

  Float_t L3pt;
  Float_t L3eta;
  Float_t L3phi;
  Float_t delRmuL3;
  Float_t sharedFrac;
  Float_t sharedFracPixel;
  Float_t sharedFracStrip;

//***************************INCLUDED*********************//
  Double_t  chi2; 
  Double_t innerchi2;

  Float_t validHits;
  Float_t innerpt;
  Float_t innereta;
  Float_t innerphi;
  Float_t innerdxy;
  Float_t innerdz;

  Int_t innervalidHits;

  Float_t dxy;
  Float_t dz;
  Float_t chi2LocalPosition;
  Float_t kickFinder;
  Float_t innerfracValidTrackhit;

  Int_t matchedStations;
  Int_t innerpixelHits;
  Int_t innerlayerHits;
  Int_t innerpixelLayers;
  Int_t isPFMuon;


  //******************************************************//

  Float_t chargedDep_dR03;
  Float_t neutralDep_dR03;
  Float_t photonDep_dR03;
  Float_t puPt_dR03;

  Float_t chargedDep_dR04;
  Float_t neutralDep_dR04;
  Float_t photonDep_dR04;
  Float_t puPt_dR04;

  MuonCand(){};
  virtual ~MuonCand(){};

  ClassDef(MuonCand,1)
};


class HLTMuonCand {
public:

  Float_t pt;           
  Float_t eta;          
  Float_t phi;          
  Float_t dxy;
  Float_t dz;

  Float_t chi2;

  Int_t NumOISeeds;
  Int_t NumOIHitlessSeeds;
  Int_t NumOIHitSeeds;
  Int_t L2ValidHits;


  Float_t trkpt;         // pt of the track of the hlt muon [GeV]
  Int_t   charge;         // pt of the track of the hlt muon [GeV]
 
  Int_t validHits;
  Int_t lostHits;

  Float_t tsos_IP_eta = -999.;
  Float_t tsos_IP_phi = -999.;
  Float_t tsos_IP_pt = -999.;
  Float_t tsos_IP_pt_eta = -999.;
  Float_t tsos_IP_pt_phi = -999.;
  Float_t err0_IP = -999.;
  Float_t err1_IP = -999.;
  Float_t err2_IP = -999.;
  Float_t err3_IP = -999.;
  Float_t err4_IP = -999.;
    
  Float_t covMat_00 = -999.;
  Float_t covMat_01 = -999.;
  Float_t covMat_02 = -999.;
  Float_t covMat_03 = -999.;
  Float_t covMat_04 = -999.;
  
  Float_t covMat_10 = -999.;
  Float_t covMat_11 = -999.;
  Float_t covMat_12 = -999.;
  Float_t covMat_13 = -999.;
  Float_t covMat_14 = -999.;

  Float_t covMat_20 = -999.;
  Float_t covMat_21 = -999.;
  Float_t covMat_22 = -999.;
  Float_t covMat_23 = -999.;
  Float_t covMat_24 = -999.;
    
  Float_t covMat_30 = -999.;
  Float_t covMat_31 = -999.;
  Float_t covMat_32 = -999.;
  Float_t covMat_33 = -999.;
  Float_t covMat_34 = -999.;

  Float_t covMat_40 = -999.;
  Float_t covMat_41 = -999.;
  Float_t covMat_42 = -999.;
  Float_t covMat_43 = -999.;
  Float_t covMat_44 = -999.;
    
  Float_t tsos_MuS_eta = -999.;
  Float_t tsos_MuS_phi = -999.;
  Float_t tsos_MuS_pt = -999.;
  Float_t tsos_MuS_pt_eta = -999.;
  Float_t tsos_MuS_pt_phi = -999.;
  Float_t err0_MuS = -999.;
  Float_t err1_MuS = -999.;
  Float_t err2_MuS = -999.;
  Float_t err3_MuS = -999.;
  Float_t err4_MuS = -999.;
    
  Int_t tsos_IP_valid;
  Int_t tsos_MuS_valid;
    
  HLTMuonCand(){};
  virtual ~HLTMuonCand(){};

  ClassDef(HLTMuonCand,1)

};


class L1MuonCand {
public:

  Float_t pt;           
  Float_t eta;          
  Float_t phi;          
  Int_t   charge;      
  Int_t   quality;      
  
  L1MuonCand(){};
  virtual ~L1MuonCand(){};

  ClassDef(L1MuonCand,1)

};



class HLTObjCand {
public:

  std::string filterTag; // name of filter passed by the object
  Float_t pt;            // pt of the object passing the filter [GeV]
  Float_t eta;           // eta of the object passing the filter
  Float_t phi;           // phi of the object passing the filter
  
  HLTObjCand(){};
  virtual ~HLTObjCand(){};

  ClassDef(HLTObjCand,1)

};





class HLTInfo {
public:
  std::vector<std::string>  triggers;  
  std::vector<HLTObjCand>   objects;   
 

  HLTInfo(){};
  virtual ~HLTInfo(){};
  bool match( const std::string & path ) {
	if (  std::find (  triggers.begin(), triggers.end(), path ) != triggers.end() )  return true;
//     if (! iname.compare("HLT_Mu20_v1") == 0) continue;
	return false;
  }

  bool find( const std::string & path ) {
	for ( std::vector<std::string>::const_iterator it = triggers.begin(); it != triggers.end(); ++it ) {
      if ( it-> compare(path) == 0) return true;
//       if ( it->find ( path ) != std::string::npos ) return true;
	}
	return false;
  }

  ClassDef(HLTInfo,1)

};


class MuonEvent {
public:

  Int_t   runNumber;             
  Int_t   luminosityBlockNumber; 
  Int_t   eventNumber;           

  Int_t   nVtx;                    
  Float_t primaryVertex[3];        
  Float_t cov_primaryVertex[3][3]; 

  Float_t trueNI;   

  
  Float_t bxId;
  Float_t instLumi; 

  std::vector <GenParticleCand> genParticles; 
  std::vector <MuonCand>        muons;         
  std::vector <HLTMuonCand>     tkmuons;      
  std::vector <HLTMuonCand>     hltNoIDmuons;     
  std::vector <HLTMuonCand>     hltmuons;     
  std::vector <HLTMuonCand>     hltOImuons;
  std::vector <HLTMuonCand>     hltIOmuons;
  std::vector <HLTMuonCand>     L2muons;      
  std::vector <L1MuonCand>      L1muons;      
  std::vector <HLTMuonCand>     L2muonsTSG;

//*******************INCLUDED*******************//
//  std::vector <SeedCand>        seeds;
  std::vector <HltTrackCand>    hltTrackOI;
  std::vector <HltTrackCand>    hltTrackIOL1;
  std::vector <HltTrackCand>    hltTrackIOL2;
//**********************************************//



  HLTInfo                       hlt;           
  HLTInfo                       hltTag;            

  MuonEvent(){};
  virtual ~MuonEvent(){};

  ClassDef(MuonEvent,1)
};


#endif

