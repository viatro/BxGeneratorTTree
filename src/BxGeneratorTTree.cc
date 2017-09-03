// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

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
#include <sstream>

BxGeneratorTTree::BxGeneratorTTree()
: BxVGenerator("BxGeneratorTTree")
, fCurrentEntry(-1)
, fFirstEntry(0)
, fLastEntry(0)
, fNEntries(0)
, fIsInitialized(false)
, fLogPrimariesInfo(true)
, fSavePrimariesInfo(true)
, fDequeParticleInfo()
, fCurrentParticlesInfo()
{
    fTreeChain = new TChain();
    
    fEventConfigTTF = new EventConfigTTF(fTreeChain);
    
    fParticleGun = new G4ParticleGun();
    
    BxReadParameters::Get()->SetRDMDecay(true);
    BxReadParameters::Get()->SetRDMChain(true);
    
    fMessenger = new BxGeneratorTTreeMessenger(this);
    
    BxLog(routine) << "BxGeneratorTTree built" << endlog;
}

BxGeneratorTTree::~BxGeneratorTTree() {
    delete fMessenger;
    delete fParticleGun;
    delete fEventConfigTTF;
    delete fTreeChain;
}

void BxGeneratorTTree::AddTree(const G4String& treename, const G4String& filename) {
    fTreeChain->Add( (filename + "/" + treename).data() );
}

void BxGeneratorTTree::SetAlias(const G4String& alias, const G4String& expression) {
    fTreeChain->SetAlias(alias.data(), expression.data());
}

void BxGeneratorTTree::Initialize() {
    BxLog(routine) << "BxGeneratorTTree initialization started" << endlog;
    
    G4int loadTree0 = fTreeChain->LoadTree(0);
    if (loadTree0 == -1) {
        BxLog(warning) << "Tree(Chain) is empty! Be sure that exact numeric values are used for variables or it'll be crash" << endlog;
    }
    
    fLastEntry = fTreeChain->GetEntries() - 1;
    if (fFirstEntry > fLastEntry && loadTree0 != -1) {
        BxLog(error) << "First entry " << fFirstEntry << " > max possible entry " << fLastEntry << " for given Tree(Chain)" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fCurrentEntry += fFirstEntry;
    if (fNEntries <= 0) fNEntries = fLastEntry - fFirstEntry + 1;
    
    fEventConfigTTF->Initialize();
    fEventConfigTTF->Log();
    
    fIsInitialized = true;
    
    BxLog(routine) << "BxGeneratorTTree initialized" << endlog;
}

G4bool BxGeneratorTTree::FillDequeFromEntry(G4int entry_number) {
    fEventConfigTTF->CheckInOnEntry(entry_number);
    
    if (fEventConfigTTF->EvalEventSkip()) return false;
    
    ParticleInfo particle_info;
    particle_info.event_id = fEventConfigTTF->IsSetEventId() ? fEventConfigTTF->EvalEventId() : entry_number;
    
    G4ThreeVector rotationAnglesEvent(0.,0.,0.);
    if (fEventConfigTTF->EvalEventRotateIso())  rotationAnglesEvent.set(twopi*G4UniformRand(), std::acos(2.*G4UniformRand() - 1.), twopi*G4UniformRand());
    
    G4int total_p_index = 0;
    for (size_t k = 0; k < fEventConfigTTF->GetSubEvents().size(); ++k) {
        SubEventConfigTTF& subEventConfigTTF = fEventConfigTTF->GetSubEvent(k);
        if (subEventConfigTTF.GetManager()->GetNdata() <= 0) {
            fDequeParticleInfo.clear();
            return false;
        }
        
        G4ThreeVector rotationAnglesSubEvent(0.,0.,0.);
        if (subEventConfigTTF.EvalSubEventRotateIso())  rotationAnglesSubEvent.set(twopi*G4UniformRand(), std::acos(2.*G4UniformRand() - 1.), twopi*G4UniformRand());
        
        for (G4int i = 0; i < subEventConfigTTF.EvalNParticles(); ++i) {
            if (subEventConfigTTF.EvalParticleSkip(i))  continue;
            particle_info.p_index = total_p_index;
            ++total_p_index;
            particle_info.status = 0;
            
            particle_info.pdg_code = subEventConfigTTF.EvalPdg(i);
            G4ParticleDefinition* fParticle = G4ParticleTable::GetParticleTable()->FindParticle(particle_info.pdg_code);
            if (!fParticle) {
                fParticle = G4IonTable::GetIonTable()->GetIon(particle_info.pdg_code);
                if (!fParticle) { // Skip unknown particle
                    BxLog(warning) << "  Entry " << entry_number
                                << ", event_id = " << particle_info.event_id
                                << " : sub_event #" << k
                                << ", particle #" << i
                                << " : WARNING!" << endlog;
                    BxLog(warning) << "    Skipping unknown particle with PDG code " << particle_info.pdg_code << endlog;
                    continue;
                }
            }
            
            particle_info.momentum.set(
                subEventConfigTTF.GetMomentumUnit() * subEventConfigTTF.EvalMomentumX(i),
                subEventConfigTTF.GetMomentumUnit() * subEventConfigTTF.EvalMomentumY(i),
                subEventConfigTTF.GetMomentumUnit() * subEventConfigTTF.EvalMomentumZ(i)
            );
            
            particle_info.energy = subEventConfigTTF.GetEnergyUnit() * subEventConfigTTF.EvalEnergy(i);
            G4double mass = fParticle->GetPDGMass();
            if (particle_info.energy < 0.) particle_info.energy = std::sqrt(mass*mass + particle_info.momentum.mag2()) - mass;
            
            if (particle_info.momentum.mag() == 0.) particle_info.momentum.set(0.,0.,1.);
            particle_info.momentum = particle_info.momentum
                                                  .unit()
                                                  .rotate(rotationAnglesEvent.x(), rotationAnglesEvent.y(), rotationAnglesEvent.z())
                                                  .rotate(rotationAnglesSubEvent.x(), rotationAnglesSubEvent.y(), rotationAnglesSubEvent.z());
            if (subEventConfigTTF.EvalParticleRotateIso(i)) {
                particle_info.momentum = particle_info.momentum.rotate(twopi*G4UniformRand(), std::acos(2.*G4UniformRand() - 1.), twopi*G4UniformRand());
            }
            
            particle_info.position.set(
                subEventConfigTTF.GetPositionUnit() * subEventConfigTTF.EvalPositionX(i),
                subEventConfigTTF.GetPositionUnit() * subEventConfigTTF.EvalPositionY(i),
                subEventConfigTTF.GetPositionUnit() * subEventConfigTTF.EvalPositionZ(i)
            );
            
            particle_info.time = subEventConfigTTF.GetTimeUnit() * subEventConfigTTF.EvalTime(i);
            
            particle_info.polarization.set(
                subEventConfigTTF.EvalPolarizationX(i),
                subEventConfigTTF.EvalPolarizationY(i),
                subEventConfigTTF.EvalPolarizationZ(i)
            );
            
            fDequeParticleInfo.push_back(particle_info);
        }
    }
    return true;
}

void BxGeneratorTTree::BxGeneratePrimaries(G4Event* event) {
    if (!fIsInitialized)  Initialize();
    
    while (fDequeParticleInfo.empty()) {
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
            G4int loadedEntry = fTreeChain->LoadTree(fCurrentEntry);
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
        } while (! FillDequeFromEntry(fCurrentEntry));
    }
    
    fCurrentParticlesInfo.clear();
    
    std::sort(fDequeParticleInfo.begin(), fDequeParticleInfo.end(), particle_info_compare_by_time);
    
    do {
        fCurrentParticlesInfo.push_back(fDequeParticleInfo.front());
        fDequeParticleInfo.pop_front();
        
        const ParticleInfo& particle_info = fCurrentParticlesInfo.back();
        
        G4ParticleDefinition* fParticle = G4ParticleTable::GetParticleTable()->FindParticle(particle_info.pdg_code);
        if (!fParticle) {
            fParticle = G4IonTable::GetIonTable()->GetIon(particle_info.pdg_code);
            if (!fParticle) { // Skip unknown particle
                BxLog(warning)
                    << "  Entry " << fCurrentEntry
                    << ", event_id = " << particle_info.event_id
                    << " : particle #" << particle_info.p_index
                    << " : WARNING!" << endlog;
                BxLog(warning) << "  Skipping unknown particle with PDG code " << particle_info.pdg_code << endlog;
                continue;
            }
        }
        
        fParticleGun->SetParticleDefinition(fParticle);
        //g4bx2 behaves wrong with particles with zero kinetic energy
        fParticleGun->SetParticleEnergy(particle_info.energy ? particle_info.energy : 1e-100*eV);
        fParticleGun->SetParticleMomentumDirection(particle_info.momentum);
        fParticleGun->SetParticlePosition(particle_info.position);
        fParticleGun->SetParticleTime(particle_info.status == 0 ? particle_info.time : 0);
        fParticleGun->SetParticlePolarization(particle_info.polarization);
        
        fParticleGun->GeneratePrimaryVertex(event);
        
        BxOutputVertex::Get()->SetEventID(particle_info.event_id);
        if (fSavePrimariesInfo) {
            BxOutputVertex::Get()->SetDId(particle_info.p_index);
            BxOutputVertex::Get()->SetDPDG(particle_info.pdg_code);
            BxOutputVertex::Get()->SetDEnergy(particle_info.energy/MeV);
            BxOutputVertex::Get()->SetDDirection(particle_info.momentum);
            BxOutputVertex::Get()->SetDPosition(particle_info.position/m);
            BxOutputVertex::Get()->SetDTime(particle_info.time/ns);
            BxOutputVertex::Get()->SetDaughters();
            BxOutputVertex::Get()->SetUserInt1(particle_info.p_index);
            BxOutputVertex::Get()->SetUserInt2(particle_info.status);
            BxOutputVertex::Get()->SetUsers();
        }
        
        if (fLogPrimariesInfo) {
            BxLog(trace) << "  Entry " << fCurrentEntry
                         << ", event_id = " << particle_info.event_id
                         << " : particle #" << particle_info.p_index
                         << (particle_info.status != 0 ? TString::Format(", POSTPONED'%d", particle_info.status) : "")
                         << " : " << fParticle->GetParticleName()
                         << "\t=>" << endlog;
            BxLog(trace) << "    Energy = " << G4BestUnit(particle_info.energy, "Energy") << endlog;
            BxLog(trace) << "    direction = " << particle_info.momentum << endlog;
            BxLog(trace) << "    position = " << G4BestUnit(particle_info.position, "Length") << endlog;
            BxLog(trace) << "    time = " << G4BestUnit(particle_info.time, "Time") << endlog;
        }
    } while (!fDequeParticleInfo.empty() && (fCurrentParticlesInfo.back().status == 0 || fDequeParticleInfo.front().time == fCurrentParticlesInfo.back().time));
}


void BxGeneratorTTree::PushFrontParticleInfo(G4int event_id, G4int p_index, G4int status,
    G4int pdg_code, G4double energy, const G4ThreeVector& momentum,
    const G4ThreeVector& position, G4double time, const G4ThreeVector& polarization) {
        ParticleInfo particle_info;
        particle_info.event_id = event_id;
        particle_info.p_index = p_index;
        particle_info.status = status;
        particle_info.pdg_code = pdg_code;
        particle_info.energy = energy;
        particle_info.momentum = momentum;
        particle_info.position = position;
        particle_info.time = time;
        particle_info.polarization = polarization;
        fDequeParticleInfo.push_front(particle_info);
}

void BxGeneratorTTree::PushBackParticleInfo(G4int event_id, G4int p_index, G4int status,
    G4int pdg_code, G4double energy, const G4ThreeVector& momentum,
    const G4ThreeVector& position, G4double time, const G4ThreeVector& polarization) {
        ParticleInfo particle_info;
        particle_info.event_id = event_id;
        particle_info.p_index = p_index;
        particle_info.status = status;
        particle_info.pdg_code = pdg_code;
        particle_info.energy = energy;
        particle_info.momentum = momentum;
        particle_info.position = position;
        particle_info.time = time;
        particle_info.polarization = polarization;
        fDequeParticleInfo.push_back(particle_info);
}


BxGeneratorTTree::SubEventConfigTTF::SubEventConfigTTF(TChain* pTreeChain)
: fTreeChain(pTreeChain)
, fUnitEnergy(MeV)
, fUnitMomentum(MeV)
, fUnitPosition(m)
, fUnitTime(ns)
, fStringSubEventRotateIso( "0")
, fStringNParticles       ( "1")
, fStringParticleSkip     ( "0")
, fStringParticleRotateIso( "0")
, fStringPdg              ("22")
, fStringEnergy           ( "1")
, fStringMomentumX        ( "0")
, fStringMomentumY        ( "0")
, fStringMomentumZ        ( "1")
, fStringPositionX        ( "0")
, fStringPositionY        ( "0")
, fStringPositionZ        ( "0")
, fStringTime             ( "0")
, fStringPolarizationX    ( "0")
, fStringPolarizationY    ( "0")
, fStringPolarizationZ    ( "0")
{
    fTTFmanager = new TTreeFormulaManager();
}

BxGeneratorTTree::SubEventConfigTTF::~SubEventConfigTTF() {
    delete fFormulaSubEventRotateIso;
    delete fFormulaNParticles       ;
    delete fFormulaParticleSkip     ;
    delete fFormulaParticleRotateIso;
    delete fFormulaPdg              ;
    delete fFormulaEnergy           ;
    delete fFormulaMomentumX        ;
    delete fFormulaMomentumY        ;
    delete fFormulaMomentumZ        ;
    delete fFormulaPositionX        ;
    delete fFormulaPositionY        ;
    delete fFormulaPositionZ        ;
    delete fFormulaTime             ;
    delete fFormulaPolarizationX    ;
    delete fFormulaPolarizationY    ;
    delete fFormulaPolarizationZ    ;
    // do NOT delete pointer to TreeChain
}

void BxGeneratorTTree::SubEventConfigTTF::SetEnergyUnit(const G4String& unitName) {
    if (G4UnitDefinition::GetCategory(unitName) != "Energy") {
        BxLog(error) << "Unit of \"Energy\" has wrong category or name!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fUnitEnergy = G4UnitDefinition::GetValueOf(unitName);
}
void BxGeneratorTTree::SubEventConfigTTF::SetMomentumUnit(const G4String& unitName) {
    if (G4UnitDefinition::GetCategory(unitName) != "Energy") {
        BxLog(error) << "Unit of \"Momentum\" has wrong category or name!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fUnitMomentum = G4UnitDefinition::GetValueOf(unitName);
}
void BxGeneratorTTree::SubEventConfigTTF::SetPositionUnit(const G4String& unitName) {
    if (G4UnitDefinition::GetCategory(unitName) != "Length") {
        BxLog(error) << "Unit of \"Position\" has wrong category or name!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fUnitPosition = G4UnitDefinition::GetValueOf(unitName);
}
void BxGeneratorTTree::SubEventConfigTTF::SetTimeUnit(const G4String& unitName) {
    if (G4UnitDefinition::GetCategory(unitName) != "Time") {
        BxLog(error) << "Unit of \"Time\" has wrong category or name!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    fUnitTime = G4UnitDefinition::GetValueOf(unitName);
}

void BxGeneratorTTree::SubEventConfigTTF::Initialize() {
    fFormulaSubEventRotateIso = new TTreeFormula("tf", fStringSubEventRotateIso, fTreeChain);
    fFormulaNParticles        = new TTreeFormula("tf", fStringNParticles       , fTreeChain);
    fFormulaParticleSkip      = new TTreeFormula("tf", fStringParticleSkip     , fTreeChain);
    fFormulaParticleRotateIso = new TTreeFormula("tf", fStringParticleRotateIso, fTreeChain);
    fFormulaPdg               = new TTreeFormula("tf", fStringPdg              , fTreeChain);
    fFormulaEnergy            = new TTreeFormula("tf", fStringEnergy           , fTreeChain);
    fFormulaMomentumX         = new TTreeFormula("tf", fStringMomentumX        , fTreeChain);
    fFormulaMomentumY         = new TTreeFormula("tf", fStringMomentumY        , fTreeChain);
    fFormulaMomentumZ         = new TTreeFormula("tf", fStringMomentumZ        , fTreeChain);
    fFormulaPositionX         = new TTreeFormula("tf", fStringPositionX        , fTreeChain);
    fFormulaPositionY         = new TTreeFormula("tf", fStringPositionY        , fTreeChain);
    fFormulaPositionZ         = new TTreeFormula("tf", fStringPositionZ        , fTreeChain);
    fFormulaTime              = new TTreeFormula("tf", fStringTime             , fTreeChain);
    fFormulaPolarizationX     = new TTreeFormula("tf", fStringPolarizationX    , fTreeChain);
    fFormulaPolarizationY     = new TTreeFormula("tf", fStringPolarizationY    , fTreeChain);
    fFormulaPolarizationZ     = new TTreeFormula("tf", fStringPolarizationZ    , fTreeChain);
    
    fFormulaSubEventRotateIso->SetQuickLoad(true);
    fFormulaNParticles       ->SetQuickLoad(true);
    fFormulaParticleSkip     ->SetQuickLoad(true);
    fFormulaParticleRotateIso->SetQuickLoad(true);
    fFormulaPdg              ->SetQuickLoad(true);
    fFormulaEnergy           ->SetQuickLoad(true);
    fFormulaMomentumX        ->SetQuickLoad(true);
    fFormulaMomentumY        ->SetQuickLoad(true);
    fFormulaMomentumZ        ->SetQuickLoad(true);
    fFormulaPositionX        ->SetQuickLoad(true);
    fFormulaPositionY        ->SetQuickLoad(true);
    fFormulaPositionZ        ->SetQuickLoad(true);
    fFormulaTime             ->SetQuickLoad(true);
    fFormulaPolarizationX    ->SetQuickLoad(true);
    fFormulaPolarizationY    ->SetQuickLoad(true);
    fFormulaPolarizationZ    ->SetQuickLoad(true);
    
    if (fFormulaSubEventRotateIso->GetMultiplicity() != 0) {
        BxLog(error) << "\"SubEventRotateIso\" variable has wrong multiplicity!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    if (fFormulaNParticles->GetMultiplicity() != 0) {
        BxLog(error) << "\"Number of particles\" variable has wrong multiplicity!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    if (fFormulaParticleSkip     ->GetMultiplicity()) fTTFmanager->Add(fFormulaParticleSkip     );
    if (fFormulaParticleRotateIso->GetMultiplicity()) fTTFmanager->Add(fFormulaParticleRotateIso);
    if (fFormulaPdg              ->GetMultiplicity()) fTTFmanager->Add(fFormulaPdg              );
    if (fFormulaEnergy           ->GetMultiplicity()) fTTFmanager->Add(fFormulaEnergy           );
    if (fFormulaMomentumX        ->GetMultiplicity()) fTTFmanager->Add(fFormulaMomentumX        );
    if (fFormulaMomentumY        ->GetMultiplicity()) fTTFmanager->Add(fFormulaMomentumY        );
    if (fFormulaMomentumZ        ->GetMultiplicity()) fTTFmanager->Add(fFormulaMomentumZ        );
    if (fFormulaPositionX        ->GetMultiplicity()) fTTFmanager->Add(fFormulaPositionX        );
    if (fFormulaPositionY        ->GetMultiplicity()) fTTFmanager->Add(fFormulaPositionY        );
    if (fFormulaPositionZ        ->GetMultiplicity()) fTTFmanager->Add(fFormulaPositionZ        );
    if (fFormulaTime             ->GetMultiplicity()) fTTFmanager->Add(fFormulaTime             );
    if (fFormulaPolarizationX    ->GetMultiplicity()) fTTFmanager->Add(fFormulaPolarizationX    );
    if (fFormulaPolarizationY    ->GetMultiplicity()) fTTFmanager->Add(fFormulaPolarizationY    );
    if (fFormulaPolarizationZ    ->GetMultiplicity()) fTTFmanager->Add(fFormulaPolarizationZ    );
    
    fTTFmanager->Sync();
}

Bool_t   BxGeneratorTTree::SubEventConfigTTF::EvalSubEventRotateIso(       ) const { return fFormulaSubEventRotateIso->EvalInstance64(0); }
Long64_t BxGeneratorTTree::SubEventConfigTTF::EvalNParticles       (       ) const { return fFormulaNParticles       ->EvalInstance64(0); }
Bool_t   BxGeneratorTTree::SubEventConfigTTF::EvalParticleSkip     (Int_t i) const { return fFormulaParticleSkip     ->EvalInstance64(i); }
Bool_t   BxGeneratorTTree::SubEventConfigTTF::EvalParticleRotateIso(Int_t i) const { return fFormulaParticleRotateIso->EvalInstance64(i); }
Long64_t BxGeneratorTTree::SubEventConfigTTF::EvalPdg              (Int_t i) const { return fFormulaPdg              ->EvalInstance64(i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalEnergy           (Int_t i) const { return fFormulaEnergy           ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalMomentumX        (Int_t i) const { return fFormulaMomentumX        ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalMomentumY        (Int_t i) const { return fFormulaMomentumY        ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalMomentumZ        (Int_t i) const { return fFormulaMomentumZ        ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalPositionX        (Int_t i) const { return fFormulaPositionX        ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalPositionY        (Int_t i) const { return fFormulaPositionY        ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalPositionZ        (Int_t i) const { return fFormulaPositionZ        ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalTime             (Int_t i) const { return fFormulaTime             ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalPolarizationX    (Int_t i) const { return fFormulaPolarizationX    ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalPolarizationY    (Int_t i) const { return fFormulaPolarizationY    ->EvalInstance  (i); }
Double_t BxGeneratorTTree::SubEventConfigTTF::EvalPolarizationZ    (Int_t i) const { return fFormulaPolarizationZ    ->EvalInstance  (i); }

const G4String& BxGeneratorTTree::SubEventConfigTTF::LogMessage() {
    std::stringstream ss;
    ss
    << "\n\tSubEventRotateIso : " << fFormulaSubEventRotateIso->GetTitle()
    << "\n\tNParticles        : " << fFormulaNParticles       ->GetTitle()
    << "\n\tParticleSkipIf    : " << fFormulaParticleSkip     ->GetTitle()
    << "\n\tParticleRotateIso : " << fFormulaParticleRotateIso->GetTitle()
    << "\n\tPdg               : " << fFormulaPdg              ->GetTitle()
    << "\n\tEnergy            : " << fFormulaEnergy           ->GetTitle()
    << "\n\tMomentumX         : " << fFormulaMomentumX        ->GetTitle()
    << "\n\tMomentumY         : " << fFormulaMomentumY        ->GetTitle()
    << "\n\tMomentumZ         : " << fFormulaMomentumZ        ->GetTitle()
    << "\n\tPositionX         : " << fFormulaPositionX        ->GetTitle()
    << "\n\tPositionY         : " << fFormulaPositionY        ->GetTitle()
    << "\n\tPositionZ         : " << fFormulaPositionZ        ->GetTitle()
    << "\n\tTime              : " << fFormulaTime             ->GetTitle()
    << "\n\tPolarizationX     : " << fFormulaPolarizationX    ->GetTitle()
    << "\n\tPolarizationY     : " << fFormulaPolarizationY    ->GetTitle()
    << "\n\tPolarizationZ     : " << fFormulaPolarizationZ    ->GetTitle()
    << endl;
    return ss.str();
}


BxGeneratorTTree::EventConfigTTF::EventConfigTTF(TChain* pTreeChain)
: fTreeChain(pTreeChain)
, fEventIdIsSet(false)
, fStringEventId       ("0")
, fStringEventSkip     ("0")
, fStringEventRotateIso("0")
{
    AddSubEvent();
    fNotifyGroup = new TObjectNotifyGroup();
}

BxGeneratorTTree::EventConfigTTF::~EventConfigTTF() {
    delete fFormulaEventId;
    delete fFormulaEventSkip;
    delete fFormulaEventRotateIso;
}

void BxGeneratorTTree::EventConfigTTF::AddSubEvent() {
    fSubEvents.push_back(new SubEventConfigTTF(fTreeChain));
}

void BxGeneratorTTree::EventConfigTTF::Initialize() {
    fFormulaEventId        = new TTreeFormula("tf", fStringEventId       .data(), fTreeChain);
    fFormulaEventSkip      = new TTreeFormula("tf", fStringEventSkip     .data(), fTreeChain);
    fFormulaEventRotateIso = new TTreeFormula("tf", fStringEventRotateIso.data(), fTreeChain);
    fFormulaEventId       ->SetQuickLoad(true);
    fFormulaEventSkip     ->SetQuickLoad(true);
    fFormulaEventRotateIso->SetQuickLoad(true);
    if (fFormulaEventId->GetMultiplicity() != 0) {
        BxLog(error) << "\"EventId\" variable has wrong multiplicity!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    if (fFormulaEventSkip->GetMultiplicity() != 0) {
        BxLog(error) << "\"EventSkipIf\" variable has wrong multiplicity!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    if (fFormulaEventRotateIso->GetMultiplicity() != 0) {
        BxLog(error) << "\"EventRotateIso\" variable has wrong multiplicity!" << endlog;
        BxLog(fatal) << "FATAL " << endlog;
    }
    for (size_t i = 0; i < fSubEvents.size(); ++i) {
        fSubEvents[i].Initialize();
        fNotifyGroup->push_back(fSubEvents[i].GetManager());
    }
    fTreeChain->SetNotify(fNotifyGroup);
}

Long64_t BxGeneratorTTree::EventConfigTTF::EvalEventId       () { return fFormulaEventId       ->EvalInstance64(0); }
Bool_t   BxGeneratorTTree::EventConfigTTF::EvalEventSkip     () { return fFormulaEventSkip     ->EvalInstance64(0); }
Bool_t   BxGeneratorTTree::EventConfigTTF::EvalEventRotateIso() { return fFormulaEventRotateIso->EvalInstance64(0); }

void BxGeneratorTTree::EventConfigTTF::CheckInOnEntry(G4int entry_number) {
    fTreeChain->LoadTree(entry_number);
    for (size_t i = 0; i < fSubEvents.size(); ++i) {
        fSubEvents[i].GetManager()->GetNdata();
    }
}

void BxGeneratorTTree::EventConfigTTF::Log() {
    std::stringstream ss;
    ss << "\nTTreeFormula-s :"
        << "\n\tEventId        : " << fFormulaEventId       ->GetTitle()
        << "\n\tEventSkipIf    : " << fFormulaEventSkip     ->GetTitle()
        << "\n\tEventRotateIso : " << fFormulaEventRotateIso->GetTitle()
        << endl;
    if (fSubEvents.size() == 1) {
        ss << fSubEvents[0].LogMessage();
    } else {
        for (size_t i = 0; i < fSubEvents.size(); ++i) {
            ss << "SubEvent " << i << " : ";
            ss << fSubEvents[i].LogMessage();
        }
    }
    BxLog(trace) << ss.str() << endlog;
}


Bool_t BxGeneratorTTree::EventConfigTTF::TObjectNotifyGroup::Notify() {
    Bool_t success = true;
    for (size_t i = 0; i < (*this).size(); ++i) success &= (*this)[i]->Notify();
    return success;
}