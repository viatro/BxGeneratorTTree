//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//Created by D. Franco
//Revised by A. Caminata and S. Marcocci, Sept. 2014
#include "BxStackingTTree.hh"
#include "BxPrimaryGeneratorAction.hh"
#include "BxGeneratorTTree.hh"
#include "BxOutputVertex.hh"
//#include "BxDataCollection.hh"
#include "BxManager.hh"
//#include "BxEventAction.hh"
#include "BxLogger.hh"
//#include "BxReadParameters.hh"

#include "G4ParticleDefinition.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
//#include "G4SystemOfUnits.hh"

//#include "G4StackManager.hh"
//#include "G4ios.hh"
//#include "G4ParticleTable.hh"
//#include "G4NeutrinoE.hh"
//#include "G4NeutrinoMu.hh"
//#include "G4NeutrinoTau.hh"
//#include "G4AntiNeutrinoE.hh"
//#include "G4AntiNeutrinoMu.hh"
//#include "G4AntiNeutrinoTau.hh"
//#include "G4OpticalPhoton.hh" 
//#include "G4GenericIon.hh" 
//#include "G4Gamma.hh" 
//#include "globals.hh"
//#include "CLHEP/Random/RandExponential.h"

using namespace std;

BxStackingTTree::BxStackingTTree() : fIsFirst(true), fPrimaries(), fMode(1)/*TODO: change to empty when Manager*/, calls(0), fCurrentPrimaryTrackID(-1) {
    BxLog(routine) << "TTree Stacking Method Active" << endlog;
}

BxStackingTTree::~BxStackingTTree(){;}

G4int BxStackingTTree::PrimariesCounter() {
    if (fPrimaries.empty())  return 0;
    else {
        G4int n = 0;
        for (std::map<G4int, G4bool>::iterator it = fPrimaries.begin(); it != fPrimaries.end(); ++it) {
            if (it->second == true) ++n;
        }
        return n;
    }
}

G4ClassificationOfNewTrack BxStackingTTree::BxClassifyNewTrack (const G4Track* aTrack) {
    if (fMode.any()) {
        if (aTrack->GetParentID() == 0) {
            fCurrentPrimaryTrackID = aTrack->GetTrackID();
            BxLog(trace) << "STACKING: calls = " << ++calls << ", trackID = " << fCurrentPrimaryTrackID << endlog;
            //std::map and is_new_primary is needed because somehow at least for electrons (and at least in Cycle 18)
            //BxClassifyNewTrack is called many times for the same particle, thus "aTrack->GetParentID() == 0" can be true
            //many times for the same primary particle, so just (G4int) ++fPrimariesNumber does not play correctly.
            //And std::set is not good here too, because this BxStacking pushs postponed particles to generator as primaries, but with non-zero status.
            //So here std::map < G4int/*trackID*/, G4bool/*is it true TTree's primary and not postponed particle?*/ > is needed.
            G4bool is_new_primary = fPrimaries.insert(std::pair<G4int, G4bool>(fCurrentPrimaryTrackID, true)).second; //if key exists in map it returns false and does not change the value
            BxLog(trace) << "STACKING: is_new_primary = " << is_new_primary << endlog;
            if (is_new_primary) {
                if (generator->GetCurrentParticleInfo().status != 0)  fPrimaries[fCurrentPrimaryTrackID] = false;
                else BxLog(trace) << "STACKING: primary #" << PrimariesCounter() << ", PDG = " << aTrack->GetParticleDefinition()->GetPDGEncoding() << ", trackID = " << fCurrentPrimaryTrackID << endlog;
            }
        } else {
            const G4ParticleDefinition* particleDef = aTrack->GetParticleDefinition();
            G4int pdg_code = particleDef->GetPDGEncoding();
            if (pdg_code == 12 || pdg_code == -12 || pdg_code == 14 || pdg_code == -14 || pdg_code == 16 || pdg_code == -16) return fKill;
            
            const G4VProcess* creator = aTrack->GetCreatorProcess();
            if (!creator) return fUrgent; //event generator
            
            if (fMode.test(0) && creator->GetProcessName() == "nCapture" && particleDef->GetParticleName() == "gamma") {
                BxGeneratorTTree::ParticleInfo particle_info = generator->GetCurrentParticleInfo();
                
                if (!generator->GetCurrentSplitMode())  particle_info.p_index = PrimariesCounter();
                BxLog(trace) << "STACKING: count(fPrimaries, true) = " << PrimariesCounter() << ", PrimaryTrackID = " << fCurrentPrimaryTrackID << endlog;
                particle_info.pdg_code = pdg_code;
                particle_info.energy = aTrack->GetKineticEnergy(); // /MeV;
                particle_info.momentum = aTrack->GetMomentumDirection();
                particle_info.position = aTrack->GetPosition(); // /m;
                particle_info.time = aTrack->GetGlobalTime(); // /ns;
                particle_info.polarization = aTrack->GetPolarization();
                particle_info.status = 1;
                
                generator->PushFrontToDeque(particle_info);
                
                return fKill;
            } else if (fMode.test(1)) {
                if (creator->GetProcessName() == "RadioactiveDecay") {
                    
                }
            } else if (fMode.test(2)) {
                
            }
        }
    }
    return fUrgent;
}

void BxStackingTTree::BxNewStage() {;}

void BxStackingTTree::BxPrepareNewEvent() {
    fPrimaries.clear(); calls = 0;
    if (fIsFirst) {
        generator = dynamic_cast<BxGeneratorTTree*>(dynamic_cast<const BxPrimaryGeneratorAction*>(BxManager::Get()->GetUserPrimaryGeneratorAction())->GetBxGenerator());
        if (!generator) {
            BxLog(error) << "TTree stacking can be used only with TTree generator!" << endlog;
            BxLog(fatal) << "FATAL" << endlog;
        }
        fIsFirst = false;
    }
}


