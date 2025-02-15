//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : F.Gaede, DESY
//
//==========================================================================

#ifndef DD4HEP_DDG4_GEANT4OUTPUT2EDM4hep_H
#define DD4HEP_DDG4_GEANT4OUTPUT2EDM4hep_H

//  Framework include files
#include "DD4hep/Detector.h"
#include "DD4hep/VolumeManager.h"
#include "DDG4/Geant4HitCollection.h"
#include "DDG4/Geant4OutputAction.h"
#include "DDG4/Geant4SensDetAction.h"
#include "DDG4/Geant4DataConversion.h"
#include "DDG4/EventParameters.h"

// Geant4 headers
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4Version.hh"

// use the Geant4 units in namespace CLHEP
#include "CLHEP/Units/SystemOfUnits.h"

// edm4hep include files
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/CaloHitContributionCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"

#include <typeinfo>
#include <iostream>
#include <ctime>
#include <unordered_map>

/// Namespace for the AIDA detector description toolkit
namespace dd4hep {

  class ComponentCast;

  /// Namespace for the Geant4 based simulation part of the AIDA detector description toolkit
  namespace sim {

    template <class T=podio::EventStore> void EventParameters::extractParameters(T& event){
      auto& lcparameters = event.getEventMetaData();

      for(auto const& ival: this->intParameters()) {
        lcparameters.setValues(ival.first, ival.second);
      }
      for(auto const& ival: this->fltParameters()) {
        lcparameters.setValues(ival.first, ival.second);
      }
      for(auto const& ival: this->strParameters()) {
        lcparameters.setValues(ival.first, ival.second);
      }
    }

    class  Geant4ParticleMap;

    /// Base class to output Geant4 event data to EDM4hep
    /**
     *  \author  F.Gaede
     *  \version 1.0
     *  \ingroup DD4HEP_SIMULATION
     */
    class Geant4Output2EDM4hep : public Geant4OutputAction  {
    protected:
      podio::EventStore*  m_store;
      podio::ROOTWriter*  m_file;
      int              m_runNo;
      int              m_runNumberOffset;
      int              m_eventNumberOffset;
      std::map< std::string, std::string > m_runHeader;
      std::map< std::string, std::string > m_eventParametersInt;
      std::map< std::string, std::string > m_eventParametersFloat;
      std::map< std::string, std::string > m_eventParametersString;
      bool m_FirstEvent =  true  ;

      std::unordered_map<std::string, podio::CollectionBase*> m_collections;

      /// create the podio collections for the particles and hits
      void createCollections(OutputContext<G4Event>& ctxt) ;
      /// Data conversion interface for MC particles to EDM4hep format
      void saveParticles(Geant4ParticleMap* particles);
    public:
      /// Standard constructor
      Geant4Output2EDM4hep(Geant4Context* ctxt, const std::string& nam);
      /// Default destructor
      virtual ~Geant4Output2EDM4hep();
      /// Callback to store the Geant4 run information
      virtual void beginRun(const G4Run* run);
      /// Callback to store the Geant4 run information
      virtual void endRun(const G4Run* run);

      /// Callback to store the Geant4 run information
      virtual void saveRun(const G4Run* run);
      /// Callback to store the Geant4 event
      virtual void saveEvent( OutputContext<G4Event>& ctxt);
      /// Callback to store each Geant4 hit collection
      virtual void saveCollection( OutputContext<G4Event>& ctxt, G4VHitsCollection* collection);
      /// Commit data at end of filling procedure
      virtual void commit( OutputContext<G4Event>& ctxt);

      /// begin-of-event callback - creates EDM4hep event and adds it to the event context
      virtual void begin(const G4Event* event);
    protected:
      /// Fill event parameters in EDM4hep event
      template <typename T>
      void saveEventParameters(const std::map<std::string, std::string >& parameters);
    };
    
    /// Fill event parameters in EDM4hep event
    template <typename T>
    inline void Geant4Output2EDM4hep::saveEventParameters(const std::map<std::string, std::string >& parameters)  {
      for(std::map<std::string, std::string >::const_iterator iter = parameters.begin(), endIter = parameters.end() ; iter != endIter ; ++iter)  {
        T parameter;
        std::istringstream iss(iter->second);
        if ( (iss >> parameter).fail() )  {
          printout(FATAL,"saveEventParameters","+++ Event parameter %s: FAILED to convert to type :%s",iter->first.c_str(),typeid(T).name());
          continue;
        }
	auto& evtMD = m_store->getEventMetaData();
	evtMD.setValue(iter->first,parameter);
      }
    }

    /// Fill event parameters in EDM4hep event - std::string specialization
    template <>
    inline void Geant4Output2EDM4hep::saveEventParameters<std::string>(const std::map<std::string, std::string >& parameters)  {
      for(std::map<std::string, std::string >::const_iterator iter = parameters.begin(), endIter = parameters.end() ; iter != endIter ; ++iter)  {
	auto& evtMD = m_store->getEventMetaData();
	evtMD.setValue(iter->first,iter->second);
      }
    }

  }    // End namespace sim
}      // End namespace dd4hep
#endif // DD4HEP_DDG4_GEANT4OUTPUT2EDM4hep_H

//==========================================================================
//  AIDA Detector description implementation 
//--------------------------------------------------------------------------
// Copyright (C) Organisation europeenne pour la Recherche nucleaire (CERN)
// All rights reserved.
//
// For the licensing terms see $DD4hepINSTALL/LICENSE.
// For the list of contributors see $DD4hepINSTALL/doc/CREDITS.
//
// Author     : F.Gaede, DESY
//
//==========================================================================

// Framework include files
#include "DD4hep/InstanceCount.h"
#include "DD4hep/Detector.h"
#include "DDG4/Geant4HitCollection.h"
#include "DDG4/Geant4DataConversion.h"
#include "DDG4/Geant4Context.h"
#include "DDG4/Geant4Particle.h"
#include "DDG4/Geant4Data.h"
#include "DDG4/Geant4Action.h"

//#include "DDG4/Geant4Output2EDM4hep.h"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"
#include "G4Event.hh"
#include "G4Run.hh"


using namespace dd4hep::sim;
using namespace dd4hep;
using namespace std;
namespace {
  G4Mutex action_mutex=G4MUTEX_INITIALIZER;
}

#include "DDG4/Factories.h"
DECLARE_GEANT4ACTION(Geant4Output2EDM4hep)

/// Standard constructor
Geant4Output2EDM4hep::Geant4Output2EDM4hep(Geant4Context* ctxt, const string& nam)
: Geant4OutputAction(ctxt,nam), m_store(0), m_file(0), m_runNo(0), m_runNumberOffset(0), m_eventNumberOffset(0)
{
  declareProperty("RunHeader", m_runHeader);
  declareProperty("EventParametersInt",    m_eventParametersInt);
  declareProperty("EventParametersFloat",  m_eventParametersFloat);
  declareProperty("EventParametersString", m_eventParametersString);
  declareProperty("RunNumberOffset", m_runNumberOffset);
  declareProperty("EventNumberOffset", m_eventNumberOffset);
  printout( INFO, "Geant4Output2EDM4hep" ," instantiated ..." ) ;
  InstanceCount::increment(this);
}

/// Default destructor
Geant4Output2EDM4hep::~Geant4Output2EDM4hep()  {
  G4AutoLock protection_lock(&action_mutex);
  if ( m_file )  {
    m_file->finish();
    detail::deletePtr(m_file);
  }
  if (nullptr != m_store) {
    delete m_store;
  }
  InstanceCount::decrement(this);
}

// Callback to store the Geant4 run information
void Geant4Output2EDM4hep::beginRun(const G4Run* run)  {
  G4AutoLock protection_lock(&action_mutex);
  if ( 0 == m_file && !m_output.empty() )   {
    m_store = new podio::EventStore ;
    m_file = new podio::ROOTWriter(m_output, m_store);

    printout( INFO, "Geant4Output2EDM4hep" ," opened %s for output", m_output.c_str() ) ;
  }
  
  saveRun(run);
}

/// Callback to store the Geant4 run information
void Geant4Output2EDM4hep::endRun(const G4Run* /*run*/)  {
  // saveRun(run);
}

/// Commit data at end of filling procedure
void Geant4Output2EDM4hep::commit( OutputContext<G4Event>& /* ctxt */)   {
  if ( m_file )   {
    G4AutoLock protection_lock(&action_mutex);
    m_file->writeEvent();
    m_store->clearCollections();
    return;
  }
  except("+++ Failed to write output file. [Stream is not open]");
}

/// Callback to store the Geant4 run information
void Geant4Output2EDM4hep::saveRun(const G4Run* /*run*/)  {
  //G4AutoLock protection_lock(&action_mutex);

  printout( WARNING, "Geant4Output2EDM4hep" ,"saveRun(): RunHeader not implemented in EDM4hep, nothing written ..." ) ;

  // --- write an edm4hep::RunHeader ---------
  //FIXME: need a suitable Runheader object in EDM4hep

//  edm4hep::LCRunHeaderImpl* rh =  new edm4hep::LCRunHeaderImpl;
//  for (std::map< std::string, std::string >::iterator it = m_runHeader.begin(); it != m_runHeader.end(); ++it) {
//    rh->parameters().setValue( it->first, it->second );
//  }
//  m_runNo = m_runNumberOffset > 0 ? m_runNumberOffset + run->GetRunID() : run->GetRunID();
//  rh->parameters().setValue("GEANT4Version", G4Version);
//  rh->parameters().setValue("DD4HEPVersion", versionString());
//  rh->setRunNumber(m_runNo);
//  rh->setDetectorName(context()->detectorDescription().header().name());
//  m_file->writeRunHeader(rh);
}

void Geant4Output2EDM4hep::begin(const G4Event* /* event */)  {
}

/// Data conversion interface for MC particles to EDM4hep format
void Geant4Output2EDM4hep::saveParticles(Geant4ParticleMap* particles)    {
  typedef detail::ReferenceBitMask<const int> PropertyMask;
  typedef Geant4ParticleMap::ParticleMap ParticleMap;
  const ParticleMap& pm = particles->particleMap;
  auto mcpc = static_cast<edm4hep::MCParticleCollection*>(m_collections["MCParticles"]);

  if ( pm.size() > 0 )  {
    size_t cnt = 0;
    // Mapping of ids in the ParticleMap to indices in the MCParticle collection
    map<int,int> p_ids;
    vector<const Geant4Particle*> p_part;
    p_part.reserve(pm.size());
    // First create the particles
    for (const auto& iParticle : pm) {
      int id = iParticle.first;
      const Geant4ParticleHandle p = iParticle.second;
      PropertyMask mask(p->status);
      //      std::cout << " ********** mcp status : 0x" << std::hex << p->status << ", mask.isSet(G4PARTICLE_GEN_STABLE) x" << std::dec << mask.isSet(G4PARTICLE_GEN_STABLE)  <<std::endl ;
      const G4ParticleDefinition* def = p.definition();
      auto mcp = mcpc->create();
      mcp.setPDG(p->pdgID);

      float ps_fa[3] = {float(p->psx/CLHEP::GeV),float(p->psy/CLHEP::GeV),float(p->psz/CLHEP::GeV)};
      mcp.setMomentum( ps_fa );

      float pe_fa[3] = {float(p->pex/CLHEP::GeV),float(p->pey/CLHEP::GeV),float(p->pez/CLHEP::GeV)};
      mcp.setMomentumAtEndpoint( pe_fa );

      double vs_fa[3] = { p->vsx/CLHEP::mm, p->vsy/CLHEP::mm, p->vsz/CLHEP::mm } ;
      mcp.setVertex( vs_fa );

      double ve_fa[3] = { p->vex/CLHEP::mm, p->vey/CLHEP::mm, p->vez/CLHEP::mm } ;
      mcp.setEndpoint( ve_fa );

      mcp.setTime(p->time/CLHEP::ns);
      mcp.setMass(p->mass/CLHEP::GeV);
      mcp.setCharge(def ? def->GetPDGCharge() : 0); // Charge(e+) = 1 !

      // Set generator status
      mcp.setGeneratorStatus(0);
      if( p->genStatus ) {
        mcp.setGeneratorStatus( p->genStatus ) ;
      } else {
        if ( mask.isSet(G4PARTICLE_GEN_STABLE) )             mcp.setGeneratorStatus(1);
        else if ( mask.isSet(G4PARTICLE_GEN_DECAYED) )       mcp.setGeneratorStatus(2);
        else if ( mask.isSet(G4PARTICLE_GEN_DOCUMENTATION) ) mcp.setGeneratorStatus(3);
        else if ( mask.isSet(G4PARTICLE_GEN_BEAM) )          mcp.setGeneratorStatus(4);
        else if ( mask.isSet(G4PARTICLE_GEN_OTHER) )         mcp.setGeneratorStatus(9);
      }

      // Set simulation status
      mcp.setCreatedInSimulation(         mask.isSet(G4PARTICLE_SIM_CREATED) );
      mcp.setBackscatter(                 mask.isSet(G4PARTICLE_SIM_BACKSCATTER) );
      mcp.setVertexIsNotEndpointOfParent( mask.isSet(G4PARTICLE_SIM_PARENT_RADIATED) );
      mcp.setDecayedInTracker(            mask.isSet(G4PARTICLE_SIM_DECAY_TRACKER) );
      mcp.setDecayedInCalorimeter(        mask.isSet(G4PARTICLE_SIM_DECAY_CALO) );
      mcp.setHasLeftDetector(             mask.isSet(G4PARTICLE_SIM_LEFT_DETECTOR) );
      mcp.setStopped(                     mask.isSet(G4PARTICLE_SIM_STOPPED) );
      mcp.setOverlay(                     false );

      //fg: if simstatus !=0 we have to set the generator status to 0:
      if( mcp.isCreatedInSimulation() )
        mcp.setGeneratorStatus( 0 )  ;

      mcp.setSpin(p->spin);
      mcp.setColorFlow(p->colorFlow);

      p_ids[id] = cnt++;
      p_part.push_back(p);
    }

    // Now establish parent-daughter relationships
    for(size_t i=0; i < p_ids.size(); ++i)   {
      const Geant4Particle* p = p_part[i];
      auto q = (*mcpc)[i];

      for (const auto& idau : p->daughters) {
        const auto k = p_ids.find(idau);
        if (k == p_ids.end()) {
          printout(FATAL,"Geant4Conversion","+++ Particle %d: FAILED to find daughter with ID:%d",p->id,idau);
          continue;
        }
        int iqdau = (*k).second;
        auto qdau = (*mcpc)[iqdau];
        qdau.addToParents(q);
        q.addToDaughters(qdau);
      }

      for (const auto& ipar : p->parents) {
        if (ipar >= 0) { // A parent ID of -1 means NO parent, because a base of 0 is perfectly legal
          const auto k = p_ids.find(ipar);
          if (k == p_ids.end()) {
            printout(FATAL,"Geant4Conversion","+++ Particle %d: FAILED to find parent with ID:%d",p->id,ipar);
            continue;
          }
          int iqpar = (*k).second;
          auto qpar = (*mcpc)[iqpar];
          q.addToParents(qpar);
          qpar.addToDaughters(q);
        }
      }
    }
  }
}

/// Callback to store the Geant4 event
void Geant4Output2EDM4hep::saveEvent(OutputContext<G4Event>& ctxt)  {

  if( m_FirstEvent ){
    createCollections( ctxt ) ;
    m_FirstEvent = false ;
  }

  EventParameters* parameters = context()->event().extension<EventParameters>(false);
  int runNumber(0), eventNumber(0);
  const int eventNumberOffset(m_eventNumberOffset > 0 ? m_eventNumberOffset : 0);
  const int runNumberOffset(m_runNumberOffset > 0 ? m_runNumberOffset : 0);
  // Get event number, run number and parameters from extension ...
  if ( parameters ) {
    runNumber = parameters->runNumber() + runNumberOffset;
    eventNumber = parameters->eventNumber() + eventNumberOffset;
    parameters->extractParameters(*m_store);
  } else { // ... or from DD4hep framework
    runNumber = m_runNo + runNumberOffset;
    eventNumber = ctxt.context->GetEventID() + eventNumberOffset;
  }
  printout(INFO,"Geant4Output2EDM4hep","+++ Saving EDM4hep event %d run %d.", eventNumber, runNumber);

  // this does not compile as create() is we only get a const ref - need to review PODIO EventStore API
  // auto& evtHCol = m_store->get<edm4hep::EventHeaderCollection>("EventHeader") ;
// auto evtHdr = evtHCol.create() ;
  auto* evtHCol = static_cast<edm4hep::EventHeaderCollection*>(m_collections["EventHeader"]);
  auto evtHdr = evtHCol->create() ;

  evtHdr.setRunNumber(runNumber);
  evtHdr.setEventNumber(eventNumber);
//not implemented in EDM4hep ?  evtHdr.setDetectorName(context()->detectorDescription().header().name());
  evtHdr.setTimeStamp( std::time(nullptr) ) ;

  saveEventParameters<int>(m_eventParametersInt);
  saveEventParameters<float>(m_eventParametersFloat);
  saveEventParameters<std::string>(m_eventParametersString);

  Geant4ParticleMap* part_map = context()->event().extension<Geant4ParticleMap>(false);
  if ( part_map )   {
    print("+++ Saving %d EDM4hep particles....",int(part_map->particleMap.size()));
    if ( part_map->particleMap.size() > 0 )  {
      saveParticles(part_map);
    }
  }
}

/// Callback to store each Geant4 hit collection
void Geant4Output2EDM4hep::saveCollection(OutputContext<G4Event>& /*ctxt*/, G4VHitsCollection* collection)  {

  size_t nhits = collection->GetSize();
  std::string colName = collection->GetName();

  printout(DEBUG,"Geant4Output2EDM4hep","+++ Saving EDM4hep collection %s with %d entries.",
     colName.c_str(),int(nhits));

  Geant4HitCollection* coll = dynamic_cast<Geant4HitCollection*>(collection);
  if( coll == nullptr ){
    printout(ERROR, "Geant4Output2EDM4hep" , " no Geant4HitCollection:  %s ", colName.c_str() );
    return ;
  }

  Geant4ParticleMap* pm = context()->event().extension<Geant4ParticleMap>(false);

  auto* mcpc = static_cast<edm4hep::MCParticleCollection*>(m_collections["MCParticles"]);

  //-------------------------------------------------------------------
  if( typeid( Geant4Tracker::Hit ) == coll->type().type()  ){

    auto* sthc = static_cast<edm4hep::SimTrackerHitCollection*>(m_collections[colName]);

    for(unsigned i=0 ; i < nhits ; ++i){
      auto sth = sthc->create() ;

      const Geant4Tracker::Hit* hit = coll->hit(i);
      const Geant4Tracker::Hit::Contribution& t = hit->truth;
      int trackID = pm->particleID(t.trackID);

      auto mcp = mcpc->at(trackID);

      sth.setCellID( hit->cellID ) ;
      sth.setEDep(hit->energyDeposit/CLHEP::GeV);
      sth.setPathLength(hit->length/CLHEP::mm);
      sth.setTime(hit->truth.time/CLHEP::ns);
      sth.setMCParticle(mcp);
      sth.setPosition({hit->position.x()/CLHEP::mm,
           hit->position.y()/CLHEP::mm,
           hit->position.z()/CLHEP::mm});
      sth.setMomentum(edm4hep::Vector3f(hit->momentum.x()/CLHEP::GeV,
          hit->momentum.y()/CLHEP::GeV,
          hit->momentum.z()/CLHEP::GeV ));

      auto particleIt = pm->particles().find(trackID);
      if( ( particleIt != pm->particles().end()) ){
        // if the original track ID of the particle is not the same as the
        // original track ID of the hit it was produced by an MCParticle that
        // is no longer stored
        sth.setProducedBySecondary( (particleIt->second->originalG4ID != t.trackID) );
      }
    }
  //-------------------------------------------------------------------
  }
  else if( typeid( Geant4Calorimeter::Hit ) == coll->type().type() ){

    Geant4Sensitive*       sd      = coll->sensitive();
    int hit_creation_mode = sd->hitCreationMode();

    auto* sCaloHitColl = static_cast<edm4hep::SimCalorimeterHitCollection*>(m_collections[colName]);

    colName += "Contributions"  ;

    auto* sCaloHitContColl = static_cast<edm4hep::CaloHitContributionCollection*>(m_collections[colName]);


    for(unsigned i=0 ; i < nhits ; ++i){
      auto sch = sCaloHitColl->create() ;

      const Geant4Calorimeter::Hit* hit = coll->hit(i);

      sch.setCellID( hit->cellID );
      sch.setPosition({
           float(hit->position.x()/CLHEP::mm),
           float(hit->position.y()/CLHEP::mm),
           float(hit->position.z()/CLHEP::mm)});
      sch.setEnergy( hit->energyDeposit/CLHEP::GeV );

      // now add the individual step contributions
      for(Geant4HitData::Contributions::const_iterator ci=hit->truth.begin();
        ci!=hit->truth.end(); ++ci){

        auto sCaloHitCont = sCaloHitContColl->create();
        sch.addToContributions( sCaloHitCont );

        const Geant4HitData::Contribution& c = *ci;
        int trackID = pm->particleID(c.trackID);
        auto mcp = mcpc->at(trackID);

        sCaloHitCont.setEnergy( c.deposit/CLHEP::GeV );
        sCaloHitCont.setTime( c.time/CLHEP::ns );
        sCaloHitCont.setParticle( mcp );

        if ( hit_creation_mode == Geant4Sensitive::DETAILED_MODE )     {
          sCaloHitCont.setPDG( c.pdgID );
          sCaloHitCont.setStepPosition( edm4hep::Vector3f(
            c.x/CLHEP::mm,
            c.y/CLHEP::mm, 
            c.z/CLHEP::mm) );
        }
      }
    }
  //-------------------------------------------------------------------
  } else {

    printout(ERROR, "Geant4Output2EDM4hep" , " unknown type in Geant4HitCollection  %s ",
	     coll->type().type().name() );
  }
}

void Geant4Output2EDM4hep::createCollections(OutputContext<G4Event>& ctxt){

  auto* mcColl = new edm4hep::MCParticleCollection();
  m_collections.emplace("MCParticles", mcColl);
  m_store->registerCollection("MCParticles", mcColl);
  m_file->registerForWrite("MCParticles");
  printout(DEBUG,"Geant4Output2EDM4hep","+++ created collection MCParticles" );

  auto* evtHeader = new edm4hep::EventHeaderCollection();
  m_collections.emplace("EventHeader", evtHeader);
  m_store->registerCollection("EventHeader", evtHeader);
  m_file->registerForWrite("EventHeader");
  printout(DEBUG,"Geant4Output2EDM4hep","+++ created collection EventHeader" );


  const G4Event* evt = ctxt.context ;
  G4HCofThisEvent* hce = evt->GetHCofThisEvent();
  int nCol = hce->GetNumberOfCollections();

  for (int i = 0; i < nCol; ++i) {
    G4VHitsCollection* hc = hce->GetHC(i);
    std::string colName =  hc->GetName() ;
    Geant4HitCollection* coll = dynamic_cast<Geant4HitCollection*>(hc);
    if( coll == nullptr ){
      printout(WARNING, "Geant4Output2EDM4hep" , " no Geant4HitCollection:  %s ", colName.c_str() );
      continue ;
    }

    Geant4Sensitive* sd = coll->sensitive();
    string sd_enc = dd4hep::sim::Geant4ConversionHelper::encoding(sd->sensitiveDetector());

    if( typeid( Geant4Tracker::Hit ) == coll->type().type()  ){

      auto* sthc = new edm4hep::SimTrackerHitCollection();
      m_collections.emplace(colName, sthc);
      m_store->registerCollection(colName, sthc);
      m_file->registerForWrite(colName);
      auto& sthc_md = m_store->getCollectionMetaData( sthc->getID() );
      sthc_md.setValue("CellIDEncodingString", sd_enc);
      printout(DEBUG,"Geant4Output2EDM4hep","+++ created collection %s",colName.c_str() );
    }
    else if( typeid( Geant4Calorimeter::Hit ) == coll->type().type() ){

      auto* schc = new edm4hep::SimCalorimeterHitCollection();
      m_collections.emplace(colName, schc);
      m_store->registerCollection(colName, schc);
      m_file->registerForWrite(colName);
      auto& schc_md = m_store->getCollectionMetaData( schc->getID() );
      schc_md.setValue("CellIDEncodingString", sd_enc);
      printout(DEBUG,"Geant4Output2EDM4hep","+++ created collection %s",colName.c_str() );

      colName += "Contributions"  ;
      auto* chContribColl = new edm4hep::CaloHitContributionCollection();
      m_collections.emplace(colName, chContribColl);
      m_store->registerCollection(colName, chContribColl);
      m_file->registerForWrite(colName);
      printout(DEBUG,"Geant4Output2EDM4hep","+++ created collection %s",colName.c_str() );

    } else {

      printout(WARNING, "Geant4Output2EDM4hep" ,
	       " unknown type in Geant4HitCollection  %s ", coll->type().type().name() );
    }
  }


}
