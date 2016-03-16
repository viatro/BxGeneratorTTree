// -------------------------------------------------- //
/** 
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#ifndef _BxGeneratorTTree_HH
#define _BxGeneratorTTree_HH

#include "TChain.h"

#include "BxVGenerator.hh"

#include <vector>
#include <deque>

class BxGeneratorTTreeMessenger;
class TTreeFormula;
class TTreeFormulaManager;
class G4Event;


class BxGeneratorTTree : public BxVGenerator {
public:
    ///default constructor
    BxGeneratorTTree();
    
    //copy constructor
    //BxGeneratorTTree(const BxGeneratorTTree &);
    
    ///destructor
    virtual ~BxGeneratorTTree();
    
    virtual void Initialize();
    
    ///public interface
    virtual void BxGeneratePrimaries(G4Event* event);
  
    /// Set the input file to read.
    void AddInputFileName(const G4String& name) {fTreeChain->Add(name.data());}

    /// Set the TTree(Chain) name.
    void SetTreeName(const G4String& name) {fTreeChain->SetName(name.data());}

    /// Get the TTree(Chain) name.
    G4String GetTreeName() const {return fTreeChain->GetName();}
    
    /// Set the first entry to be processed.
    inline void SetFirstEntry(const G4int a) {fFirstEntry = a;}

    /// Get the first entry to be processed.
    inline G4int GetFirstEntry() const {return fFirstEntry;}
    
    /// Set the number of entries to be processed.
    inline void SetNEntries(const G4int a) {fNEntries = a;}

    /// Get the number of entries to be processed.
    inline G4int GetNEntries() const {return fNEntries;}
    
    void SetVarStringEventId      (const G4String& a) {fVarString_EventId      = a;}
    void SetVarStringEventSkip    (const G4String& a) {fVarString_EventSkip    = a;}
    void SetVarStringParticleSkip (const G4String& a) {fVarString_ParticleSkip = a;}
    void SetVarStringNParticles   (const G4String& a) {fVarString_NParticles   = a;}
    void SetVarStringSplit        (const G4String& a) {fVarString_Split        = a;}
    void SetVarStringRotateIso    (const G4String& a) {fVarString_RotateIso    = a;}
    void SetVarStringPdg          (const G4String& a) {fVarString_Pdg          = a;}
    void SetVarStringEkin         (const G4String& a) {fVarString_Ekin         = a;}
    void SetVarStringMomentum     (const G4String& a) {fVarString_Momentum     = a;}
    void SetVarStringPosition     (const G4String& a) {fVarString_Position     = a;}
    void SetVarStringTime         (const G4String& a) {fVarString_Time         = a;}
    void SetVarStringPolarization (const G4String& a) {fVarString_Polarization = a;}
    
    const G4String& GetVarStringEventId      () const {return fVarString_EventId     ;}
    const G4String& GetVarStringEventSkip    () const {return fVarString_EventSkip   ;}
    const G4String& GetVarStringParticleSkip () const {return fVarString_ParticleSkip;}
    const G4String& GetVarStringNParticles   () const {return fVarString_NParticles  ;}
    const G4String& GetVarStringSplit        () const {return fVarString_Split       ;}
    const G4String& GetVarStringRotateIso    () const {return fVarString_RotateIso   ;}
    const G4String& GetVarStringPdg          () const {return fVarString_Pdg         ;}
    const G4String& GetVarStringEkin         () const {return fVarString_Ekin        ;}
    const G4String& GetVarStringMomentum     () const {return fVarString_Momentum    ;}
    const G4String& GetVarStringPosition     () const {return fVarString_Position    ;}
    const G4String& GetVarStringTime         () const {return fVarString_Time        ;}
    const G4String& GetVarStringPolarization () const {return fVarString_Polarization;}
    
//private  members
private:
    TChain*     fTreeChain;     ///< event tree that contains the events.
    G4int       fCurrentEntry;  ///< TTree(Chain) entry counter
    G4int       fFirstEntry;    ///< first entry to be read.
    G4int       fLastEntry;     ///< last entry to be read.
    G4int       fNEntries;      ///< number of entries to be processed
    G4bool      fIsInitialized; ///< initialization flag
    
    //G4int       fTreeNumber;
    G4int       fParticleCounter;
    
    G4String    fVarString_EventId;
    G4String    fVarString_EventSkip;
    G4String    fVarString_ParticleSkip;
    G4String    fVarString_NParticles;
    G4String    fVarString_Split;
    G4String    fVarString_RotateIso;
    G4String    fVarString_Pdg;
    G4String    fVarString_Ekin;
    G4String    fVarString_Momentum;
    G4String    fVarString_Position;
    G4String    fVarString_Time;
    G4String    fVarString_Polarization;
    
    G4double    fVarUnit_Ekin;       ///< unit of energy
    G4double    fVarUnit_Momentum;   ///< unit of momentum
    G4double    fVarUnit_Position;   ///< unit of position
    G4double    fVarUnit_Time;       ///< unit of time
    
    
    G4bool      fVarIsSet_EventId;
    //G4bool      fVarIsSet_Polarization;
    
    TTreeFormula* fVarTTF_EventId;         ///< sequence number of the event (the event number)
    TTreeFormula* fVarTTF_EventSkip;       ///< 
    TTreeFormula* fVarTTF_ParticleSkip;    ///< 
    TTreeFormula* fVarTTF_NParticles;      ///< number of particles
    TTreeFormula* fVarTTF_Split;           ///< 
    TTreeFormula* fVarTTF_RotateIso;       ///< 
    TTreeFormula* fVarTTF_Pdg;             ///< PDG code of particle
    TTreeFormula* fVarTTF_Ekin;            ///< particle kinetic energy
    TTreeFormula* fVarTTF_Momentum[3];     ///< particle momentum
    TTreeFormula* fVarTTF_Position[3];     ///< event vertex position in detector coord. syst.
    TTreeFormula* fVarTTF_Time;            ///< event time
    TTreeFormula* fVarTTF_Polarization[3]; ///< particle polarization vector
    
    TTreeFormulaManager*       fTTFmanager;
    
    G4ParticleTable*           fParticleTable;
	G4ParticleGun*             fParticleGun;
    //G4ParticleDefinition*      fParticle;
    
    BxGeneratorTTreeMessenger* fMessenger;
    
public:
    struct ParticleInfo {
        G4int               event_id;
        G4int               p_index;
        G4int               pdg_code;
        G4double            energy;
        G4ThreeVector       momentum;
        G4ThreeVector       position;
        G4double            time;
        G4ThreeVector       polarization;
        G4int               status;
    };
    
    const ParticleInfo& GetCurrentParticleInfo() const {return fCurrentParticleInfo;}
    void  PushFrontToDeque(const ParticleInfo& particle_info) {fDequeParticleInfo.push_front(particle_info);}
    void  PushBackToDeque(const ParticleInfo& particle_info) {fDequeParticleInfo.push_back(particle_info);}
    
    void PushFrontToDeque(G4int event_id, G4int p_index, G4int pdg_code,
        G4double energy, const G4ThreeVector& momentum, const G4ThreeVector& position, G4double time,
        const G4ThreeVector& polarization, G4int status) {
            ParticleInfo particle_info;
            particle_info.event_id = event_id;
            particle_info.p_index = p_index;
            particle_info.pdg_code = pdg_code;
            particle_info.energy = energy;
            particle_info.momentum = momentum;
            particle_info.position = position;
            particle_info.time = time;
            particle_info.polarization = polarization;
            particle_info.status = status;
            fDequeParticleInfo.push_front(particle_info);
        }
    
    void PushBackToDeque(G4int event_id, G4int p_index, G4int pdg_code,
        G4double energy, const G4ThreeVector& momentum, const G4ThreeVector& position, G4double time,
        const G4ThreeVector& polarization, G4int status) {
            ParticleInfo particle_info;
            particle_info.event_id = event_id;
            particle_info.p_index = p_index;
            particle_info.pdg_code = pdg_code;
            particle_info.energy = energy;
            particle_info.momentum = momentum;
            particle_info.position = position;
            particle_info.time = time;
            particle_info.polarization = polarization;
            particle_info.status = status;
            fDequeParticleInfo.push_back(particle_info);
        }
    
    
private:
    void FillDequeFromEntry(G4int entry_number);
    
    ParticleInfo               fCurrentParticleInfo;
    std::deque<ParticleInfo>   fDequeParticleInfo;
    
    std::vector<G4int>         fPrimaryIndexes;
};

#endif
