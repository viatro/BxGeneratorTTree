// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "BxStackingTTree.hh"
#include "BxStackingTTreeMessenger.hh"
#include "BxPrimaryGeneratorAction.hh"
#include "BxGeneratorTTree.hh"
#include "BxTrackInformation.hh"
#include "BxOutputVertex.hh"
#include "BxManager.hh"
#include "BxLogger.hh"

#include "G4ParticleDefinition.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

using namespace std;

BxStackingTTree::BxStackingTTree() : fIsFirst(true), fMode() {
    fMessenger = new BxStackingTTreeMessenger(this);
    BxLog(routine) << "TTree Stacking Method Active" << endlog;
}

BxStackingTTree::~BxStackingTTree(){;}

G4ClassificationOfNewTrack BxStackingTTree::BxClassifyNewTrack (const G4Track* aTrack) {
    if (fMode.any()) {
        const G4ParticleDefinition* particleDef = aTrack->GetParticleDefinition();
        G4int pdg_code = particleDef->GetPDGEncoding();
        if (pdg_code == 12 || pdg_code == -12 || pdg_code == 14 || pdg_code == -14 || pdg_code == 16 || pdg_code == -16) return fKill;
        
        const G4VProcess* creator = aTrack->GetCreatorProcess();
        if (!creator) return fUrgent; //particle from event generator
        
        if (fMode.test(0)) {
            if (creator->GetProcessName() == "nCapture" && particleDef->GetParticleName() == "gamma") {
                BxGeneratorTTree::ParticleInfo particle_info = fGenerator->GetCurrentParticleInfo();
                
                if (!fGenerator->GetCurrentSplitMode()) {
                    BxTrackInformation* track_info = static_cast<BxTrackInformation*>(aTrack->GetUserInformation());
                    if (track_info)  particle_info.p_index = track_info->GetPrimaryTrackID();
                }
                BxLog(trace) << "STACKING: MODE 0 POSTPONING: p_index = " << particle_info.p_index << endlog;
                particle_info.pdg_code = pdg_code;
                particle_info.energy = aTrack->GetKineticEnergy(); // /MeV;
                particle_info.momentum = aTrack->GetMomentumDirection();
                particle_info.position = aTrack->GetPosition(); // /m;
                particle_info.time = aTrack->GetGlobalTime(); // /ns;
                particle_info.polarization = aTrack->GetPolarization();
                particle_info.status = 1;
                
                fGenerator->PushFrontToDeque(particle_info);
                
                return fKill;
            }
        }
        if (fMode.test(1)) {
            if (creator->GetProcessName() == "RadioactiveDecay") {
                
            }
        }
        if (fMode.test(2)) {
            
        }
    }
    return fUrgent;
}

void BxStackingTTree::BxNewStage() {;}

void BxStackingTTree::BxPrepareNewEvent() {
    if (fIsFirst) {
        fGenerator = dynamic_cast<BxGeneratorTTree*>(static_cast<const BxPrimaryGeneratorAction*>(BxManager::Get()->GetUserPrimaryGeneratorAction())->GetBxGenerator());
        if (!fGenerator) {
            BxLog(error) << "TTree stacking can be used only with TTree generator!" << endlog;
            BxLog(fatal) << "FATAL" << endlog;
        }
        fIsFirst = false;
    }
}


