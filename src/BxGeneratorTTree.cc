// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "TString.h"
#include "TObjArray.h"
#include "TTreeFormula.h"
#include "TTreeFormulaManager.h"

#include "BxGeneratorTTree.hh"
#include "BxOutputVertex.hh"
#include "BxVGenerator.hh"
#include "BxGeneratorTTreeMessenger.hh"
#include "BxLogger.hh"
#include "BxManager.hh"
#include "BxReadParameters.hh"

#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4ParticleGun.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleMomentum.hh"
#include "G4PrimaryParticle.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"

#include <algorithm>

BxGeneratorTTree::BxGeneratorTTree() : BxVGenerator("BxGeneratorTTree")
    //, fTreeNumber(-1),
    , fCurrentEntry(-1)
    , fFirstEntry(0)
    , fLastEntry(0)
    , fNEntries(0)
    , fIsInitialized(false)
    , fParticleCounter(0)
    , fVarUnit_Ekin(MeV)
    , fVarUnit_Momentum(MeV)
    , fVarUnit_Position(m)
    , fVarUnit_Time(ns)
    , fVarIsSet_EventId(false)
    , fCurrentSplitMode(false)
    , fCurrentParticleInfo()
    , fDequeParticleInfo()
    , fPrimaryIndexes()
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
    fVarTTF_Position[0]     = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Position[1]     = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Position[2]     = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Time            = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Polarization[0] = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Polarization[1] = new TTreeFormula("tf", "0" , 0);
    fVarTTF_Polarization[2] = new TTreeFormula("tf", "0" , 0);
    
    fTTFmanager = new TTreeFormulaManager();
    
    fParticleGun = new G4ParticleGun();
    
    BxReadParameters::Get()->SetRDMDecay(true);
    BxReadParameters::Get()->SetRDMChain(true);
    
    fMessenger = new BxGeneratorTTreeMessenger(this);
    
    BxLog(routine) << "BxGeneratorTTree built" << endlog;
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
    delete    fVarTTF_Time         ;
    for (G4int i = 0; i < 3; ++i) {
        delete    fVarTTF_Momentum     [i];
        delete    fVarTTF_Position     [i];
        delete    fVarTTF_Polarization [i];
    }
    
    delete fTreeChain;
}

void BxGeneratorTTree::Initialize() {
    
    BxLog(routine) << "BxGeneratorTTree initialization started" << endlog;
    
    G4int loadTree0 = fTreeChain->LoadTree(0);
    if (loadTree0 == -1) {
        BxLog(warning) << "Tree(Chain) is empty! Exact numeric values must be used for variables" << endlog;
    }
    
    fLastEntry = fTreeChain->GetEntries() - 1;
    if (fFirstEntry > fLastEntry && loadTree0 != -1) {
        BxLog(error) << "First entry " << fFirstEntry << " > max possible entry " << fLastEntry << " for given Tree(Chain)" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fCurrentEntry += fFirstEntry;
    
    TObjArray* tobjarr = 0;
    G4int ntokens = 0;
    TString tstring = "";
    
    if (!fVarString_EventId.isNull()) {
        tstring = fVarString_EventId.data();
        if (!tstring.IsFloat()) {
            delete fVarTTF_EventId;
            fVarTTF_EventId = new TTreeFormula("tf", tstring.Data(), fTreeChain);
            if (fVarTTF_EventId->GetMultiplicity() != 0) {
                BxLog(error) << "\"Event ID\" variable has wrong multiplicity!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarTTF_EventId->SetQuickLoad(true);
            fVarIsSet_EventId = true;
        }
    }
    
    if (!fVarString_EventSkip.isNull()) {
        tstring = fVarString_EventSkip.data();
        delete fVarTTF_EventSkip;
        fVarTTF_EventSkip = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        /*if (fVarTTF_EventSkip->GetMultiplicity() != 0) {
            BxLog(error) << "\"Event skip if\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }*/
        fVarTTF_EventSkip->SetQuickLoad(true);
    }
    
    if (!fVarString_ParticleSkip.isNull()) {
        tstring = fVarString_ParticleSkip.data();
        delete fVarTTF_ParticleSkip;
        fVarTTF_ParticleSkip = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        /*if (fVarTTF_ParticleSkip->GetMultiplicity() != 0) {
            BxLog(error) << "\"Particle skip if\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }*/
        if (fVarTTF_ParticleSkip->GetMultiplicity()) fTTFmanager->Add(fVarTTF_ParticleSkip);
        fVarTTF_ParticleSkip->SetQuickLoad(true);
    }
    
    if (!fVarString_NParticles.isNull()) {
        tstring = fVarString_NParticles.data();
        delete fVarTTF_NParticles;
        fVarTTF_NParticles = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_NParticles->GetMultiplicity() != 0) {
            BxLog(error) << "\"Number of particles\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        fVarTTF_NParticles->SetQuickLoad(true);
    }
    
    if (!fVarString_Split.isNull()) {
        tstring = fVarString_Split.data();
        delete fVarTTF_Split;
        fVarTTF_Split = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_Split->GetMultiplicity() != 0) {
            BxLog(error) << "\"Split\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        fVarTTF_Split->SetQuickLoad(true);
    }
    
    if (!fVarString_RotateIso.isNull()) {
        tstring = fVarString_RotateIso.data();
        delete fVarTTF_RotateIso;
        fVarTTF_RotateIso = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_RotateIso->GetMultiplicity() != 0) {
            BxLog(error) << "\"RotateIso\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        fVarTTF_RotateIso->SetQuickLoad(true);
    }
    
    if (!fVarString_Pdg.isNull()) {
        tstring = fVarString_Pdg.data();
        if (tstring.IsDigit()) {
            G4ParticleDefinition* fParticle = G4ParticleTable::GetParticleTable()->FindParticle(tstring.Atoi());
            if (!fParticle)    fParticle = G4IonTable::GetIonTable()->GetIon(tstring.Atoi());
            if (!fParticle) { // Unknown particle
                BxLog(error) << "Unknown PDG code : " << tstring.Atoi() << " !" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
        }
        delete fVarTTF_Pdg;
        fVarTTF_Pdg = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_Pdg->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Pdg);
        fVarTTF_Pdg->SetQuickLoad(true);
    }
    
    if (!fVarString_Ekin.isNull()) {
        tstring = fVarString_Ekin.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 1 || ntokens > 2) {
            BxLog(error) << "Variable String \"Ekin\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 2) {
            if (G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(1)->GetName()) != "Energy") {
                BxLog(error) << "Variable String \"Ekin\" has wrong unit category or unit name!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarUnit_Ekin = G4UnitDefinition::GetValueOf(tobjarr->UncheckedAt(1)->GetName());
        }
        tstring = TString(tobjarr->UncheckedAt(0)->GetName());
        delete fVarTTF_Ekin;
        fVarTTF_Ekin = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_Ekin->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Ekin);
        fVarTTF_Ekin->SetQuickLoad(true);
    }
    
    if (!fVarString_Momentum.isNull()) {
        tstring = fVarString_Momentum.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 3 || ntokens > 4) {
            BxLog(error) << "Variable string \"Momentum\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 4) {
            if (G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(3)->GetName()) != "Energy") {
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
            fVarTTF_Momentum[i]->SetQuickLoad(true);
        }
    }
    
    if (!fVarString_Position.isNull()) {
        tstring = fVarString_Position.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 3 || ntokens > 4) {
            BxLog(error) << "Variable string \"Position\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 4) {
            if (G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(3)->GetName()) != "Length") {
                BxLog(error) << "Variable string \"Position\" has wrong unit category or unit name!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarUnit_Position = G4UnitDefinition::GetValueOf(tobjarr->UncheckedAt(3)->GetName());
        }
        for (G4int i = 0; i < 3; ++i) {
            tstring = TString(tobjarr->UncheckedAt(i)->GetName());
            delete fVarTTF_Position[i];
            fVarTTF_Position[i] = new TTreeFormula("tf", tstring.Data(), fTreeChain);
            if (fVarTTF_Position[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Position[i]);
            fVarTTF_Position[i]->SetQuickLoad(true);
        }
    }
    
    if (!fVarString_Time.isNull()) {
        tstring = fVarString_Time.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens < 1 || ntokens > 2) {
            BxLog(error) << "Variable String \"Time\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        if (ntokens == 2) {
            if (G4UnitDefinition::GetCategory(tobjarr->UncheckedAt(1)->GetName()) != "Time") {
                BxLog(error) << "Variable String \"Time\" has wrong unit category or unit name!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
            fVarUnit_Time = G4UnitDefinition::GetValueOf(tobjarr->UncheckedAt(1)->GetName());
        }
        tstring = TString(tobjarr->UncheckedAt(0)->GetName());
        delete fVarTTF_Time;
        fVarTTF_Time = new TTreeFormula("tf", tstring.Data(), fTreeChain);
        if (fVarTTF_Time->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Time);
        fVarTTF_Time->SetQuickLoad(true);
    }
    
    if (!fVarString_Polarization.isNull()) {
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
            fVarTTF_Polarization[i]->SetQuickLoad(true);
        }
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
        << "\n\tPosition[0]     : " << fVarTTF_Position[0]     -> GetTitle()
        << "\n\tPosition[1]     : " << fVarTTF_Position[1]     -> GetTitle()
        << "\n\tPosition[2]     : " << fVarTTF_Position[2]     -> GetTitle()
        << "\n\tTime            : " << fVarTTF_Time            -> GetTitle()
        << "\n\tPolarization[0] : " << fVarTTF_Polarization[0] -> GetTitle()
        << "\n\tPolarization[1] : " << fVarTTF_Polarization[1] -> GetTitle()
        << "\n\tPolarization[2] : " << fVarTTF_Polarization[2] -> GetTitle()
        << endlog;
    
    fIsInitialized = true;
    
    BxLog(routine) << "BxGeneratorTTree initialized" << endlog;
}

void BxGeneratorTTree::FillDequeFromEntry(G4int entry_number) {
    fTreeChain->LoadTree(entry_number);
    fTTFmanager->GetNdata();
    
    ParticleInfo particle_info;
    particle_info.event_id = fVarIsSet_EventId ? fVarTTF_EventId->EvalInstance64(0) : entry_number;
    
    G4ThreeVector rotationAngles(0.,0.,0.);
    if (fVarTTF_RotateIso->EvalInstance64(0))  rotationAngles.set(twopi*G4UniformRand(), std::acos(2.*G4UniformRand() - 1.), twopi*G4UniformRand());
    
    for (G4int i = 0; i < fVarTTF_NParticles->EvalInstance64(0); ++i) {
        if (fVarTTF_ParticleSkip->EvalInstance64(i))  continue;
        particle_info.p_index = i;
        particle_info.status = 0;
        
        particle_info.pdg_code = fVarTTF_Pdg->EvalInstance64(i);
        G4ParticleDefinition* fParticle = G4ParticleTable::GetParticleTable()->FindParticle(particle_info.pdg_code);
        if (!fParticle) {
            fParticle = G4IonTable::GetIonTable()->GetIon(particle_info.pdg_code);
            if (!fParticle) { // Skip unknown particle
                BxLog(warning) << "  Entry " << entry_number
                            << ", event_id = " << particle_info.event_id
                            << " : particle #" << i
                            << " : WARNING!" << endlog;
                BxLog(warning) << "    Skipping unknown particle with PDG code " << particle_info.pdg_code << endlog;
                continue;
            }
        }
        
        particle_info.momentum.set(
            fVarUnit_Momentum * fVarTTF_Momentum[0]->EvalInstance(i),
            fVarUnit_Momentum * fVarTTF_Momentum[1]->EvalInstance(i),
            fVarUnit_Momentum * fVarTTF_Momentum[2]->EvalInstance(i)
        );
        
        particle_info.energy = fVarUnit_Ekin * fVarTTF_Ekin->EvalInstance(i);
        G4double mass = fParticle->GetPDGMass();
        if (particle_info.energy < 0.) particle_info.energy = std::sqrt(mass*mass + particle_info.momentum.mag2()) - mass;
        
        if (particle_info.momentum.mag() == 0.) particle_info.momentum.set(0.,0.,1.);
        particle_info.momentum = particle_info.momentum.unit().rotate(rotationAngles.x(), rotationAngles.y(), rotationAngles.z());
        
        particle_info.position.set(
            fVarUnit_Position * fVarTTF_Position[0]->EvalInstance(i),
            fVarUnit_Position * fVarTTF_Position[1]->EvalInstance(i),
            fVarUnit_Position * fVarTTF_Position[2]->EvalInstance(i)
        );
        
        particle_info.time = fVarUnit_Time * fVarTTF_Time->EvalInstance(i);
        
        particle_info.polarization.set(
            fVarTTF_Polarization[0]->EvalInstance(i),
            fVarTTF_Polarization[1]->EvalInstance(i),
            fVarTTF_Polarization[2]->EvalInstance(i)
        );
        
        fDequeParticleInfo.push_back(particle_info);
    }
}

void BxGeneratorTTree::BxGeneratePrimaries(G4Event* event) {
    if (!fIsInitialized)  Initialize();
    
    if (fDequeParticleInfo.empty()) {
        do {
            ++fCurrentEntry; //after initialization fCurrentEntry == fFirstEntry - 1
            if ( (fCurrentEntry > fFirstEntry + fNEntries - 1) || (fCurrentEntry > fLastEntry && fLastEntry != -1)) {
                // RunManager cannot abort the event from inside UserGeneratePrimaries(), so we do a soft abort
                // to the RunManager, and abort the event ourselves. The result is the same as a hard abort.
                BxManager::Get()->AbortRun(true);
                event->SetEventAborted();
                if (fCurrentEntry > fLastEntry && fLastEntry != -1) BxLog(routine) << "End of Tree(Chain) reached" << endlog;
                return;
            }
            G4int loadedEntry = -100;
            loadedEntry = fTreeChain->LoadTree(fCurrentEntry);
            if (loadedEntry < -1) {
                //if loadedEntry == -1 (i.e. chain is empty) and one (or more) of TTreeFormula-s is not a float number,
                //it already failed in Initialize() with "Bad numerical expression".
                //So it is safe here to use fTreeChain->LoadTree(fCurrentEntry) == -1 as good case
                //to provide possibility for using this generator without TTree.
                
                //error descriptions from TChain::LoadTree()
                     if (loadedEntry == -2) BxLog(fatal/*error*/) << "The requested entry number is less than zero or too large for the chain or too large for the large TTree" << endlog;
                else if (loadedEntry == -3) BxLog(fatal/*error*/) << "The file corresponding to the entry could not be correctly open" << endlog;
                else if (loadedEntry == -4) BxLog(fatal/*error*/) << "The TChainElement corresponding to the entry is missing or the TTree is missing from the file"  << endlog;
            }
        } while (fTTFmanager->GetNdata() <= 0 || fVarTTF_EventSkip->EvalInstance64(0));
        
        FillDequeFromEntry(fCurrentEntry);
    }
    
    fPrimaryIndexes.clear();
    
    fCurrentSplitMode = fVarTTF_Split->EvalInstance64(0);
    if (!fCurrentSplitMode) {
        std::sort(fDequeParticleInfo.begin(), fDequeParticleInfo.end(), particle_info_compare_by_time);
    }
    
    do {
        fCurrentParticleInfo = fDequeParticleInfo.front();
        fDequeParticleInfo.pop_front();
        
        G4ParticleDefinition* fParticle = G4ParticleTable::GetParticleTable()->FindParticle(fCurrentParticleInfo.pdg_code);
        if (!fParticle) {
            fParticle = G4IonTable::GetIonTable()->GetIon(fCurrentParticleInfo.pdg_code);
            if (!fParticle) { // Skip unknown particle
                BxLog(warning) << "  Entry " << fCurrentEntry
                            << ", event_id = " << fCurrentParticleInfo.event_id
                            << " : particle #" << fParticleCounter
                            << " : WARNING!" << endlog;
                BxLog(warning) << "  Skipping unknown particle with PDG code " << fCurrentParticleInfo.pdg_code << endlog;
                if (fCurrentSplitMode)  event->SetEventAborted();
                continue;
            }
        }
        
        fParticleGun->SetParticleDefinition(fParticle);
        //g4bx2 behaves wrong with particles with zero kinetic energy
        fParticleGun->SetParticleEnergy(fCurrentParticleInfo.energy ? fCurrentParticleInfo.energy : 1e-100*eV);
        fParticleGun->SetParticleMomentumDirection(fCurrentParticleInfo.momentum);
        fParticleGun->SetParticlePosition(fCurrentParticleInfo.position);
        fParticleGun->SetParticleTime(fCurrentParticleInfo.status == 0 ? fCurrentParticleInfo.time : 0);
        fParticleGun->SetParticlePolarization(fCurrentParticleInfo.polarization);
        
        fParticleGun->GeneratePrimaryVertex(event);
        
        fPrimaryIndexes.push_back(fCurrentParticleInfo.p_index);
        
        BxOutputVertex::Get()->SetEventID(fCurrentParticleInfo.event_id);
        BxOutputVertex::Get()->SetUserInt1(fCurrentParticleInfo.p_index);
        BxOutputVertex::Get()->SetUserInt2(fCurrentParticleInfo.status);
        BxOutputVertex::Get()->SetUsers();
        if (fCurrentSplitMode) {
            BxOutputVertex::Get()->SetPDG(fCurrentParticleInfo.pdg_code);
            BxOutputVertex::Get()->SetEnergy(fCurrentParticleInfo.energy/MeV);
            BxOutputVertex::Get()->SetDirection(fCurrentParticleInfo.momentum);
            BxOutputVertex::Get()->SetPosition(fCurrentParticleInfo.position/m);
            BxOutputVertex::Get()->SetTime(fCurrentParticleInfo.time/ns);
        } else {
            BxOutputVertex::Get()->SetDId(fCurrentParticleInfo.p_index);
            BxOutputVertex::Get()->SetDPDG(fCurrentParticleInfo.pdg_code);
            BxOutputVertex::Get()->SetDEnergy(fCurrentParticleInfo.energy/MeV);
            BxOutputVertex::Get()->SetDDirection(fCurrentParticleInfo.momentum);
            BxOutputVertex::Get()->SetDPosition(fCurrentParticleInfo.position/m);
            BxOutputVertex::Get()->SetDTime(fCurrentParticleInfo.time/ns);
            BxOutputVertex::Get()->SetDaughters();
        }
        BxLog(trace) << "  Entry " << fCurrentEntry
                    << ", event_id = " << fCurrentParticleInfo.event_id
                    << " : particle #" << fCurrentParticleInfo.p_index
                    << (fCurrentParticleInfo.status != 0 ? TString::Format(", POSTPONED'%d", fCurrentParticleInfo.status) : "")
                    << " : " << fParticle->GetParticleName()
                    << "\t=>" << endlog;
        BxLog(trace) << "    Ekin = " << G4BestUnit(fCurrentParticleInfo.energy, "Energy") << endlog;
        BxLog(trace) << "    direction = " << fCurrentParticleInfo.momentum << endlog;
        BxLog(trace) << "    position = " << G4BestUnit(fCurrentParticleInfo.position, "Length") << endlog;
        BxLog(trace) << "    time = " << G4BestUnit(fCurrentParticleInfo.time, "Time") << endlog;
    } while (!fCurrentSplitMode && !fDequeParticleInfo.empty() && fDequeParticleInfo.front().time == fCurrentParticleInfo.time);
}
