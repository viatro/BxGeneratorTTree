// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "BxGeneratorTTree.hh"

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
#include "G4SPSAngDistribution.hh"

#include "BxOutputVertex.hh"
#include "BxVGenerator.hh"
#include "BxGeneratorTTreeMessenger.hh"
#include "BxLogger.hh"
#include "BxManager.hh"

BxGeneratorTTree::BxGeneratorTTree(): BxVGenerator("BxGeneratorTTree"),
    fTreeChain(0),
    //fTreeNumber(-1),
    fCurrentEntry(0),
    fFirstEntry(0),
    fIsInitialized(false),
    fParticleCounter(0),
    fVarUnit_Ekin(MeV),
    fVarUnit_Momentum(MeV),
    fVarUnit_Coords(m),
    fVarTTF_EventId         (0),
    fVarTTF_EventSkip       (0),
    fVarTTF_ParticleSkip    (0),
    fVarTTF_NParticles      (0),
    fVarTTF_Split           (0),
    fVarTTF_RotateIso       (0),
    fVarTTF_Pdg             (0),
    fVarTTF_Ekin            (0),
    fVarTTF_Momentum[0]     (0),
    fVarTTF_Momentum[1]     (0),
    fVarTTF_Momentum[2]     (0),
    fVarTTF_Coords[0]       (0),
    fVarTTF_Coords[1]       (0),
    fVarTTF_Coords[2]       (0),
    fVarTTF_Polarization[0] (0),
    fVarTTF_Polarization[1] (0),
    fVarTTF_Polarization[2] (0),
    fTTFmanager(0),
    fVarIsSet_EventId(false),
    fVarIsSet_Polarization(false),
    fParticle(),
    fParticleGun(0),
    fParticleTable(0),
    fSPSAng(0),
    fRotation(G4ParticleMomentum(0,0,1)),
    fMessenger(0)
    {

    fTreeChain = new TChain();
    
    fVarTTF_EventId         = new TTreeFormula("tf", "0" , 0);
    fVarTTF_EventSkip       = new TTreeFormula("tf", "1" , 0);
    fVarTTF_ParticleSkip    = new TTreeFormula("tf", "1" , 0);
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

    fParticleGun = new G4ParticleGun();

    fParticleTable = G4ParticleTable::GetParticleTable();
    
    fSPSAng = new G4SPSAngDistribution();
    G4SPSRandomGenerator *biasRndm = new G4SPSRandomGenerator;
    fSPSAng->SetBiasRndm(biasRndm);
    fSPSAng->SetAngDistType("iso");
    
    fMessenger  = new BxGeneratorTTreeMessenger(this);
    
    BxLog(warning) << "\n!!!!!!!!!!    BxGeneratorTTree built    !!!!!!!!!!" << endlog;
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
    
    delete    fSPSAng;
    
    delete fTreeChain;
}

void BxGeneratorTTree::Initialize() {

    BxLog(routine) << "\n!!!!!!!!!!    BxGeneratorTTree initialization started    !!!!!!!!!!" << endlog;

    //fInputFile = TFile::Open(fInputFileName.data(),"read");
    //if (!fInputFile || fInputFile->IsZombie()) {
    //    BxLog(error) << fInputFileName.data() << " ROOT file does not exist or zombie!" << endlog;
    //    BxLog(fatal) << "FATAL " << endlog ;
    //} else {
    //    BxLog(routine) << fInputFileName.data() << " ROOT file opened" << endlog;
    //}
    //fTree = dynamic_cast<TTree*>(fInputFile->Get(fTTreeName.data()));
    //if (!fTree) {
    //    BxLog(error) << fTTreeName.data() << " tree not found by generator initializing function!" << endlog;
    //    BxLog(fatal) << "FATAL " << endlog;
    //}

    fLastEntry = fTreeChain->GetEntries() - 1;
    if (fFirstEntry > fLastEntry) {
        BxLog(error) << "First entry " << fFirstEntry << " > max possible entry " << fLastEntry << " for given Tree" << endlog ;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fTreeChain->LoadTree(0);

    TObjArray* tobjarr = 0;
    G4int ntokens = 0;
    TString tstring = "";

    if ( ! fVarString_EventId.isNull() ) {
        tstring = fVarString_EventId.data();
        fVarTTF_EventId->SetTitle(tstring.Data());
        if ( ! tstring.IsDigit() ) {
            fVarTTF_EventId->SetTree(fTreeChain);
            if (fVarTTF_EventId->GetMultiplicity()) fTTFmanager->Add(fVarTTF_EventId);
        }
        fVarTTF_EventId->Compile();
        fVarIsSet_EventId = true;
    }
    
    if ( ! fVarString_EventSkip.isNull() ) {
        tstring = fVarString_EventSkip.data();
        fVarTTF_EventSkip->SetTitle(tstring.Data());
        fVarTTF_EventSkip->SetTree(fTreeChain);
        if (fVarTTF_EventSkip->GetMultiplicity() != 0) {
            BxLog(error) << "\"Event skip if\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        fTTFmanager->Add(fVarTTF_EventSkip);
        fVarTTF_EventSkip->Compile();
    }
    
    if ( ! fVarString_ParticleSkip.isNull() ) {
        tstring = fVarString_ParticleSkip.data();
        fVarTTF_ParticleSkip->SetTitle(tstring.Data());
        fVarTTF_ParticleSkip->SetTree(fTreeChain);
        if (fVarTTF_ParticleSkip->GetMultiplicity() != 0) {
            BxLog(error) << "\"Particle skip if\" variable has wrong multiplicity!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        fTTFmanager->Add(fVarTTF_ParticleSkip);
        fVarTTF_ParticleSkip->Compile();
    }

    if ( ! fVarString_NParticles.isNull() ) {
        tstring = fVarString_NParticles.data();
        fVarTTF_NParticles->SetTitle(tstring.Data());
        if ( ! tstring.IsDigit() ) {
            fVarTTF_NParticles->SetTree(fTreeChain);
            if ( fVarTTF_NParticles->GetMultiplicity() != 0 ) {
                BxLog(error) << "\"Number of particles\" variable has wrong multiplicity!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
        }
        fVarTTF_NParticles->Compile();
    }
    
    if ( ! fVarString_Split.isNull() ) {
        tstring = fVarString_Split.data();
        fVarTTF_Split->SetTitle(tstring.Data());
        if ( ! tstring.IsDigit() ) {
            fVarTTF_Split->SetTree(fTreeChain);
            if ( fVarTTF_Split->GetMultiplicity() != 0 ) {
                BxLog(error) << "\"Split\" variable has wrong multiplicity!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
        }
        fVarTTF_Split->Compile();
    }
    
    if ( ! fVarString_RotateIso.isNull() ) {
        tstring = fVarString_RotateIso.data();
        fVarTTF_RotateIso->SetTitle(tstring.Data());
        if ( ! tstring.IsDigit() ) {
            fVarTTF_RotateIso->SetTree(fTreeChain);
            if ( fVarTTF_RotateIso->GetMultiplicity() != 0 ) {
                BxLog(error) << "\"RotateIso\" variable has wrong multiplicity!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
        }
        fVarTTF_RotateIso->Compile();
    }

    if ( ! fVarString_Pdg.isNull() ) {
        tstring = fVarString_Pdg.data();
        fVarTTF_Pdg->SetTitle(tstring.Data());
        if (tstring.IsDigit()) {
            fParticle = fParticleTable->FindParticle(tstring.Atoi());
            if (!fParticle)    fParticleTable->GetIonTable()->GetIon(tstring.Atoi());
            if (!fParticle) { // Unknown particle
                BxLog(error) << "Unknown PDG code" << tstring.Atoi() << "!" << endlog;
                BxLog(fatal) << "FATAL " << endlog;
            }
        } else {
            fVarTTF_Pdg->SetTree(fTreeChain);
            if (fVarTTF_Pdg->GetMultiplicity())    fTTFmanager->Add(fVarTTF_Pdg);
        }
        fVarTTF_Pdg->Compile();
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
        fVarTTF_Ekin->SetTitle(tstring.Data());
        if (!tstring.IsFloat()) {
            fVarTTF_Ekin->SetTree(fTreeChain);
            if (fVarTTF_Ekin->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Ekin);
        }
        fVarTTF_Ekin->Compile();
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
            fVarTTF_Momentum[i]->SetTitle(tstring.Data());
            if (!tstring.IsFloat()) {
                fVarTTF_Momentum[i]->SetTree(fTreeChain);
                if (fVarTTF_Momentum[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Momentum[i]);
            }
            fVarTTF_Momentum[i]->Compile();
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
            fVarTTF_Coords[i]->SetTitle(tstring.Data());
            if (!tstring.IsFloat()) {
                fVarTTF_Coords[i]->SetTree(fTreeChain);
                if (fVarTTF_Coords[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Coords[i]);
            }
            fVarTTF_Coords[i]->Compile();
        }
    }

    if ( ! fVarString_Polarization.isNull() ) {
        tstring = fVarString_Polarization.data();
        tobjarr = tstring.Tokenize(" \t\n");
        ntokens = tobjarr->GetEntries();
        if (ntokens != 3) {
            BxLog(error) << "Variable String \"Polarization\" has wrong tokens number!" << endlog;
            BxLog(fatal) << "FATAL " << endlog;
        }
        for (G4int i = 0; i < 3; ++i) {
            tstring = TString(tobjarr->UncheckedAt(i)->GetName());
            fVarTTF_Polarization[i]->SetTitle(tstring.Data());
            if (!tstring.IsFloat()) {
                fVarTTF_Polarization[i]->SetTree(fTreeChain);
                if (fVarTTF_Polarization[i]->GetMultiplicity()) fTTFmanager->Add(fVarTTF_Polarization[i]);
            }
            fVarTTF_Polarization[i]->Compile();
        }
    }

    fTTFmanager->Sync();
    fTreeChain->SetNotify(fTTFmanager);

    BxLog(routine) << "\n!!!!!!!!!!    BxGeneratorTTree initialized    !!!!!!!!!!" << endlog;
}

void BxGeneratorTTree::BxGeneratePrimaries(G4Event *event) {
    if (!fIsInitialized) {
        Initialize();
        fIsInitialized = true;
    }

    if (fCurrentEntry > fLastEntry) {
        BxManager::Get()->AbortRun(true);
        event->SetEventAborted();
        BxLog(routine) << "End of Tree(Chain) reached" << endlog;
        return;
    }

    fTreeChain->LoadTree(fCurrentEntry);

    //NOTE: without fTreeChain->SetNotify(fTTFmanager) in BxGeneratorTTree::Initialize()
    //if (fTreeNumber != fTreeChain->GetTreeNumber()) {
    //    fTreeNumber = fTreeChain->GetTreeNumber();
    //    fTTFmanager->UpdateFormulaLeaves();
    //}
    
    if (fTTFmanager->GetNdata() <= 0)  continue;
    
    if ( fVarTTF_EventSkip->EvalInstance64(0) )  continue;
    
    event->SetEventID(fVarIsSet_EventId ? fVarTTF_EventId->EvalInstance64(0) : fCurrentEntry);
    
    if ( fVarTTF_RotateIso->EvalInstance64(0) && fRotation == G4ParticleMomentum(0,0,1) )   fRotation = fSPSAng->GenerateOne();
    
    for ( fParticleCounter = fVarTTF_Split->EvalInstance64(0) ? fParticleCounter : 0;
          fParticleCounter < fVarTTF_Split->EvalInstance64(0) ? fParticleCounter + 1 : fVarTTF_NParticles->EvalInstance64(0);
          ++fParticleCounter ) {
        
        if ( fVarTTF_ParticleSkip->EvalInstance64(0) )  continue;
        
        fParticle = fParticleTable->FindParticle( fVarTTF_Pdg->EvalInstance64(fParticleCounter) );
        if (!fParticle) {
            fParticle = fParticleTable->GetIonTable()->GetIon( fVarTTF_Pdg->EvalInstance64(fParticleCounter) );
            if (!fParticle) { // Unknown particle --> geantino
                fParticle = fParticleTable->FindParticle("geantino");
                BxLog(warning) << "WARNING! " << "Entry " << fCurrentEntry << " : replace particle with PDG " << fVarTTF_Pdg->EvalInstance64(fParticleCounter) << " to geantino" << endlog;
            }
        }
    
        fParticleGun->SetParticleDefinition(fParticle);
    
        fParticleGun->SetParticlePosition (
            G4ThreeVector (
                fVarUnit_Coords * fVarTTF_Coords[0]->EvalInstance(fParticleCounter),
                fVarUnit_Coords * fVarTTF_Coords[1]->EvalInstance(fParticleCounter),
                fVarUnit_Coords * fVarTTF_Coords[2]->EvalInstance(fParticleCounter)
            )
        );
    
        if (fVarTTF_Ekin->EvalInstance(fParticleCounter) >= 0) {
            fParticleGun->SetParticleEnergy( fVarUnit_Ekin * fVarTTF_Ekin->EvalInstance(fParticleCounter) );
            fParticleGun->SetParticleMomentumDirection (
                G4ParticleMomentum (
                    fVarTTF_Momentum[0]->EvalInstance(fParticleCounter),
                    fVarTTF_Momentum[1]->EvalInstance(fParticleCounter),
                    fVarTTF_Momentum[2]->EvalInstance(fParticleCounter)
                ).rotateUz(fRotation)
            );
        } else {
            fParticleGun->SetParticleMomentum (
                G4ParticleMomentum (
                    fVarUnit_Momentum * fVarTTF_Momentum[0]->EvalInstance(fParticleCounter),
                    fVarUnit_Momentum * fVarTTF_Momentum[1]->EvalInstance(fParticleCounter),
                    fVarUnit_Momentum * fVarTTF_Momentum[2]->EvalInstance(fParticleCounter)
                ).rotateUz(fRotation)
            );
        }
    
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
    
        //TODO: make this to be working properly
        BxOutputVertex::Get()->SetEventID(event->GetEventID());
        BxOutputVertex::Get()->SetPDG(fParticleGun->GetParticleDefinition()->GetPDGEncoding());
        BxOutputVertex::Get()->SetEnergy(fParticleGun->GetParticleEnergy()/MeV);
        BxOutputVertex::Get()->SetDirection(fParticleGun->GetParticleMomentumDirection());
        BxOutputVertex::Get()->SetPosition(fParticleGun->GetParticlePosition());
        BxOutputVertex::Get()->SetTime(0.);
    
        BxLog(trace) << "Entry " << fCurrentEntry
                    << ", event_id = " << event->GetEventID()
                    << " :  " << fParticle->GetParticleName()
                    << "\t->"
                    << ( (fParticleGun->GetParticleEnergy() >= 0) ? "\tEkin = " : "" )
                    << ( (fParticleGun->GetParticleEnergy() >= 0) ? G4String(G4BestUnit(fParticleGun->GetParticleEnergy(), "Energy")) : "" )
                    << ( (fParticleGun->GetParticleEnergy() >= 0) ? ", direction = " : "\tmomentum = " )
                    << ( (fParticleGun->GetParticleEnergy() >= 0) ? G4BestUnit(fParticleGun->GetParticleMomentumDirection(), "Energy") : G4BestUnit(fParticleGun->GetParticleMomentum(), "Energy") )
                    << "\tat position " << G4BestUnit(fParticleGun->GetParticlePosition(), "Length") << endlog;
    }

    //++fParticleCounter;
    if (fParticleCounter >= fVarTTF_NParticles->EvalInstance64(0)) {
        ++fCurrentEntry;
        fParticleCounter = 0;
        fRotation = G4ParticleMomentum(0,0,1);
    }
}
