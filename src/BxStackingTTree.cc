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

#include "G4MuonMinus.hh"
#include "G4Electron.hh"

#include "G4UnitsTable.hh"

using namespace std;

BxStackingTTree::BxStackingTTree()
: fIsFirst(true)
, fMode()
, fKillOpticalPhotons(false)
, fAugerElectron()
, fMuPlusTrackIDs()
, fRadNucleiTrackTimes()
, fRadNucleiLifetimeThreshold(0.)
{
    G4double m_mu = G4MuonMinus::Definition()->GetPDGMass();
    G4double m_e = G4Electron::Definition()->GetPDGMass();
    fEkinMaxMuonDecay = (m_mu*m_mu + m_e*m_e)/(2.*m_mu) - m_e;
    
    fMessenger = new BxStackingTTreeMessenger(this);
    
    BxLog(routine) << "BxStackingTTree built" << endlog;
}

BxStackingTTree::~BxStackingTTree() {}

G4ClassificationOfNewTrack BxStackingTTree::BxClassifyNewTrack (const G4Track* aTrack) {
    const G4ParticleDefinition* particleDef = aTrack->GetParticleDefinition();
    G4int pdg_code = particleDef->GetPDGEncoding();
    
    if (fKillOpticalPhotons && pdg_code == 50) return fKill;
    
    if (fMode.any()) {
        if (fMode.test(1) && particleDef->GetParticleType() == "nucleus" /*&& particleDef->GetPDGLifeTime() > fRadNucleiLifetimeThreshold*/) {
            fRadNucleiTrackTimes.insert(std::pair<G4int, G4double>(aTrack->GetTrackID(), aTrack->GetGlobalTime()));
        }
        if (fMode.test(2) && pdg_code == -13) fMuPlusTrackIDs.insert(aTrack->GetTrackID());
        
        const G4VProcess* creatorProcess = aTrack->GetCreatorProcess();
        if (!creatorProcess) return fUrgent; //particle from event generator
        G4String creatorProcessName = creatorProcess->GetProcessName();
        
        if (fMode.test(0)) {
            if (pdg_code == 22 && creatorProcessName == "nCapture") {
                BxGeneratorTTree::ParticleInfo particle_info = fGenerator->GetCurrentParticleInfo();
                if (!fGenerator->GetCurrentSplitMode()) {
                    BxTrackInformation* track_info = static_cast<BxTrackInformation*>(aTrack->GetUserInformation());
                    if (track_info)  particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[track_info->GetPrimaryTrackID() - 1];
                } else particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[0];
                particle_info.status = 1;
                particle_info.pdg_code = pdg_code;
                particle_info.energy = aTrack->GetKineticEnergy();
                particle_info.momentum = aTrack->GetMomentumDirection();
                particle_info.position = aTrack->GetPosition();
                particle_info.time = aTrack->GetGlobalTime();
                particle_info.polarization = aTrack->GetPolarization();
                
                fGenerator->PushFrontToDeque(particle_info);
                
                return fKill;
            }
        }
        if (fMode.test(1)) {
            if (creatorProcessName == "RadioactiveDecay" && fRadNucleiTrackTimes[aTrack->GetParentID()] >= 0.) {
                BxGeneratorTTree::ParticleInfo particle_info = fGenerator->GetCurrentParticleInfo();
                if (!fGenerator->GetCurrentSplitMode()) {
                    BxTrackInformation* track_info = static_cast<BxTrackInformation*>(aTrack->GetUserInformation());
                    if (track_info)  particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[track_info->GetPrimaryTrackID() - 1];
                } else particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[0];
                particle_info.status = 2;
                particle_info.pdg_code = pdg_code;
                particle_info.energy = aTrack->GetKineticEnergy();
                particle_info.momentum = aTrack->GetMomentumDirection();
                particle_info.position = aTrack->GetPosition();
                particle_info.time = aTrack->GetGlobalTime();
                particle_info.polarization = aTrack->GetPolarization();
                
                fGenerator->PushFrontToDeque(particle_info);
                
                return fKill;
            }
        }
        if (fMode.test(2)) {
            if (creatorProcessName == "muMinusCaptureAtRest") {
                //mean lifetime of muonic carbon is 2.026 mus, of muonic hydrogen is almost equal to free muon lifetime
                if (pdg_code == 11) {
                    //Decay in orbit: 93.6318% of mu- captures for Borexino scintillator
                    //Processes "Decay" and "DecayWithSpin" seem to do not give any significant delay time between mu- and e-
                    //For mu- with uniformly distributed kinetic energies from 0 to 1000 MeV, delay times from "Decay" are up to 20 ns
                    //They are only decays in flight
                    //Stopped mu- are captured by nuclei into muonic atoms, bound mu- decays are handled by "muMinusCaptureAtRest" process
                    //But there are two types of electrons from it:
                    //  1) Auger electrons with tiny energies and times about few ns
                    //  2) electron from bound muon decay (decay in orbit), which we interested in
                    //The first electron from "muMinusCaptureAtRest" is always Auger electron, even if it is the only electron
                    //All Auger electrons have the same track time, not equal to track time of electron from decay
                    //If electron from decay is appear, it is the last electron
                    
                    G4double trackTime = aTrack->GetGlobalTime();
                    if (fAugerElectron.parentID != aTrack->GetParentID()) {
                        fAugerElectron.Set(aTrack->GetParentID(), aTrack->GetTrackID(), trackTime);
                    } else {
                        if (fAugerElectron.parentID == aTrack->GetParentID()) {
                            if (fAugerElectron.time != trackTime && fAugerElectron.trackID != aTrack->GetTrackID()) {
                                BxGeneratorTTree::ParticleInfo particle_info = fGenerator->GetCurrentParticleInfo();
                                if (!fGenerator->GetCurrentSplitMode()) {
                                    BxTrackInformation* track_info = static_cast<BxTrackInformation*>(aTrack->GetUserInformation());
                                    if (track_info)  particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[track_info->GetPrimaryTrackID() - 1];
                                } else particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[0];
                                particle_info.status = 3;
                                particle_info.pdg_code = pdg_code;
                                particle_info.energy = aTrack->GetKineticEnergy();
                                particle_info.momentum = aTrack->GetMomentumDirection();
                                particle_info.position = aTrack->GetPosition();
                                particle_info.time = trackTime;
                                particle_info.polarization = aTrack->GetPolarization();
                                
                                fGenerator->PushFrontToDeque(particle_info);
                                
                                return fKill;
                            }
                        }
                    }
                } else if (aTrack->GetGlobalTime() != fAugerElectron.time && pdg_code != 14 && pdg_code != -12) {
                    //Nuclear capture: 6.3682% of mu- captures for Borexino scintillator
                    //by    mu- + p -> n + nu_mu    or    mu- + (A,Z) -> (A,Z-1) + nu_mu
                    //WARNING! There is a bug in Geant4 versions lower than 10.0.p04 and 10.1.p01: http://bugzilla-geant4.kek.jp/show_bug.cgi?id=1695
                    //         Gammas, protons, neutrons, deutrons, tritons, alphas and residual nuclei with delay time ~ 0.1-10 mus can be produced
                    //         but because of bug they have the same time as Auger electrons
                    BxGeneratorTTree::ParticleInfo particle_info = fGenerator->GetCurrentParticleInfo();
                    if (!fGenerator->GetCurrentSplitMode()) {
                        BxTrackInformation* track_info = static_cast<BxTrackInformation*>(aTrack->GetUserInformation());
                        if (track_info)  particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[track_info->GetPrimaryTrackID() - 1];
                    } else particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[0];
                    particle_info.status = 3;
                    particle_info.pdg_code = pdg_code;
                    particle_info.energy = aTrack->GetKineticEnergy();
                    particle_info.momentum = aTrack->GetMomentumDirection();
                    particle_info.position = aTrack->GetPosition();
                    particle_info.time = aTrack->GetGlobalTime();
                    particle_info.polarization = aTrack->GetPolarization();
                    
                    fGenerator->PushFrontToDeque(particle_info);
                    
                    return fKill;
                }
            }  else if (pdg_code == -11 && (creatorProcessName == "Decay" || creatorProcessName == "DecayWithSpin")
                && fMuPlusTrackIDs.count(aTrack->GetParentID()) && aTrack->GetKineticEnergy() <= fEkinMaxMuonDecay) {
                //There is only free muon decay for mu+, no capture
                BxGeneratorTTree::ParticleInfo particle_info = fGenerator->GetCurrentParticleInfo();
                if (!fGenerator->GetCurrentSplitMode()) {
                    BxTrackInformation* track_info = static_cast<BxTrackInformation*>(aTrack->GetUserInformation());
                    if (track_info)  particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[track_info->GetPrimaryTrackID() - 1];
                } else particle_info.p_index = fGenerator->GetCurrentPrimaryIndexes()[0];
                particle_info.status = 3;
                particle_info.pdg_code = pdg_code;
                particle_info.energy = aTrack->GetKineticEnergy();
                particle_info.momentum = aTrack->GetMomentumDirection();
                particle_info.position = aTrack->GetPosition();
                particle_info.time = aTrack->GetGlobalTime();
                particle_info.polarization = aTrack->GetPolarization();
                
                fGenerator->PushFrontToDeque(particle_info);
                
                return fKill;
            }
        }
    }
    return fUrgent;
}

void BxStackingTTree::BxNewStage() {}

void BxStackingTTree::BxPrepareNewEvent() {
    if (fIsFirst) {
        const BxPrimaryGeneratorAction* primGen = static_cast<const BxPrimaryGeneratorAction*>(BxManager::Get()->GetUserPrimaryGeneratorAction());
        fGenerator = dynamic_cast<BxGeneratorTTree*>(primGen->GetBxGenerator());
        if (!fGenerator) {
            BxLog(error) << "TTree stacking can be used only with TTree generator!" << endlog;
            BxLog(fatal) << "FATAL" << endlog;
        }
        fIsFirst = false;
    }
    fAugerElectron.Set(0,0,0.);
    fMuPlusTrackIDs.clear();
    fRadNucleiTrackTimes.clear();
}
