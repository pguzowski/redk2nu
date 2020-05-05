////////////////////////////////////////////////////////////////////////
// Class:       ReDk2Nu
// Plugin Type: producer (art v3_01_02)
// File:        ReDk2Nu_module.cc
//
// Generated at Mon May  4 15:17:36 2020 by Pawel Guzowski using cetskelgen
// from cetlib version v3_05_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "nusimdata/SimulationBase/MCTruth.h"
#include "nusimdata/SimulationBase/MCFlux.h"
#include "dk2nu/tree/dk2nu.h"
#include "lardata/Utilities/AssociationUtil.h"
#include "art/Persistency/Common/PtrMaker.h"

#include <memory>

#include <map>
#include <vector>
#include <string>
#include <fstream>

#include "TFile.h"
#include "TTree.h"

namespace redk2nu {
  class ReDk2Nu;
}


class redk2nu::ReDk2Nu : public art::EDProducer {
public:
  explicit ReDk2Nu(fhicl::ParameterSet const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.
  virtual ~ReDk2Nu();

  // Plugins should not be copied or assigned.
  ReDk2Nu(ReDk2Nu const&) = delete;
  ReDk2Nu(ReDk2Nu&&) = delete;
  ReDk2Nu& operator=(ReDk2Nu const&) = delete;
  ReDk2Nu& operator=(ReDk2Nu&&) = delete;

  // Required functions.
  void produce(art::Event& e) override;

private:

  // Declare member data here.
  
  std::string fGenLabel;
  std::string fLocTemplate;

  // run, event -> list of entries in file
  std::map<int, std::map<int, std::vector<long>>> fMapOfFluxTreeEntries;
  std::map<int, TTree*> fTrees;

  bsim::Dk2Nu* dk2nu_entry;


};


redk2nu::ReDk2Nu::ReDk2Nu(fhicl::ParameterSet const& p)
  : EDProducer{p}, fGenLabel(p.get<std::string>("truth_label")),
  fLocTemplate(p.get<std::string>("flux_location_template"))
  // ,
  // More initializers here.
{
  // Call appropriate produces<>() functions here.
  // Call appropriate consumes<>() for any products to be retrieved by this module.
  produces< std::vector<bsim::Dk2Nu> >();
  produces< art::Assns<simb::MCTruth, bsim::Dk2Nu> >();

  consumes< std::vector<simb::MCTruth> >(fGenLabel);
  consumes< std::vector<simb::MCFlux>  >(fGenLabel);
  consumes< art::Assns<simb::MCTruth, simb::MCFlux> >(fGenLabel);

  dk2nu_entry = new bsim::Dk2Nu;
}

redk2nu::ReDk2Nu::~ReDk2Nu() {
  delete dk2nu_entry;
}

void redk2nu::ReDk2Nu::produce(art::Event& e)
{
  // Implementation of required member function here.
  art::Handle<std::vector<simb::MCFlux>> fluxHandle;
  art::Handle<std::vector<simb::MCTruth>> truthHandle;
  std::unique_ptr<std::vector<bsim::Dk2Nu>> dk2nucol(new std::vector<bsim::Dk2Nu>);
  std::unique_ptr< art::Assns<simb::MCTruth, bsim::Dk2Nu> > tdkassn(new art::Assns<simb::MCTruth, bsim::Dk2Nu>);
  art::PtrMaker<bsim::Dk2Nu> makeDk2NuPtr(e);
  if(e.getByLabel(fGenLabel, fluxHandle) && e.getByLabel(fGenLabel, truthHandle)) {
    art::FindManyP<simb::MCTruth> flux2truth(fluxHandle, e, fGenLabel);
    for(size_t iflux = 0; iflux < fluxHandle->size(); ++iflux) {
      auto const& f = fluxHandle->at(iflux);
      const int run = f.frun;
      auto clear_cache = [&]() {
        for(auto const& rev : fMapOfFluxTreeEntries) {
          std::string fname = Form("%d.cache",rev.first);
          if(!std::ifstream(fname.c_str()).good()) {
            std::ofstream f(fname, std::ofstream::binary);
            for(auto const& p : rev.second) {
              int event = p.first;
              auto entries = p.second;
              for(auto const& q : entries) {
                long entry = q;
                f.write((char*)&event,sizeof(event));
                f.write((char*)&entry,sizeof(entry));
              }
            }
            f.close();
          }
        }
        fMapOfFluxTreeEntries.clear();
      };
      if(fTrees.find(run) == fTrees.end()) {
        std::string fname = Form(fLocTemplate.c_str(), run);
        std::cout << "opening file " << fname << std::endl; 
        TFile *file = new TFile(fname.c_str());
        TTree *t = (TTree*)file->Get("dk2nuTree");
        if(!t) {
          throw cet::exception("LogicError") << "Cannot find dk2nuTree in flux file "<<fname <<  std::endl;
        }
        t->SetBranchAddress("dk2nu", &dk2nu_entry);
        clear_cache();
        for(long i = 0; i < t->GetEntries(); ++i) {
          t->GetEntry(i);
          fMapOfFluxTreeEntries[run][dk2nu_entry->potnum].push_back(i);
        }
        fTrees[run] = t;
      }
      if(fMapOfFluxTreeEntries.find(run) == fMapOfFluxTreeEntries.end()) {
        clear_cache();
        std::string fname = Form("%d.cache",run);
        std::ifstream f(fname.c_str(),std::ifstream::binary);
          if(f.good()) {
            int event;
            long entry;
            while(true) {
              f.read((char*)&event,sizeof(event));
              if(!f.good()) break;
              f.read((char*)&entry,sizeof(entry));
              if(!f.good()) break;
              fMapOfFluxTreeEntries[run][event].push_back(entry);
            }
          } else {
            throw cet::exception("LogicError") << "Cannot find cache "<<fname <<  std::endl;
          }
          f.close();
      }
      const int event = f.fevtno;
      if(fMapOfFluxTreeEntries[run].find(event) == fMapOfFluxTreeEntries[run].end()) {
        throw cet::exception("LogicError") << "Cannot find potnum "<<event<<" in flux files" <<  std::endl;
      }
      std::vector<bsim::Dk2Nu> found_dk2nus;
      found_dk2nus.clear();
      found_dk2nus.reserve(1);
      const int nu_pdg = f.fntype;
      const double decay_vertex_x = f.fvx;
      const double decay_vertex_y = f.fvy;
      const double decay_vertex_z = f.fvz;
      for(long entry : fMapOfFluxTreeEntries[run][event]) {
        fTrees[run]->GetEntry(entry);
        if(nu_pdg == dk2nu_entry->decay.ntype && 
            decay_vertex_x == dk2nu_entry->decay.vx && 
            decay_vertex_y == dk2nu_entry->decay.vy && 
            decay_vertex_z == dk2nu_entry->decay.vz) {
          found_dk2nus.push_back(*dk2nu_entry);
        }
      }
      if(found_dk2nus.size() != 1){
        
        for(long entry : fMapOfFluxTreeEntries[run][event]) {
          fTrees[run]->GetEntry(entry);
          if(nu_pdg == dk2nu_entry->decay.ntype && 
              decay_vertex_x == dk2nu_entry->decay.vx && 
              decay_vertex_y == dk2nu_entry->decay.vy && 
              decay_vertex_z == dk2nu_entry->decay.vz) {
            std::cerr <<"Error in: Run " << run << " Entry " << entry << std::endl;
          }
        }
        throw cet::exception("LogicError") << "Found "<<found_dk2nus.size()<<" allowed dk2nu entries" <<  std::endl;
      }
      dk2nucol->push_back(found_dk2nus.front());
      auto dk2nuPtr = makeDk2NuPtr(dk2nucol->size()-1);
      for(auto const& mctruthPtr : flux2truth.at(iflux)) {
        tdkassn->addSingle(mctruthPtr, dk2nuPtr);
      }

    }
  }
  else {
    throw cet::exception("Configuration") << "there should be a truth generator product in the event" <<  std::endl;
  }
  e.put(std::move(dk2nucol));
  e.put(std::move(tdkassn));
}

DEFINE_ART_MODULE(redk2nu::ReDk2Nu)
