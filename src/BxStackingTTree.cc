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
#include "BxTrackInformation.hh"
#include "BxOutputVertex.hh"
#include "BxManager.hh"
#include "BxLogger.hh"

#include "G4ParticleDefinition.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

using namespace std;

BxStackingTTree::BxStackingTTree() : fIsFirst(true), fMode(1)/*TODO: change to empty when Manager*/ {
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
                BxGeneratorTTree::ParticleInfo particle_info = generator->GetCurrentParticleInfo();
                
                if (!generator->GetCurrentSplitMode()) {
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
                
                generator->PushFrontToDeque(particle_info);
                
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
        generator = dynamic_cast<BxGeneratorTTree*>(static_cast<const BxPrimaryGeneratorAction*>(BxManager::Get()->GetUserPrimaryGeneratorAction())->GetBxGenerator());
        if (!generator) {
            BxLog(error) << "TTree stacking can be used only with TTree generator!" << endlog;
            BxLog(fatal) << "FATAL" << endlog;
        }
        fIsFirst = false;
    }
}


