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

class G4Event;
class BxGeneratorTTreeMessenger;
class TTreeFormula;
class TTreeFormulaManager;


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

    // Get the input file to read.
    //virtual inline G4String GetInputFileName() const {return fInputFileName;}
    
    /// Set the TTree name.
    void SetTreeName(const G4String& name) {fTreeChain->SetName(name.data());}

    /// Get the TTree name.
    G4String GetTreeName() const {return fTreeChain->GetName();}
    
    /// Set the first entry to be processed.
    inline void SetFirstEntry(const G4int a) {fFirstEntry = a;}

    /// Get the first entry to be processed.
    inline G4int GetFirstEntry() const {return fFirstEntry;}
    
    /// Set the number of entries to be processed.
    inline void SetNEntries(const G4int a) {fNEntries = a;}

    /// Get the number of entries to be processed.
    inline G4int GetNEntries() const {return fNEntries;}
    
    inline void SetVarStringEventId      (const G4String& a) {fVarString_EventId      = a;}
    inline void SetVarStringEventSkip    (const G4String& a) {fVarString_EventSkip    = a;}
    inline void SetVarStringParticleSkip (const G4String& a) {fVarString_ParticleSkip = a;}
    inline void SetVarStringNParticles   (const G4String& a) {fVarString_NParticles   = a;}
    inline void SetVarStringSplit        (const G4String& a) {fVarString_Split        = a;}
    inline void SetVarStringPdg          (const G4String& a) {fVarString_Pdg          = a;}
    inline void SetVarStringEkin         (const G4String& a) {fVarString_Ekin         = a;}
    inline void SetVarStringMomentum     (const G4String& a) {fVarString_Momentum     = a;}
    inline void SetVarStringCoords       (const G4String& a) {fVarString_Coords       = a;}
    inline void SetVarStringPolarization (const G4String& a) {fVarString_Polarization = a;}
    
    inline G4String GetVarStringEventId      () const {return fVarString_EventId     ;}
    inline G4String GetVarStringEventSkip    () const {return fVarString_EventSkip   ;}
    inline G4String GetVarStringParticleSkip () const {return fVarString_ParticleSkip;}
    inline G4String GetVarStringNParticles   () const {return fVarString_NParticles  ;}
    inline G4String GetVarStringSplit        () const {return fVarString_Split       ;}
    inline G4String GetVarStringPdg          () const {return fVarString_Pdg         ;}
    inline G4String GetVarStringEkin         () const {return fVarString_Ekin        ;}
    inline G4String GetVarStringMomentum     () const {return fVarString_Momentum    ;}
    inline G4String GetVarStringCoords       () const {return fVarString_Coords      ;}
    inline G4String GetVarStringPolarization () const {return fVarString_Polarization;}
  
  //private  members
private:
    G4int       fCurrentEntry;  ///< TTree entry counter
    G4bool      fIsInitialized; ///< initialization flag
    TChain*     fTreeChain;     ///< event tree that contains the events.
    G4int       fFirstEntry;    ///< first entry to be read.
    G4int       fLastEntry;     ///< last entry to be read.
    G4int       fNEntries;      ///< 
    
    G4int       fTreeNumber;
    G4int       fParticleCounter;
    
    G4String    fVarString_EventId;
    G4String    fVarString_EventSkip;
    G4String    fVarString_ParticleSkip;
    G4String    fVarString_NParticles;
    G4String    fVarString_Split;
    G4String    fVarString_Pdg;
    G4String    fVarString_Ekin;
    G4String    fVarString_Momentum;
    G4String    fVarString_Coords;
    G4String    fVarString_Polarization;
    
    G4double    fVarUnit_Ekin;       ///< unit of energy
    G4double    fVarUnit_Momentum;   ///< unit of momentum
    G4double    fVarUnit_Coords;     ///< unit of coordinates
    
    G4bool      fVarIsSet_EventId;
    G4bool      fVarIsSet_Polarization;
    
    TTreeFormula* fVarTTF_EventId;         ///< sequence number of the event (the event number)
    TTreeFormula* fVarTTF_EventSkip;       ///< 
    TTreeFormula* fVarTTF_ParticleSkip;    ///< 
    TTreeFormula* fVarTTF_NParticles;      ///< number of particles
    TTreeFormula* fVarTTF_Split;           ///< 
    TTreeFormula* fVarTTF_Pdg;             ///< PDG code of particle
    TTreeFormula* fVarTTF_Ekin;            ///< particle kinetic energy
    TTreeFormula* fVarTTF_Momentum[3];     ///< particle momentum
    TTreeFormula* fVarTTF_Coords[3];       ///< event vertex position in detector coord. syst.
    TTreeFormula* fVarTTF_Polarization[3]; ///< particle polarization vector
    
    TTreeFormulaManager* fTTFmanager;
    
    G4ParticleTable*           fParticleTable;
    G4ParticleDefinition*      fParticle;
	G4ParticleGun*             fParticleGun;
    BxGeneratorTTreeMessenger* fMessenger;
};

#endif
