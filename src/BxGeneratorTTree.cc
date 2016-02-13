// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "BxGeneratorTTree.hh"
#include "BxOutputVertex.hh"
#include "BxVGenerator.hh"
#include "BxGeneratorTTreeMessenger.hh"
#include "BxLogger.hh"
#include "BxManager.hh"

#include "TString.h"
#include "TObjArray.h"
#include "TTreeFormula.h"
#include "TTreeFormulaManager.h"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleMomentum.hh"
#include "G4PrimaryParticle.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

BxGeneratorTTree::BxGeneratorTTree(): BxVGenerator("BxGeneratorTTree"),
    //fTreeNumber(-1),
    fCurrentEntry(0),
    fFirstEntry(0),
    fLastEntry(0),
    fNEntries(0),
    fIsInitialized(false),
    fParticleCounter(0),
    //fPostponed(0),
    fVarUnit_Ekin(MeV),
    fVarUnit_Momentum(MeV),
    fVarUnit_Coords(m),
    fVarIsSet_EventId(false),
    fVarIsSet_Polarization(false)
    {
    
    fTreeChain = new TChain();
    
    fVarTTF_EventId         = new TTreeFormula("tf", "0" , 0);
    fVarTTF_EventSkip       = new TTreeFormula("tf", "0" , 0);
    fVarTTF_ParticleSkip    = new TTreeFormula("tf", "0" , 0);
    fVarTTF_NParticles      = new TTreeFormula("tf", "1" , 0);
    fVarTTF_Split           = new TTreeFormula("tf", "0" , 0);
    fVarTTF_RotateIso       = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Pdg             = new TTreeFormula("tf", "22", 0);
    fVarTTF_Ekin            = new TTreeFormula("tf", "1" , 0);
    fVarTTF_Momentum[0]     = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Momentum[1]     = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Momentum[2]     = new TTreeFormula("tf", "1" , 0);
    fVarTTF_Coords[0]       = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Coords[1]       = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Coords[2]       = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Polarization[0] = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Polarization[1] = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Polarization[2] = new TTreeFormula("tf", "0" , 0);
    
    fTTFmanager = new TTreeFormulaManager();
    
    fParticleTable = G4ParticleTable::GetParticleTable();
    fParticleGun = new G4ParticleGun();
    
    fRotationAngles = new G4ThreeVector();
    
    fMessenger  = new BxGeneratorTTreeMessenger(this);
    
    BxLog(warning) << "BxGeneratorTTree built" << endlog;
}

BxGeneratorTTree::~BxGeneratorTTree() {
    delete    fMessenger;
    delete    fParticleGun;
    
    delete    fVarTTF_EventId      ;
    delete    fVarTTF_EventSkip    ;
    delete    fVarTTF_ParticleSkip ;
    delete    fVarTTF_NParticles   ;
    delete    fVarTTF_Split        ;
    delete    fVarTTF_RotateIso    ;
    delete    fVarTTF_Pdg          ;
    delete    fVarTTF_Ekin         ;
    delete[]  fVarTTF_Momentum     ;
    delete[]  fVarTTF_Coords       ;
    delete[]  fVarTTF_Polarization ;
    
    delete fRotationAngles;
    
    delete fTreeChain;
}

void BxGeneratorTTree::Initialize() {
    
    BxLog(routine) << "BxGeneratorTTree initialization started" << endlog;
    
    fLastEntry = fTreeChain->GetEntries() - 1;
    if (fFirstEntry > fLastEntry && fTreeChain->LoadTree(0) != -1) {
        BxLog(error) << "First entry " << fFirstEntry << " > max possible entry " << fLastEntry << " for given Tree(Chain)" << endlog ;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fCurrentEntry += fFirstEntry;
    
    //fTreeChain->LoadTree(0);
    
    TObjArray* tobjarr = 0;
    G4int ntokens = 0;
    TString tstring = "";
    
    if ( ! fVarString_EventId.isNull() ) {
        tstring = fVarString_EventId.data();
        if ( ! tstring.IsFloat() ) {
            delete fVarTTF_EventId;
            fVarTTF_EventId = new TTreeFormula("tf", tstring.Data(), fTreeChain);
            if ( fVarTTF_EventId->GetMultiplicity() != 0 ) {
                BxLog(error) << "\"Event ID\" variable has wrong multiplicity!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            //fVarTTF_EventId->SetQuickLoad(true);
            fVarIsSet_EventId = true;
        }
    }
    
    if ( ! fVarString_EventSkip.isNull() ) {
        tstring = fVarString_EventSkip.data();
        delete fVarTTF_EventSkip;
        fVarTTF_EventSkip = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        /*if (fVarTTF_EventSkip->GetMultiplicity() != 0) {
            BxLog(error) << "\"Event skip if\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }*/
        //fVarTTF_EventSkip->SetQuickLoad(true);
    }
    
    if ( ! fVarString_ParticleSkip.isNull() ) {
        tstring = fVarString_ParticleSkip.data();
        delete fVarTTF_ParticleSkip;
        fVarTTF_ParticleSkip = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        /*if (fVarTTF_ParticleSkip->GetMultiplicity() != 0) {
            BxLog(error) << "\"Particle skip if\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }*/
        if (fVarTTF_ParticleSkip->GetMultiplicity()) fTTFmanager->Add(fVarTTF_ParticleSkip);
        //fVarTTF_ParticleSkip->SetQuickLoad(true);
    }
    
    if ( ! fVarString_NParticles.isNull() ) {
        tstring = fVarString_NParticles.data();
        delete fVarTTF_NParticles;
        fVarTTF_NParticles = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if ( fVarTTF_NParticles->GetMultiplicity() != 0 ) {
            BxLog(error) << "\"Number of particles\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        //fVarTTF_NParticles->SetQuickLoad(true);
    }
    
    if ( ! fVarString_Split.isNull() ) {
        tstring = fVarString_Split.data();
        delete fVarTTF_Split;
        fVarTTF_Split = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if ( fVarTTF_Split->GetMultiplicity() != 0 ) {
            BxLog(error) << "\"Split\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        //fVarTTF_Split->SetQuickLoad(true);
    }
    
    if ( ! fVarString_RotateIso.isNull() ) {
        tstring = fVarString_RotateIso.data();
        delete fVarTTF_RotateIso;
        fVarTTF_RotateIso = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if ( fVarTTF_RotateIso->GetMultiplicity() != 0 ) {
            BxLog(error) << "\"RotateIso\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        //fVarTTF_RotateIso->SetQuickLoad(true);
    }
    
    if ( ! fVarString_Pdg.isNull() ) {
        tstring = fVarString_Pdg.data();
        if (tstring.IsDigit()) {
            fParticle = fParticleTable->FindParticle(tstring.Atoi());
            if (!fParticle)    fParticleTable->GetIonTable()->GetIon(tstring.Atoi());
            if (!fParticle) { // Unknown particle
                BxLog(error) << "Unknown PDG code : " << tstring.Atoi() << " !" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
        }
        delete fVarTTF_Pdg;
        fVarTTF_Pdg = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_Pdg->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Pdg);
        //fVarTTF_Pdg->SetQuickLoad(true);
    }
    
    if ( ! fVarString_Ekin.isNull() ) {
        tstring = fVarString_Ekin.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 1 || ntokens > 2) {
            BxLog(error) << "Variable String \"Ekin\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 2) {
            if ( G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(1)->GetName()) != "Energy" ) {
                BxLog(error) << "Variable String \"Ekin\" has wrong unit category or unit name!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarUnit_Ekin = G4UnitDefinition::GetValueOf(tobjarr->UncheckedAt(1)->GetName());
        }
        tstring = TString(tobjarr->UncheckedAt(0)->GetName());
        delete fVarTTF_Ekin;
        fVarTTF_Ekin = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_Ekin->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Ekin);
        //fVarTTF_Ekin->SetQuickLoad(true);
    }
    
    if ( ! fVarString_Momentum.isNull() ) {
        tstring = fVarString_Momentum.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 3 || ntokens > 4) {
            BxLog(error) << "Variable string \"Momentum\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 4) {
            if ( G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(3)->GetName()) != "Energy" ) {
                BxLog(error) << "Variable string \"Momentum\" has wrong unit category or unit name!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarUnit_Momentum = G4UnitDefinition::GetValueOf(tobjarr->UncheckedAt(3)->GetName());
        }
        for (G4int i = 0; i < 3; ++i) {
            tstring = TString(tobjarr->UncheckedAt(i)->GetName());
            delete fVarTTF_Momentum[i];
            fVarTTF_Momentum[i] = new TTreeFormula("tf", tstring.Data(), fTreeChain);
            if (fVarTTF_Momentum[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Momentum[i]);
            //fVarTTF_Momentum[i]->SetQuickLoad(true);
        }
    }
    
    if ( ! fVarString_Coords.isNull() ) {
        tstring = fVarString_Coords.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 3 || ntokens > 4) {
            BxLog(error) << "Variable string \"Coords\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 4) {
            if ( G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(3)->GetName()) != "Length" ) {
                BxLog(error) << "Variable string \"Coords\" has wrong unit category or unit name!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarUnit_Coords = G4UnitDefinition::GetValueOf(tobjarr->UncheckedAt(3)->GetName());
        }
        for (G4int i = 0; i < 3; ++i) {
            tstring = TString(tobjarr->UncheckedAt(i)->GetName());
            delete fVarTTF_Coords[i];
            fVarTTF_Coords[i] = new TTreeFormula("tf", tstring.Data(), fTreeChain);
            if (fVarTTF_Coords[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Coords[i]);
            //fVarTTF_Coords[i]->SetQuickLoad(true);
        }
    }
    
    if ( ! fVarString_Polarization.isNull() ) {
        tstring = fVarString_Polarization.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens != 3) {
            BxLog(error) << "Variable string \"Polarization\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        for (G4int i = 0; i < 3; ++i) {
            tstring = TString(tobjarr->UncheckedAt(i)->GetName());
            delete fVarTTF_Polarization[i];
            fVarTTF_Polarization[i] = new TTreeFormula("tf", tstring.Data(), fTreeChain);
            if (fVarTTF_Polarization[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Polarization[i]);
            //fVarTTF_Polarization[i]->SetQuickLoad(true);
        }
        fVarIsSet_Polarization = true;
    }
    
    fTTFmanager->Sync();
    fTreeChain->SetNotify(fTTFmanager);
    
    BxLog(trace) << "\nTTreeFormula-s :"
        << "\n\tEventId         : " << fVarTTF_EventId         -> GetTitle()
        << "\n\tEventSkip       : " << fVarTTF_EventSkip       -> GetTitle()
        << "\n\tParticleSkip    : " << fVarTTF_ParticleSkip    -> GetTitle()
        << "\n\tNParticles      : " << fVarTTF_NParticles      -> GetTitle()
        << "\n\tSplit           : " << fVarTTF_Split           -> GetTitle()
        << "\n\tRotateIso       : " << fVarTTF_RotateIso       -> GetTitle()
        << "\n\tPdg             : " << fVarTTF_Pdg             -> GetTitle()
        << "\n\tEkin            : " << fVarTTF_Ekin            -> GetTitle()
        << "\n\tMomentum[0]     : " << fVarTTF_Momentum[0]     -> GetTitle()
        << "\n\tMomentum[1]     : " << fVarTTF_Momentum[1]     -> GetTitle()
        << "\n\tMomentum[2]     : " << fVarTTF_Momentum[2]     -> GetTitle()
        << "\n\tCoords[0]       : " << fVarTTF_Coords[0]       -> GetTitle()
        << "\n\tCoords[1]       : " << fVarTTF_Coords[1]       -> GetTitle()
        << "\n\tCoords[2]       : " << fVarTTF_Coords[2]       -> GetTitle()
        << "\n\tPolarization[0] : " << fVarTTF_Polarization[0] -> GetTitle()
        << "\n\tPolarization[1] : " << fVarTTF_Polarization[1] -> GetTitle()
        << "\n\tPolarization[2] : " << fVarTTF_Polarization[2] -> GetTitle()
        << endlog;
    
    BxLog(routine) << "BxGeneratorTTree initialized" << endlog;
}

void BxGeneratorTTree::BxGeneratePrimaries(G4Event *event) {
    if (!fIsInitialized) {
        Initialize();
        fIsInitialized = true;
    }
    
    G4int loadedEntry = fTreeChain->LoadTree(fCurrentEntry);
    
    if (loadedEntry < -1) {
        //if loadedEntry == -1 (i.e. chain is empty) and one (or more) of TTreeFormula-s is not a float number,
        //it already failed in Initialize() with "Bad numerical expression".
        //So it is safe here to use fTreeChain->LoadTree(fCurrentEntry) == -1 as good case
        //to provide possibility for using this generator without TTree.
        
        //error descriptions from TChain::LoadTree()
             if (loadedEntry == -2) BxLog(fatal/*error*/) << "The requested entry number of less than zero or too large for the chain or too large for the large TTree" << endlog;
        else if (loadedEntry == -3) BxLog(fatal/*error*/) << "The file corresponding to the entry could not be correctly open" << endlog;
        else if (loadedEntry == -4) BxLog(fatal/*error*/) << "The TChainElement corresponding to the entry is missing or the TTree is missing from the file"  << endlog;
        //event->SetEventAborted();
        //BxManager::Get()->AbortRun();
        //return;
    }
    
    if (fPostponed.empty()) {
        if ( fCurrentEntry > fFirstEntry + fNEntries - 1  ||  ( fCurrentEntry > fLastEntry  &&  loadedEntry != -1 ) ) {
            event->SetEventAborted();
            BxManager::Get()->AbortRun();
            if ( fCurrentEntry > fLastEntry  &&  loadedEntry != -1 ) BxLog(routine) << "End of Tree(Chain) reached" << endlog;
            return;
        }
    } else {
        std::pair<G4int,G4int> postponed_info = fPostponed.back();
        fPostponed.pop_back();
        BxOutputVertex::Get()->SetEventID(postponed_info.first);
        BxOutputVertex::Get()->SetUserInt1(postponed_info.second);
        BxOutputVertex::Get()->SetUserInt2(1);
        BxOutputVertex::Get()->SetUsers();
        BxLog(trace) << "  Entry " << (fParticleCounter == 0  ?  fCurrentEntry - 1  :  fCurrentEntry)
                     << ", event_id = " << BxOutputVertex::Get()->GetEventID()
                     << " : particle #" << postponed_info.second
                     << " : POSTPONED" << endlog;
        return;
    }
    
    //NOTE: without fTreeChain->SetNotify(fTTFmanager) in BxGeneratorTTree::Initialize()
    //if (fTreeNumber != fTreeChain->GetTreeNumber()) {
    //    fTreeNumber = fTreeChain->GetTreeNumber();
    //    fTTFmanager->UpdateFormulaLeaves();
    //}
    
    if ( fTTFmanager->GetNdata() <= 0  ||  fVarTTF_EventSkip->EvalInstance64(0) )  {
        ++fCurrentEntry;
        return;
    }
    
    G4int event_id = fVarIsSet_EventId ? fVarTTF_EventId->EvalInstance64(0) : fCurrentEntry;
    
    if ( fVarTTF_RotateIso->EvalInstance64(0)  &&  fParticleCounter == 0 )   fRotationAngles->set(twopi * G4UniformRand(), std::acos(1 - 2*G4UniformRand()), twopi * G4UniformRand());
    
    G4bool split = fVarTTF_Split->EvalInstance64(0);
    
    G4int loop_begin = 0;
    G4int loop_end = fVarTTF_NParticles->EvalInstance64(0);
    if (split) {
        loop_begin = fParticleCounter;
        loop_end = fParticleCounter + 1;
    }
    
    for ( G4int loop_iter = loop_begin; loop_iter < loop_end; ++loop_iter, ++fParticleCounter) {
        if ( fVarTTF_ParticleSkip->EvalInstance64(fParticleCounter) )  {
            BxLog(trace) << "  Entry " << fCurrentEntry
                         << ", event_id = " << event_id
                         << " : particle #" << fParticleCounter
                         << " : SKIP" << endlog;
            if (split)  event->SetEventAborted();
            continue;
        }
        
        G4int pdg = fVarTTF_Pdg->EvalInstance64(fParticleCounter);
        fParticle = fParticleTable->FindParticle(pdg);
        if (!fParticle) {
            //fParticle = fParticleTable->GetIonTable()->GetIon(pdg);
            //if (!fParticle) { // Skip unknown particle
                BxLog(warning) << "  Entry " << fCurrentEntry
                               << ", event_id = " << event_id
                               << " : particle #" << fParticleCounter
                               << " : WARNING!" << endlog;
                BxLog(warning) << "  Skipping unknown particle with PDG code " << pdg << endlog;
                if (split)  event->SetEventAborted();
                continue;
            //}
        }
    
        fParticleGun->SetParticleDefinition(fParticle);
        
        G4ParticleMomentum momentum (
            fVarUnit_Momentum * fVarTTF_Momentum[0]->EvalInstance(fParticleCounter),
            fVarUnit_Momentum * fVarTTF_Momentum[1]->EvalInstance(fParticleCounter),
            fVarUnit_Momentum * fVarTTF_Momentum[2]->EvalInstance(fParticleCounter)
        );
        
        G4double mass = fParticle->GetPDGMass();
        
        G4double energy = fVarUnit_Ekin * fVarTTF_Ekin->EvalInstance(fParticleCounter);
        if (energy < 0) energy = std::sqrt(mass*mass + momentum.mag2()) - mass;
        fParticleGun->SetParticleEnergy(energy);
        
        if (momentum.mag() == 0.) momentum = G4ParticleMomentum(0,0,1);
        momentum = momentum.unit().rotate(fRotationAngles->x(), fRotationAngles->y(), fRotationAngles->z());
        fParticleGun->SetParticleMomentumDirection(momentum);
        
        G4ThreeVector position (
            fVarUnit_Coords * fVarTTF_Coords[0]->EvalInstance(fParticleCounter),
            fVarUnit_Coords * fVarTTF_Coords[1]->EvalInstance(fParticleCounter),
            fVarUnit_Coords * fVarTTF_Coords[2]->EvalInstance(fParticleCounter)
        );
        
        fParticleGun->SetParticlePosition(position);
    
        if (fVarIsSet_Polarization) {
            fParticleGun->SetParticlePolarization (
                G4ThreeVector (
                    fVarTTF_Polarization[0]->EvalInstance(fParticleCounter),
                    fVarTTF_Polarization[1]->EvalInstance(fParticleCounter),
                    fVarTTF_Polarization[2]->EvalInstance(fParticleCounter)
                )
            );
        }
    
        fParticleGun->GeneratePrimaryVertex(event);
        
        BxOutputVertex::Get()->SetEventID(event_id);
        BxOutputVertex::Get()->SetUserInt1(fParticleCounter);
        BxOutputVertex::Get()->SetUserInt2(0);
        BxOutputVertex::Get()->SetUsers();
        if (split) {
            BxOutputVertex::Get()->SetPDG(pdg);
            BxOutputVertex::Get()->SetEnergy(energy/MeV);
            BxOutputVertex::Get()->SetDirection(momentum);
            BxOutputVertex::Get()->SetPosition(position/m);
            BxOutputVertex::Get()->SetTime(0.);
        } else {
            BxOutputVertex::Get()->SetDId(fParticleCounter);
            BxOutputVertex::Get()->SetDPDG(pdg);
            BxOutputVertex::Get()->SetDEnergy(energy/MeV);
            BxOutputVertex::Get()->SetDDirection(momentum);
            BxOutputVertex::Get()->SetDPosition(position/m);
            BxOutputVertex::Get()->SetDTime(0.);
            BxOutputVertex::Get()->SetDaughters();
        }
        BxLog(trace) << "  Entry " << fCurrentEntry
                     << ", event_id = " << BxOutputVertex::Get()->GetEventID()
                     << " : particle #" << fParticleCounter
                     << " : " << fParticle->GetParticleName()
                     << "\t=>" << endlog;
        BxLog(trace) << "    Ekin = " << G4BestUnit(energy, "Energy") << endlog;
        BxLog(trace) << "    direction = " << momentum << endlog;
        BxLog(trace) << "    position = " << G4BestUnit(position, "Length") << endlog;
        
        if (split  &&  pdg == 2112) {
            fPostponed.push_back(std::make_pair(event_id, fParticleCounter));
        }
    }
    
    if (fParticleCounter >= fVarTTF_NParticles->EvalInstance64(0)) {
        ++fCurrentEntry;
        fParticleCounter = 0;
        fRotationAngles->set(0,0,0);
    }
}
