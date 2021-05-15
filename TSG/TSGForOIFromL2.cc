/**
  \class    TSGForOIFromL2
  \brief    Create L3MuonTrajectorySeeds from L2 Muons updated at vertex in an outside-in manner
  \author   Benjamin Radburn-Smith, Santiago Folgueras, Bibhuprasad Mahakud, Jan Frederik Schulte, Dmitry Kondratyev (Purdue University, West Lafayette, USA)
 */

#include "RecoMuon/TrackerSeedGenerator/plugins/TSGForOIFromL2.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/Math/interface/deltaR.h"

#include <memory>

TSGForOIFromL2::TSGForOIFromL2(const edm::ParameterSet& iConfig)
    : src_(consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("src"))),
      maxSeeds_(iConfig.getParameter<uint32_t>("maxSeeds")),
      maxHitSeeds_(iConfig.getParameter<uint32_t>("maxHitSeeds")),
      maxHitlessSeeds_(iConfig.getParameter<uint32_t>("maxHitlessSeeds")),
      numOfLayersToTry_(iConfig.getParameter<int32_t>("layersToTry")),
      numOfHitsToTry_(iConfig.getParameter<int32_t>("hitsToTry")),
      numL2ValidHitsCutAllEta_(iConfig.getParameter<uint32_t>("numL2ValidHitsCutAllEta")),
      numL2ValidHitsCutAllEndcap_(iConfig.getParameter<uint32_t>("numL2ValidHitsCutAllEndcap")),
      fixedErrorRescalingForHits_(iConfig.getParameter<double>("fixedErrorRescaleFactorForHits")),
      fixedErrorRescalingForHitless_(iConfig.getParameter<double>("fixedErrorRescaleFactorForHitless")),
      adjustErrorsDynamicallyForHits_(iConfig.getParameter<bool>("adjustErrorsDynamicallyForHits")),
      adjustErrorsDynamicallyForHitless_(iConfig.getParameter<bool>("adjustErrorsDynamicallyForHitless")),
      estimatorName_(iConfig.getParameter<std::string>("estimator")),
      minEtaForTEC_(iConfig.getParameter<double>("minEtaForTEC")),
      maxEtaForTOB_(iConfig.getParameter<double>("maxEtaForTOB")),
      useHitLessSeeds_(iConfig.getParameter<bool>("UseHitLessSeeds")),
      updator_(new KFUpdator()),
      measurementTrackerTag_(
          consumes<MeasurementTrackerEvent>(iConfig.getParameter<edm::InputTag>("MeasurementTrackerEvent"))),
      pT1_(iConfig.getParameter<double>("pT1")),
      pT2_(iConfig.getParameter<double>("pT2")),
      pT3_(iConfig.getParameter<double>("pT3")),
      eta1_(iConfig.getParameter<double>("eta1")),
      eta2_(iConfig.getParameter<double>("eta2")),
      eta3_(iConfig.getParameter<double>("eta3")),
      eta4_(iConfig.getParameter<double>("eta4")),
      eta5_(iConfig.getParameter<double>("eta5")),
      eta6_(iConfig.getParameter<double>("eta6")),
      eta7_(iConfig.getParameter<double>("eta7")),
      SF1_(iConfig.getParameter<double>("SF1")),
      SF2_(iConfig.getParameter<double>("SF2")),
      SF3_(iConfig.getParameter<double>("SF3")),
      SF4_(iConfig.getParameter<double>("SF4")),
      SF5_(iConfig.getParameter<double>("SF5")),
      SF6_(iConfig.getParameter<double>("SF6")),
      tsosDiff1_(iConfig.getParameter<double>("tsosDiff1")),
      tsosDiff2_(iConfig.getParameter<double>("tsosDiff2")),
      propagatorName_(iConfig.getParameter<std::string>("propagatorName")),
      theCategory_(std::string("Muon|RecoMuon|TSGForOIFromL2")),
      useBothAsInRun2_(iConfig.getParameter<bool>("useBothAsInRun2")),
      dontCreateHitbasedInBarrelAsInRun2_(iConfig.getParameter<bool>("dontCreateHitbasedInBarrelAsInRun2")),
      maxHitlessSeedsIP_(iConfig.getParameter<uint32_t>("maxHitlessSeedsIP")),
      maxHitlessSeedsMuS_(iConfig.getParameter<uint32_t>("maxHitlessSeedsMuS")),
      maxHitDoubletSeeds_(iConfig.getParameter<uint32_t>("maxHitDoubletSeeds")),
      getStrategyFromDNN_(iConfig.getParameter<bool>("getStrategyFromDNN")),
      dnnModelPath_(iConfig.getParameter<std::string>("dnnModelPath"))
{
  produces<std::vector<TrajectorySeed> >();
}

TSGForOIFromL2::~TSGForOIFromL2() {}

//
// Produce seeds
//
void TSGForOIFromL2::produce(edm::StreamID sid, edm::Event& iEvent, const edm::EventSetup& iSetup) const {
  // Initialize variables
  unsigned int numSeedsMade = 0;
  unsigned int layerCount = 0;
  unsigned int hitlessSeedsMadeIP = 0;
  unsigned int hitlessSeedsMadeMuS = 0;
  unsigned int hitSeedsMade = 0;
  unsigned int hitDoubletSeedsMade = 0;

  // Surface used to make a TSOS at the PCA to the beamline
  Plane::PlanePointer dummyPlane = Plane::build(Plane::PositionType(), Plane::RotationType());

  // Read ESHandles
  edm::Handle<MeasurementTrackerEvent> measurementTrackerH;
  edm::ESHandle<Chi2MeasurementEstimatorBase> estimatorH;
  edm::ESHandle<MagneticField> magfieldH;
  edm::ESHandle<Propagator> propagatorAlongH;
  edm::ESHandle<Propagator> propagatorOppositeH;
  edm::ESHandle<TrackerGeometry> tmpTkGeometryH;
  edm::ESHandle<GlobalTrackingGeometry> geometryH;
  edm::ESHandle<NavigationSchool> navSchool;

  iSetup.get<IdealMagneticFieldRecord>().get(magfieldH);
  iSetup.get<TrackingComponentsRecord>().get(propagatorName_, propagatorOppositeH);
  iSetup.get<TrackingComponentsRecord>().get(propagatorName_, propagatorAlongH);
  iSetup.get<GlobalTrackingGeometryRecord>().get(geometryH);
  iSetup.get<TrackerDigiGeometryRecord>().get(tmpTkGeometryH);
  iSetup.get<TrackingComponentsRecord>().get(estimatorName_, estimatorH);
  iEvent.getByToken(measurementTrackerTag_, measurementTrackerH);
  iSetup.get<NavigationSchoolRecord>().get("SimpleNavigationSchool", navSchool);

  // Read L2 track collection
  edm::Handle<reco::TrackCollection> l2TrackCol;
  iEvent.getByToken(src_, l2TrackCol);

  // The product
  std::unique_ptr<std::vector<TrajectorySeed> > result(new std::vector<TrajectorySeed>());

  // Get vector of Detector layers
  std::vector<BarrelDetLayer const*> const& tob = measurementTrackerH->geometricSearchTracker()->tobLayers();
  std::vector<ForwardDetLayer const*> const& tecPositive =
      tmpTkGeometryH->isThere(GeomDetEnumerators::P2OTEC)
          ? measurementTrackerH->geometricSearchTracker()->posTidLayers()
          : measurementTrackerH->geometricSearchTracker()->posTecLayers();
  std::vector<ForwardDetLayer const*> const& tecNegative =
      tmpTkGeometryH->isThere(GeomDetEnumerators::P2OTEC)
          ? measurementTrackerH->geometricSearchTracker()->negTidLayers()
          : measurementTrackerH->geometricSearchTracker()->negTecLayers();

  // Get suitable propagators
  std::unique_ptr<Propagator> propagatorAlong = SetPropagationDirection(*propagatorAlongH, alongMomentum);
  std::unique_ptr<Propagator> propagatorOpposite = SetPropagationDirection(*propagatorOppositeH, oppositeToMomentum);

  // Stepping Helix Propagator for propogation from muon system to tracker
  edm::ESHandle<Propagator> SHPOpposite;
  iSetup.get<TrackingComponentsRecord>().get("hltESPSteppingHelixPropagatorOpposite", SHPOpposite);

  // Loop over the L2's and make seeds for all of them
  LogTrace(theCategory_) << "TSGForOIFromL2::produce: Number of L2's: " << l2TrackCol->size();
  for (unsigned int l2TrackColIndex(0); l2TrackColIndex != l2TrackCol->size(); ++l2TrackColIndex) {
    const reco::TrackRef l2(l2TrackCol, l2TrackColIndex);

    // Container of Seeds
    std::vector<TrajectorySeed> out;
    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::produce: L2 muon pT, eta, phi --> " << l2->pt() << " , " << l2->eta()
                               << " , " << l2->phi() << std::endl;

    FreeTrajectoryState fts = trajectoryStateTransform::initialFreeState(*l2, magfieldH.product());

    dummyPlane->move(fts.position() - dummyPlane->position());
    TrajectoryStateOnSurface tsosAtIP = TrajectoryStateOnSurface(fts, *dummyPlane);
    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::produce: Created TSOSatIP: " << tsosAtIP << std::endl;

    // Get the TSOS on the innermost layer of the L2
    TrajectoryStateOnSurface tsosAtMuonSystem =
        trajectoryStateTransform::innerStateOnSurface(*l2, *geometryH, magfieldH.product());
    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::produce: Created TSOSatMuonSystem: " << tsosAtMuonSystem
                               << std::endl;

    LogTrace("TSGForOIFromL2")
        << "TSGForOIFromL2::produce: Check the error of the L2 parameter and use hit seeds if big errors" << std::endl;

    StateOnTrackerBound fromInside(propagatorAlong.get());
    TrajectoryStateOnSurface outerTkStateInside = fromInside(fts);

    StateOnTrackerBound fromOutside(&*SHPOpposite);
    TrajectoryStateOnSurface outerTkStateOutside = fromOutside(tsosAtMuonSystem);

    // Check if the two positions (using updated and not-updated TSOS) agree withing certain extent.
    // If both TSOSs agree, use only the one at vertex, as it uses more information. If they do not agree, search for seeds based on both.
    double L2muonEta = l2->eta();
    double absL2muonEta = std::abs(L2muonEta);
    bool useBoth = false;
    if (useBothAsInRun2_ && outerTkStateInside.isValid() && outerTkStateOutside.isValid()) {
      if (l2->numberOfValidHits() < numL2ValidHitsCutAllEta_)
        useBoth = true;
      if (l2->numberOfValidHits() < numL2ValidHitsCutAllEndcap_ && absL2muonEta > eta7_)
        useBoth = true;
      if (absL2muonEta > eta1_ && absL2muonEta < eta1_)
        useBoth = true;
    }
    
    //if (getStrategyFromDNN_){
    //    tensorflow::setLogging("2");
    //    tensorflow::GraphDef* graphDef = tensorflow::loadGraphDef(dnnModelPath_);
    //    tensorflow::Session* tf_session = tensorflow::createSession(graphDef);
    //    int dnn_decision = evaluateDnn(l2, tsosAtIP, outerTkStateOutside, tf_session);
    //    std::cout << "DNN decision: " << dnn_decision << std::endl;
    //    // TODO: select strategy based on the returned number
    //}
    
    numSeedsMade = 0;
    hitlessSeedsMadeIP = 0;
    hitlessSeedsMadeMuS = 0;
    hitSeedsMade = 0;
    hitDoubletSeedsMade = 0;

    // calculate scale factors
    double errorSFHits = (adjustErrorsDynamicallyForHits_ ? calculateSFFromL2(l2) : fixedErrorRescalingForHits_);
    double errorSFHitless =
        (adjustErrorsDynamicallyForHitless_ ? calculateSFFromL2(l2) : fixedErrorRescalingForHitless_);

    // BARREL
    if (absL2muonEta < maxEtaForTOB_) {
      layerCount = 0;
      for (auto it = tob.rbegin(); it != tob.rend(); ++it) {
        LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::produce: looping in TOB layer " << layerCount << std::endl;
        if (useHitLessSeeds_ && hitlessSeedsMadeIP < maxHitlessSeedsIP_ && numSeedsMade < maxSeeds_)
          makeSeedsWithoutHits(**it,
                               tsosAtIP,
                               *(propagatorAlong.get()),
                               estimatorH,
                               errorSFHitless,
                               hitlessSeedsMadeIP,
                               numSeedsMade,
                               out);
        if (outerTkStateInside.isValid() && outerTkStateOutside.isValid() &&
            useHitLessSeeds_ && hitlessSeedsMadeMuS < maxHitlessSeedsMuS_ && numSeedsMade < maxSeeds_)
            makeSeedsWithoutHits(**it,
                                 outerTkStateOutside,
                                 *(propagatorOpposite.get()),
                                 estimatorH,
                                 errorSFHitless,
                                 hitlessSeedsMadeMuS,
                                 numSeedsMade,
                                 out);
        // Do not create hitbased seeds in barrel region
        if (hitSeedsMade < maxHitSeeds_ && numSeedsMade < maxSeeds_){
            // Run2 approach, preserved for backward compatibility
            if (!(dontCreateHitbasedInBarrelAsInRun2_ && (absL2muonEta <= 1.0)))
              makeSeedsFromHits(**it,
                            tsosAtIP,
                            *(propagatorAlong.get()),
                            estimatorH,
                            measurementTrackerH,
                            errorSFHits,
                            hitSeedsMade,
                            numSeedsMade,
                            layerCount,
                            out);
        }

        if (hitDoubletSeedsMade < maxHitDoubletSeeds_ && numSeedsMade < maxSeeds_){
            makeSeedsFromHitDoublets(**it,
                            tsosAtIP,
                            *(propagatorAlong.get()),
                            estimatorH,
                            measurementTrackerH,
                            navSchool,
                            errorSFHits,
                            hitDoubletSeedsMade,
                            numSeedsMade,
                            layerCount,
                            out);
        }
        // Run2 approach, preserved for backward compatibility
        if (useBoth) {
          if (useHitLessSeeds_ && hitlessSeedsMadeMuS < maxHitlessSeedsIP_ && numSeedsMade < maxSeeds_)
            makeSeedsWithoutHits(**it,
                                 outerTkStateOutside,
                                 *(propagatorOpposite.get()),
                                 estimatorH,
                                 errorSFHitless,
                                 hitlessSeedsMadeMuS,
                                 numSeedsMade,
                                 out);
        }
      }
      LogTrace("TSGForOIFromL2") << "TSGForOIFromL2:::produce: NumSeedsMade = " << numSeedsMade
                                 << " , layerCount = " << layerCount << std::endl;
    }

    // Reset number of seeds if in overlap region
    if (absL2muonEta > minEtaForTEC_ && absL2muonEta < maxEtaForTOB_) {
      numSeedsMade = 0;
      hitlessSeedsMadeIP = 0;
      hitlessSeedsMadeMuS = 0;
      hitSeedsMade = 0;
      hitDoubletSeedsMade = 0;
    }

    // ENDCAP+
    if (L2muonEta > minEtaForTEC_) {
      layerCount = 0;
      for (auto it = tecPositive.rbegin(); it != tecPositive.rend(); ++it) {
        LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::produce: looping in TEC+ layer " << layerCount << std::endl;
        if (useHitLessSeeds_ && hitlessSeedsMadeIP < maxHitlessSeedsIP_ && numSeedsMade < maxSeeds_)
          makeSeedsWithoutHits(**it,
                               tsosAtIP,
                               *(propagatorAlong.get()),
                               estimatorH,
                               errorSFHitless,
                               hitlessSeedsMadeIP,
                               numSeedsMade,
                               out);
        if (outerTkStateInside.isValid() && outerTkStateOutside.isValid() &&
            useHitLessSeeds_ && hitlessSeedsMadeMuS < maxHitlessSeedsMuS_ && numSeedsMade < maxSeeds_)
            makeSeedsWithoutHits(**it,
                                 outerTkStateOutside,
                                 *(propagatorOpposite.get()),
                                 estimatorH,
                                 errorSFHitless,
                                 hitlessSeedsMadeMuS,
                                 numSeedsMade,
                                 out);
        if (hitSeedsMade < maxHitSeeds_ && numSeedsMade < maxSeeds_){
            // Run2 approach, preserved for backward compatibility
            if (!(dontCreateHitbasedInBarrelAsInRun2_ && (absL2muonEta <= 1.0)))
              makeSeedsFromHits(**it,
                            tsosAtIP,
                            *(propagatorAlong.get()),
                            estimatorH,
                            measurementTrackerH,
                            errorSFHits,
                            hitSeedsMade,
                            numSeedsMade,
                            layerCount,
                            out);
        }
        if (hitDoubletSeedsMade < maxHitDoubletSeeds_ && numSeedsMade < maxSeeds_){
            makeSeedsFromHitDoublets(**it,
                            tsosAtIP,
                            *(propagatorAlong.get()),
                            estimatorH,
                            measurementTrackerH,
                            navSchool,
                            errorSFHits,
                            hitDoubletSeedsMade,
                            numSeedsMade,
                            layerCount,
                            out);
        }
         // Run2 approach, preserved for backward compatibility
        if (useBoth) {
          if (useHitLessSeeds_ && hitlessSeedsMadeMuS < maxHitlessSeedsIP_ && numSeedsMade < maxSeeds_)
            makeSeedsWithoutHits(**it,
                                 outerTkStateOutside,
                                 *(propagatorOpposite.get()),
                                 estimatorH,
                                 errorSFHitless,
                                 hitlessSeedsMadeMuS,
                                 numSeedsMade,
                                 out);
        }
      }
      LogTrace("TSGForOIFromL2") << "TSGForOIFromL2:::produce: NumSeedsMade = " << numSeedsMade
                                 << " , layerCount = " << layerCount << std::endl;
    }

    // ENDCAP-
    if (L2muonEta < -minEtaForTEC_) {
      layerCount = 0;
      for (auto it = tecNegative.rbegin(); it != tecNegative.rend(); ++it) {
        LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::produce: looping in TEC- layer " << layerCount << std::endl;
        if (useHitLessSeeds_ && hitlessSeedsMadeIP < maxHitlessSeedsIP_ && numSeedsMade < maxSeeds_)
          makeSeedsWithoutHits(**it,
                               tsosAtIP,
                               *(propagatorAlong.get()),
                               estimatorH,
                               errorSFHitless,
                               hitlessSeedsMadeIP,
                               numSeedsMade,
                               out);
        if (outerTkStateInside.isValid() && outerTkStateOutside.isValid() &&
            useHitLessSeeds_ && hitlessSeedsMadeMuS < maxHitlessSeedsMuS_ && numSeedsMade < maxSeeds_)
            makeSeedsWithoutHits(**it,
                                 outerTkStateOutside,
                                 *(propagatorOpposite.get()),
                                 estimatorH,
                                 errorSFHitless,
                                 hitlessSeedsMadeMuS,
                                 numSeedsMade,
                                 out);

        if (hitSeedsMade < maxHitSeeds_ && numSeedsMade < maxSeeds_){
            // Run2 approach, preserved for backward compatibility
            if (!(dontCreateHitbasedInBarrelAsInRun2_ && (absL2muonEta <= 1.0)))
              makeSeedsFromHits(**it,
                            tsosAtIP,
                            *(propagatorAlong.get()),
                            estimatorH,
                            measurementTrackerH,
                            errorSFHits,
                            hitSeedsMade,
                            numSeedsMade,
                            layerCount,
                            out);
        }
        if (hitDoubletSeedsMade < maxHitDoubletSeeds_ && numSeedsMade < maxSeeds_){
            makeSeedsFromHitDoublets(**it,
                            tsosAtIP,
                            *(propagatorAlong.get()),
                            estimatorH,
                            measurementTrackerH,
                            navSchool,
                            errorSFHits,
                            hitDoubletSeedsMade,
                            numSeedsMade,
                            layerCount,
                            out);
        }
        // Run2 approach, preserved for backward compatibility
        if (useBoth) {
          if (useHitLessSeeds_ && hitlessSeedsMadeMuS < maxHitlessSeedsIP_ && numSeedsMade < maxSeeds_)
            makeSeedsWithoutHits(**it,
                                 outerTkStateOutside,
                                 *(propagatorOpposite.get()),
                                 estimatorH,
                                 errorSFHitless,
                                 hitlessSeedsMadeMuS,
                                 numSeedsMade,
                                 out);
        }
      }
      LogTrace("TSGForOIFromL2") << "TSGForOIFromL2:::produce: NumSeedsMade = " << numSeedsMade
                                 << " , layerCount = " << layerCount << std::endl;
    }

    for (std::vector<TrajectorySeed>::iterator it = out.begin(); it != out.end(); ++it) {
      result->push_back(*it);
    }

  }  // L2Collection

  edm::LogInfo(theCategory_) << "TSGForOIFromL2::produce: number of seeds made: " << result->size();

  iEvent.put(std::move(result));
}

//
// Create seeds without hits on a given layer (TOB or TEC)
//
void TSGForOIFromL2::makeSeedsWithoutHits(const GeometricSearchDet& layer,
                                          const TrajectoryStateOnSurface& tsos,
                                          const Propagator& propagatorAlong,
                                          edm::ESHandle<Chi2MeasurementEstimatorBase>& estimator,
                                          double errorSF,
                                          unsigned int& hitlessSeedsMade,
                                          unsigned int& numSeedsMade,
                                          std::vector<TrajectorySeed>& out) const {
  // create hitless seeds
  LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsWithoutHits: Start hitless" << std::endl;
  std::vector<GeometricSearchDet::DetWithState> dets;
  layer.compatibleDetsV(tsos, propagatorAlong, *estimator, dets);
  if (!dets.empty()) {
    auto const& detOnLayer = dets.front().first;
    auto const& tsosOnLayer = dets.front().second;
    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsWithoutHits: tsosOnLayer " << tsosOnLayer << std::endl;
    if (!tsosOnLayer.isValid()) {
      edm::LogInfo(theCategory_) << "ERROR!: Hitless TSOS is not valid!";
    } else {
      dets.front().second.rescaleError(errorSF);
      PTrajectoryStateOnDet const& ptsod =
          trajectoryStateTransform::persistentState(tsosOnLayer, detOnLayer->geographicalId().rawId());
      TrajectorySeed::recHitContainer rHC;
      out.push_back(TrajectorySeed(ptsod, rHC, oppositeToMomentum));
      LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsWithoutHits: TSOS (Hitless) done " << std::endl;
      hitlessSeedsMade++;
      numSeedsMade++;
    }
  }
}

//
// Find hits on a given layer (TOB or TEC) and create seeds from updated TSOS with hit
//
void TSGForOIFromL2::makeSeedsFromHits(const GeometricSearchDet& layer,
                                       const TrajectoryStateOnSurface& tsos,
                                       const Propagator& propagatorAlong,
                                       edm::ESHandle<Chi2MeasurementEstimatorBase>& estimator,
                                       edm::Handle<MeasurementTrackerEvent>& measurementTracker,
                                       double errorSF,
                                       unsigned int& hitSeedsMade,
                                       unsigned int& numSeedsMade,
                                       unsigned int& layerCount,
                                       std::vector<TrajectorySeed>& out) const {
  if (layerCount > numOfLayersToTry_)
    return;

  // Error Rescaling
  TrajectoryStateOnSurface onLayer(tsos);
  onLayer.rescaleError(errorSF);

  std::vector<GeometricSearchDet::DetWithState> dets;
  layer.compatibleDetsV(onLayer, propagatorAlong, *estimator, dets);

  // Find Measurements on each DetWithState
  LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHits: Find measurements on each detWithState  "
                             << dets.size() << std::endl;
  std::vector<TrajectoryMeasurement> meas;
  for (std::vector<GeometricSearchDet::DetWithState>::iterator it = dets.begin(); it != dets.end(); ++it) {
    MeasurementDetWithData det = measurementTracker->idToDet(it->first->geographicalId());
    if (det.isNull())
      continue;
    if (!it->second.isValid())
      continue;  // Skip if TSOS is not valid

    std::vector<TrajectoryMeasurement> mymeas =
        det.fastMeasurements(it->second, onLayer, propagatorAlong, *estimator);  // Second TSOS is not used
    for (std::vector<TrajectoryMeasurement>::const_iterator it2 = mymeas.begin(), ed2 = mymeas.end(); it2 != ed2;
         ++it2) {
      if (it2->recHit()->isValid())
        meas.push_back(*it2);  // Only save those which are valid
    }
  }

  // Update TSOS using TMs after sorting, then create Trajectory Seed and put into vector
  LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHits: Update TSOS using TMs after sorting, then create "
                                "Trajectory Seed, number of TM = "
                             << meas.size() << std::endl;
  std::sort(meas.begin(), meas.end(), TrajMeasLessEstim());

  unsigned int found = 0;
  for (std::vector<TrajectoryMeasurement>::const_iterator it = meas.begin(); it != meas.end(); ++it) {
    TrajectoryStateOnSurface updatedTSOS = updator_->update(it->forwardPredictedState(), *it->recHit());
    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHits: TSOS for TM " << found << std::endl;
    if (not updatedTSOS.isValid())
      continue;

    edm::OwnVector<TrackingRecHit> seedHits;
    seedHits.push_back(*it->recHit()->hit());
    PTrajectoryStateOnDet const& pstate =
        trajectoryStateTransform::persistentState(updatedTSOS, it->recHit()->geographicalId().rawId());
    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHits: Number of seedHits: " << seedHits.size()
                               << std::endl;
    TrajectorySeed seed(pstate, std::move(seedHits), oppositeToMomentum);
    out.push_back(seed);
    found++;
    numSeedsMade++;
    hitSeedsMade++;
    if (found == numOfHitsToTry_)
      break;
    if (hitSeedsMade > maxHitSeeds_)
      return;
  }

  if (found)
    layerCount++;
}



void TSGForOIFromL2::makeSeedsFromHitDoublets(const GeometricSearchDet& layer,
                                       const TrajectoryStateOnSurface& tsos,
                                       const Propagator& propagatorAlong,
                                       edm::ESHandle<Chi2MeasurementEstimatorBase>& estimator,
                                       edm::Handle<MeasurementTrackerEvent>& measurementTracker,
                                       edm::ESHandle<NavigationSchool> navSchool,
                                       double errorSF,
                                       unsigned int& hitDoubletSeedsMade,
                                       unsigned int& numSeedsMade,
                                       unsigned int& layerCount,
                                       std::vector<TrajectorySeed>& out) const {

  // This method is similar to makeSeedsFromHits, but the seed is created
  // only when in addition to a hit on a given layer, there are more compatible hits
  // on next layers (going from outside inwards), compatible with updated TSOS.
  // If that's the case, multiple compatible hits are used to create a single seed.

  // Configured to only check the immideately adjacent layer and add one more hit
  int max_addtnl_layers = 1; // max number of additional layers to scan
  int max_meas = 1; // number of measurements to consider on each additional layer

  // // // First, regular procedure to find a compatible hit - like in makeSeedsFromHits // // //
  
  // Error Rescaling
  TrajectoryStateOnSurface onLayer(tsos);
  onLayer.rescaleError(errorSF);

  // Find dets compatible with original TSOS
  std::vector< GeometricSearchDet::DetWithState > dets;
  layer.compatibleDetsV(onLayer, propagatorAlong, *estimator, dets);

  LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHitDoublets: Find measurements on each detWithState  " << dets.size() << std::endl;
  std::vector<TrajectoryMeasurement> meas;
    
  // Loop over dets
  for (std::vector<GeometricSearchDet::DetWithState>::iterator idet=dets.begin(); idet!=dets.end(); ++idet) {
    MeasurementDetWithData det = measurementTracker->idToDet(idet->first->geographicalId());

    if (det.isNull()) continue;    // skip if det does not exist
    if (!idet->second.isValid()) continue;    // skip if TSOS is invalid

    // Find measurements on this det
    std::vector <TrajectoryMeasurement> mymeas = det.fastMeasurements(idet->second, onLayer, propagatorAlong, *estimator);
    
    // Save valid measurements 
    for (std::vector<TrajectoryMeasurement>::const_iterator imea = mymeas.begin(), ed2 = mymeas.end(); imea != ed2; ++imea) {
      if (imea->recHit()->isValid()) meas.push_back(*imea);
    } // end loop over meas
  } // end loop over dets

  LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHitDoublets: Update TSOS using TMs after sorting, then create Trajectory Seed, number of TM = " << meas.size() << std::endl;
  
  // sort valid measurements found on the first layer
  std::sort(meas.begin(), meas.end(), TrajMeasLessEstim());

  unsigned int found = 0;
  int hit_num = 0;
  
  // Loop over all valid measurements compatible with original TSOS
  for (std::vector<TrajectoryMeasurement>::const_iterator mea=meas.begin(); mea!=meas.end(); ++mea) {
    hit_num++;
    
    // Update TSOS with measurement on first considered layer
    TrajectoryStateOnSurface updatedTSOS = updator_->update(mea->forwardPredictedState(), *mea->recHit());

    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHitDoublets: TSOS for TM " << found << std::endl;
    if (not updatedTSOS.isValid()) continue;    // Skip if updated TSOS is invalid

    edm::OwnVector<TrackingRecHit> seedHits;
      
    // Save hit on first layer
    seedHits.push_back(*mea->recHit()->hit());
    const DetLayer* detLayer = dynamic_cast<const DetLayer*>(&layer);


    // // // Now for this measurement we will loop over additional layers and try to update the TSOS again // // //

    // find layers compatible with updated TSOS
    auto const& compLayers = (*navSchool).nextLayers(*detLayer, *updatedTSOS.freeState(), alongMomentum);

    int addtnl_layers_scanned=0;
    int found_compatible_on_next_layer = 0;
    int det_id = 0;
      
    // Copy updated TSOS - we will update it again with a measurement from the next layer, if we find it
    TrajectoryStateOnSurface updatedTSOS_next(updatedTSOS);

    // loop over layers compatible with updated TSOS
    for (auto compLayer : compLayers) {
      int nmeas=0;

      if (addtnl_layers_scanned>=max_addtnl_layers) break;    // break if we already looped over enough layers
      if (found_compatible_on_next_layer>0) break;    // break if we already found additional hit

      // find dets compatible with updated TSOS
      std::vector< GeometricSearchDet::DetWithState > dets_next;
      TrajectoryStateOnSurface onLayer_next(updatedTSOS);
      onLayer_next.rescaleError(errorSF);//errorSF
      compLayer->compatibleDetsV(onLayer_next, propagatorAlong, *estimator, dets_next);

      //if (!detWithState.size()) continue;
      std::vector<TrajectoryMeasurement> meas_next;
      
      // find measurements on dets_next and save the valid ones
      for (std::vector<GeometricSearchDet::DetWithState>::iterator idet_next=dets_next.begin(); idet_next!=dets_next.end(); ++idet_next) {
        MeasurementDetWithData det = measurementTracker->idToDet(idet_next->first->geographicalId());

        if (det.isNull()) continue;    // skip if det does not exist
        if (!idet_next->second.isValid()) continue;    // skip if TSOS is invalid

        // Find measurements on this det
        std::vector <TrajectoryMeasurement>mymeas_next=det.fastMeasurements(idet_next->second, onLayer_next, propagatorAlong, *estimator);

        for (std::vector<TrajectoryMeasurement>::const_iterator imea_next=mymeas_next.begin(), ed2=mymeas_next.end(); imea_next != ed2; ++imea_next) {
            
          // save valid measurements
          if (imea_next->recHit()->isValid()) meas_next.push_back(*imea_next);

        }    // end loop over mymeas_next
      }    // end loop over dets_next

      // sort valid measurements found on this layer
      std::sort(meas_next.begin(), meas_next.end(), TrajMeasLessEstim());
        
      // loop over valid measurements compatible with updated TSOS (TSOS updated with a hit on the first layer)
      for (std::vector<TrajectoryMeasurement>::const_iterator mea_next=meas_next.begin(); mea_next!=meas_next.end(); ++mea_next) {

        if (nmeas>=max_meas) break;    // skip if we already found enough hits
        
        // try to update TSOS again, with an additional hit
        updatedTSOS_next = updator_->update(mea_next->forwardPredictedState(), *mea_next->recHit());
        
        if (not updatedTSOS_next.isValid()) continue;    // skip if TSOS updated with additional hit is not valid

        // If there was a compatible hit on this layer, we end up here.
        // An additional compatible hit is saved.
        seedHits.push_back(*mea_next->recHit()->hit());
        det_id = mea_next->recHit()->geographicalId().rawId();
        nmeas++;
        found_compatible_on_next_layer++;

      } // end loop over meas_next

    addtnl_layers_scanned++;    

    } // end loop over compLayers (additional layers scanned after the original layer)

    if (found_compatible_on_next_layer==0) continue;
    // only consider the hit if there was a compatible hit on one of the additional scanned layers

    // Create a seed from two saved hits
    PTrajectoryStateOnDet const& pstate = trajectoryStateTransform::persistentState(updatedTSOS_next, det_id);
    TrajectorySeed seed(pstate, std::move(seedHits), oppositeToMomentum);

    LogTrace("TSGForOIFromL2") << "TSGForOIFromL2::makeSeedsFromHitDoublets: Number of seedHits: " << seedHits.size() << std::endl;
    out.push_back(seed);

    found++;
    numSeedsMade++;
    hitDoubletSeedsMade++;

    if (found == numOfHitsToTry_) break;    // break if enough measurements scanned
    if (hitDoubletSeedsMade > maxHitDoubletSeeds_) return;    // abort if enough seeds created

  } // end loop over measurements compatible with original TSOS

  if (found)
      layerCount++;

}


//
// Calculate the dynamic error SF by analysing the L2
//
double TSGForOIFromL2::calculateSFFromL2(const reco::TrackRef track) const {
  double theSF = 1.0;
  // L2 direction vs pT blowup - as was previously done:
  // Split into 4 pT ranges: <pT1_, pT1_<pT2_, pT2_<pT3_, <pT4_: 13,30,70
  // Split into different eta ranges depending in pT
  double abseta = std::abs(track->eta());
  if (track->pt() <= pT1_)
    theSF = SF1_;
  else if (track->pt() > pT1_ && track->pt() <= pT2_) {
    if (abseta <= eta3_)
      theSF = SF3_;
    else if (abseta > eta3_ && abseta <= eta6_)
      theSF = SF2_;
    else if (abseta > eta6_)
      theSF = SF3_;
  } else if (track->pt() > pT2_ && track->pt() <= pT3_) {
    if (abseta <= eta1_)
      theSF = SF6_;
    else if (abseta > eta1_ && abseta <= eta2_)
      theSF = SF4_;
    else if (abseta > eta2_ && abseta <= eta3_)
      theSF = SF6_;
    else if (abseta > eta3_ && abseta <= eta4_)
      theSF = SF1_;
    else if (abseta > eta4_ && abseta <= eta5_)
      theSF = SF1_;
    else if (abseta > eta5_)
      theSF = SF5_;
  } else if (track->pt() > pT3_) {
    if (abseta <= eta3_)
      theSF = SF5_;
    else if (abseta > eta3_ && abseta <= eta4_)
      theSF = SF4_;
    else if (abseta > eta4_ && abseta <= eta5_)
      theSF = SF4_;
    else if (abseta > eta5_)
      theSF = SF5_;
  }

  LogTrace(theCategory_) << "TSGForOIFromL2::calculateSFFromL2: SF has been calculated as: " << theSF;

  return theSF;
}

//
// calculate Chi^2 of two trajectory states
//
double TSGForOIFromL2::match_Chi2(const TrajectoryStateOnSurface& tsos1, const TrajectoryStateOnSurface& tsos2) const {
  if (!tsos1.isValid() || !tsos2.isValid())
    return -1.;

  AlgebraicVector5 v(tsos1.localParameters().vector() - tsos2.localParameters().vector());
  AlgebraicSymMatrix55 m(tsos1.localError().matrix() + tsos2.localError().matrix());

  bool ierr = !m.Invert();

  if (ierr) {
    edm::LogInfo("TSGForOIFromL2") << "Error inverting covariance matrix";
    return -1;
  }

  double est = ROOT::Math::Similarity(v, m);

  return est;
}

/*
int TSGForOIFromL2::evaluateDnn(
    reco::TrackRef l2,
    const TrajectoryStateOnSurface& tsos_IP,
    const TrajectoryStateOnSurface& tsos_MuS,
    tensorflow::Session* session
) const {    
    int n_features = 26;
    int n_outputs = 6;
    double features[26];
    
    features[0] = l2->pt();// pt
    features[1] = l2->eta();// eta
    features[2] = l2->phi();// phi
    features[3] = l2->found();// validHits
    if (tsos_IP.isValid()) {
        features[4] = tsos_IP.globalPosition().eta();// tsos_IP_eta
        features[5] = tsos_IP.globalPosition().phi();// tsos_IP_phi
        features[6] = tsos_IP.globalMomentum().perp();// tsos_IP_pt
        features[7] = tsos_IP.globalMomentum().eta();// tsos_IP_pt_eta
        features[8] = tsos_IP.globalMomentum().phi();// tsos_IP_pt_phi
        AlgebraicSymMatrix55 matrix_IP = tsos_IP.curvilinearError().matrix();
        features[9]  = sqrt(matrix_IP[0][0]);// err0_IP
        features[10] = sqrt(matrix_IP[1][1]);// err1_IP
        features[11] = sqrt(matrix_IP[2][2]);// err2_IP
        features[12] = sqrt(matrix_IP[3][3]);// err3_IP
        features[13] = sqrt(matrix_IP[4][4]);// err4_IP
        features[24] = 1.0;// tsos_IP_valid
    } else {
        features[4] = -999.;// tsos_IP_eta
        features[5] = -999.;// tsos_IP_phi
        features[6] = -999.;// tsos_IP_pt
        features[7] = -999.;// tsos_IP_pt_eta
        features[8] = -999.;// tsos_IP_pt_phi
        features[9]  = -999.;// err0_IP
        features[10] = -999.;// err1_IP
        features[11] = -999.;// err2_IP
        features[12] = -999.;// err3_IP
        features[13] = -999.;// err4_IP
        features[24] = 0.0;// tsos_IP_valid
    }
    if (tsos_MuS.isValid()) {
        features[14] = tsos_MuS.globalPosition().eta();// tsos_MuS_eta
        features[15] = tsos_MuS.globalPosition().phi();// tsos_MuS_phi
        features[16] = tsos_MuS.globalMomentum().perp();// tsos_MuS_pt
        features[17] = tsos_MuS.globalMomentum().eta();// tsos_MuS_pt_eta
        features[18] = tsos_MuS.globalMomentum().phi();// tsos_MuS_pt_phi
        AlgebraicSymMatrix55 matrix_MuS = tsos_MuS.curvilinearError().matrix();
        features[19] = sqrt(matrix_MuS[0][0]);// err0_MuS
        features[20] = sqrt(matrix_MuS[1][1]);// err1_MuS
        features[21] = sqrt(matrix_MuS[2][2]);// err2_MuS
        features[22] = sqrt(matrix_MuS[3][3]);// err3_MuS
        features[23] = sqrt(matrix_MuS[4][4]);// err4_MuS
        features[25] = 1.0;// tsos_MuS_valid
    } else {
        features[14] = -999.;// tsos_MuS_eta
        features[15] = -999.;// tsos_MuS_phi
        features[16] = -999.;// tsos_MuS_pt
        features[17] = -999.;// tsos_MuS_pt_eta
        features[18] = -999.;// tsos_MuS_pt_phi
        features[19] = -999.;// err0_MuS
        features[20] = -999.;// err1_MuS
        features[21] = -999.;// err2_MuS
        features[22] = -999.;// err3_MuS
        features[23] = -999.;// err4_MuS
        features[25] = 0.0;// tsos_MuS_valid
    }
    
    tensorflow::Tensor input(tensorflow::DT_FLOAT, { 1, n_features });
    for (int i = 0; i < n_features; i++) {
        input.matrix<float>()(0, i) = float(features[i]);
        // std::cout << "Input " << i << ": " << features[i] << std::endl;
    }
    
    std::vector<tensorflow::Tensor> outputs;
    tensorflow::run(session, { { "dnn_0_input", input } }, { "model/dnn_0_output/Sigmoid" }, &outputs);

    int imax = -1;
    float max = 0;
    for (int i = 0; i < n_outputs; i++) {
        float ith_output = outputs[0].matrix<float>()(0, i);
        //std::cout << "Output "<< i << ": " << ith_output << std::endl;
        if (ith_output > max){
            max = ith_output;
            imax = i;
        }
    }
    return imax;
}
*/

//
//
//
void TSGForOIFromL2::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("src", edm::InputTag("hltL2Muons", "UpdatedAtVtx"));
  desc.add<int>("layersToTry", 2);
  desc.add<double>("fixedErrorRescaleFactorForHitless", 2.0);
  desc.add<int>("hitsToTry", 1);
  desc.add<bool>("adjustErrorsDynamicallyForHits", false);
  desc.add<bool>("adjustErrorsDynamicallyForHitless", true);
  desc.add<edm::InputTag>("MeasurementTrackerEvent", edm::InputTag("hltSiStripClusters"));
  desc.add<bool>("UseHitLessSeeds", true);
  desc.add<std::string>("estimator", "hltESPChi2MeasurementEstimator100");
  desc.add<double>("maxEtaForTOB", 1.8);
  desc.add<double>("minEtaForTEC", 0.7);
  desc.addUntracked<bool>("debug", false);
  desc.add<double>("fixedErrorRescaleFactorForHits", 1.0);
  desc.add<unsigned int>("maxSeeds", 20);
  desc.add<unsigned int>("maxHitlessSeeds", 5);  
  desc.add<unsigned int>("maxHitSeeds", 1);
  desc.add<unsigned int>("numL2ValidHitsCutAllEta", 20);
  desc.add<unsigned int>("numL2ValidHitsCutAllEndcap", 30);
  desc.add<double>("pT1", 13.0);
  desc.add<double>("pT2", 30.0);
  desc.add<double>("pT3", 70.0);
  desc.add<double>("eta1", 0.2);
  desc.add<double>("eta2", 0.3);
  desc.add<double>("eta3", 1.0);
  desc.add<double>("eta4", 1.2);
  desc.add<double>("eta5", 1.6);
  desc.add<double>("eta6", 1.4);
  desc.add<double>("eta7", 2.1);
  desc.add<double>("SF1", 3.0);
  desc.add<double>("SF2", 4.0);
  desc.add<double>("SF3", 5.0);
  desc.add<double>("SF4", 7.0);
  desc.add<double>("SF5", 10.0);
  desc.add<double>("SF6", 2.0);
  desc.add<double>("tsosDiff1", 0.2);
  desc.add<double>("tsosDiff2", 0.02);
  desc.add<std::string>("propagatorName", "PropagatorWithMaterialParabolicMf");
  desc.add<bool>("useBothAsInRun2", true);
  desc.add<bool>("dontCreateHitbasedInBarrelAsInRun2", true);
  desc.add<unsigned int>("maxHitlessSeedsIP", 5);
  desc.add<unsigned int>("maxHitlessSeedsMuS", 0);
  desc.add<unsigned int>("maxHitDoubletSeeds", 0);
  desc.add<bool>("getStrategyFromDNN", false);
  desc.add<std::string>("dnnModelPath", "");
  descriptions.add("TSGForOIFromL2", desc);
}

DEFINE_FWK_MODULE(TSGForOIFromL2);
