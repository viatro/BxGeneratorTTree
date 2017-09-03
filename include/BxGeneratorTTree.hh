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
#include <map>
#include <deque>

class BxGeneratorTTreeMessenger;
class TTreeFormula;
class TTreeFormulaManager;
class G4Event;
class G4ParticleGun;

class BxGeneratorTTree : public BxVGenerator {
public:
    /// default constructor
    BxGeneratorTTree();
    
    //copy constructor
    //BxGeneratorTTree(const BxGeneratorTTree &);
    
    /// destructor
    virtual ~BxGeneratorTTree();
    
    virtual void Initialize();
    
    /// public interface
    virtual void BxGeneratePrimaries(G4Event* event);
    
    /**
     *  Set the TTree(Chain) name and path to input ROOT file(s).
     *  \param[in] treename Name of TTree. If TTree is in directory inside ROOT file, its name is "directory/treename"
     *  \ param filename Name of ROOT file with extension and path
     */
    void AddTree(const G4String& treename, const G4String& filename);
    
    /// Set alias for expression, same as in TTree::SetAlias
    void SetAlias(const G4String& alias, const G4String& expression);
    
    /// Set the first entry to be processed.
    inline void SetFirstEntry(const G4int a) { fFirstEntry = a; }

    /// Get the first entry to be processed.
    inline G4int GetFirstEntry() const { return fFirstEntry; }
    
    /// Set the number of entries to be processed.
    inline void SetNEntries(const G4int a) { fNEntries = a; }

    /// Get the number of entries to be processed.
    inline G4int GetNEntries() const { return fNEntries; }
    
    /**
     *  Write info about primary particles to log.
     *  Default is true.
     *  Turn off when simulate huge amounts of particles per event.
     */
    inline void SetLogPrimariesInfo(G4bool a) { fLogPrimariesInfo  = a; }
    
    /**
     *  Write info about primary particles to output file.
     *  Default is true.
     *  Turn off when simulate huge amounts of particles per event with known parameters.
     */
    inline void SetSavePrimariesInfo(G4bool a) { fSavePrimariesInfo = a; }
    
    /// Configurator sub-class
    class SubEventConfigTTF; // forward declaration of nested class
    
    /// Configurator class
    class EventConfigTTF;    // forward declaration of nested class
    
    /// Get configurator as const
    const EventConfigTTF* GetEventConfigTTF() const { return fEventConfigTTF; }
    
    /// Get configurator non-const
          EventConfigTTF* GetEventConfigTTF()       { return fEventConfigTTF; }
    
    
private:
    TChain* fTreeChain;       
    G4int   fCurrentEntry;    ///< Entry counter
    G4int   fFirstEntry;      ///< First entry to be read.
    G4int   fLastEntry;       ///< Last entry to be read.
    G4int   fNEntries;        ///< Number of entries to be processed
    G4bool  fIsInitialized;   ///< Initialization flag
    
    G4bool  fLogPrimariesInfo;  ///< Flag to write info about primary particles to log
    G4bool  fSavePrimariesInfo; ///< Flag to write info about primary particles to output file
    
    G4ParticleGun* fParticleGun;
    
    BxGeneratorTTreeMessenger* fMessenger; ///< Messenger
    
    EventConfigTTF* fEventConfigTTF; ///< Configurator
    
public:
    /// Holder of particles parameters
    struct ParticleInfo {
        G4int         event_id;     
        G4int         p_index;      ///< Sequence number of primary particle in TTree entry
        G4int         status;       ///< Stacking mode status of particle
        G4int         pdg_code;     
        G4double      energy;       
        G4ThreeVector momentum;     
        G4ThreeVector position;     
        G4double      time;         
        G4ThreeVector polarization; 
    };
    
    void  PushFrontParticleInfo(const ParticleInfo& particle_info) { fDequeParticleInfo.push_front(particle_info); }
    void  PushBackParticleInfo (const ParticleInfo& particle_info) { fDequeParticleInfo.push_back(particle_info); }
    const std::vector<ParticleInfo>& GetCurrentPrimaryParticlesInfo() const { return fCurrentParticlesInfo; }
    
    void PushFrontParticleInfo(G4int event_id, G4int p_index, G4int status,
        G4int pdg_code, G4double energy, const G4ThreeVector& momentum,
        const G4ThreeVector& position, G4double time, const G4ThreeVector& polarization);
    
    void PushBackParticleInfo(G4int event_id, G4int p_index, G4int status,
        G4int pdg_code, G4double energy, const G4ThreeVector& momentum,
        const G4ThreeVector& position, G4double time, const G4ThreeVector& polarization);
    
private:
    G4bool FillDequeFromEntry(G4int entry_number);
    
    std::deque<ParticleInfo>  fDequeParticleInfo;
    std::vector<ParticleInfo> fCurrentParticlesInfo;
    
    //TODO: with C++11 change to lambda in BxGeneratorTTree.cc
    struct ParticleInfoCompareByTime {
        bool operator() (const ParticleInfo& p1, const ParticleInfo& p2) { return p1.time < p2.time; }
    } particle_info_compare_by_time;
};


class BxGeneratorTTree::SubEventConfigTTF {
public:
    SubEventConfigTTF(TChain*);
    virtual ~SubEventConfigTTF();
    
    Double_t GetEnergyUnit  () const { return fUnitEnergy  ; }
    Double_t GetMomentumUnit() const { return fUnitMomentum; }
    Double_t GetPositionUnit() const { return fUnitPosition; }
    Double_t GetTimeUnit    () const { return fUnitTime    ; }
    
    void SetEnergyUnit  (G4double unit) { fUnitEnergy   = unit; }
    void SetMomentumUnit(G4double unit) { fUnitMomentum = unit; }
    void SetPositionUnit(G4double unit) { fUnitPosition = unit; }
    void SetTimeUnit    (G4double unit) { fUnitTime     = unit; }
    
    void SetEnergyUnit  (const G4String& unitName);
    void SetMomentumUnit(const G4String& unitName);
    void SetPositionUnit(const G4String& unitName);
    void SetTimeUnit    (const G4String& unitName);
    
    void SetSubEventRotateIso(const G4String& val) { fStringSubEventRotateIso = val; }
    void SetNParticles       (const G4String& val) { fStringNParticles        = val; }
    void SetParticleSkip     (const G4String& val) { fStringParticleSkip      = val; }
    void SetParticleRotateIso(const G4String& val) { fStringParticleRotateIso = val; }
    void SetPdg              (const G4String& val) { fStringPdg               = val; }
    void SetEnergy           (const G4String& val) { fStringEnergy            = val; }
    void SetMomentumX        (const G4String& val) { fStringMomentumX         = val; }
    void SetMomentumY        (const G4String& val) { fStringMomentumY         = val; }
    void SetMomentumZ        (const G4String& val) { fStringMomentumZ         = val; }
    void SetPositionX        (const G4String& val) { fStringPositionX         = val; }
    void SetPositionY        (const G4String& val) { fStringPositionY         = val; }
    void SetPositionZ        (const G4String& val) { fStringPositionZ         = val; }
    void SetTime             (const G4String& val) { fStringTime              = val; }
    void SetPolarizationX    (const G4String& val) { fStringPolarizationX     = val; }
    void SetPolarizationY    (const G4String& val) { fStringPolarizationY     = val; }
    void SetPolarizationZ    (const G4String& val) { fStringPolarizationZ     = val; }
    
    void Initialize();
    
    Bool_t   EvalSubEventRotateIso(       ) const;
    Long64_t EvalNParticles       (       ) const;
    Bool_t   EvalParticleSkip     (Int_t i) const;
    Bool_t   EvalParticleRotateIso(Int_t i) const;
    Long64_t EvalPdg              (Int_t i) const;
    Double_t EvalEnergy           (Int_t i) const;
    Double_t EvalMomentumX        (Int_t i) const;
    Double_t EvalMomentumY        (Int_t i) const;
    Double_t EvalMomentumZ        (Int_t i) const;
    Double_t EvalPositionX        (Int_t i) const;
    Double_t EvalPositionY        (Int_t i) const;
    Double_t EvalPositionZ        (Int_t i) const;
    Double_t EvalTime             (Int_t i) const;
    Double_t EvalPolarizationX    (Int_t i) const;
    Double_t EvalPolarizationY    (Int_t i) const;
    Double_t EvalPolarizationZ    (Int_t i) const;
    
    const TTreeFormulaManager* GetManager() const { return fTTFmanager; }
          TTreeFormulaManager* GetManager()       { return fTTFmanager; }
    
    const G4String& LogMessage();
    
private:
    TChain* fTreeChain;
    
    TTreeFormula* fFormulaSubEventRotateIso; ///< Formula of sub-event isotropic rotation flag
    TTreeFormula* fFormulaNParticles;        ///< Formula of number of particles in sub-event
    TTreeFormula* fFormulaParticleSkip;      ///< Formula of particle skipping flag
    TTreeFormula* fFormulaParticleRotateIso; ///< Formula of particle isotropic rotation flag
    TTreeFormula* fFormulaPdg;               ///< Formula of PDG code of particle
    TTreeFormula* fFormulaEnergy;            ///< Formula of particle kinetic energy
    TTreeFormula* fFormulaMomentumX;         ///< Formula of particle momentum X-component
    TTreeFormula* fFormulaMomentumY;         ///< Formula of particle momentum Y-component
    TTreeFormula* fFormulaMomentumZ;         ///< Formula of particle momentum Z-component
    TTreeFormula* fFormulaPositionX;         ///< Formula of event vertex position X-component in detector coord. syst.
    TTreeFormula* fFormulaPositionY;         ///< Formula of event vertex position Y-component in detector coord. syst.
    TTreeFormula* fFormulaPositionZ;         ///< Formula of event vertex position Z-component in detector coord. syst.
    TTreeFormula* fFormulaTime;              ///< Formula of event time
    TTreeFormula* fFormulaPolarizationX;     ///< Formula of particle polarization X-component
    TTreeFormula* fFormulaPolarizationY;     ///< Formula of particle polarization Y-component
    TTreeFormula* fFormulaPolarizationZ;     ///< Formula of particle polarization Z-component
    
    TTreeFormulaManager* fTTFmanager;
    
    G4double fUnitEnergy;   ///< unit of energy
    G4double fUnitMomentum; ///< unit of momentum
    G4double fUnitPosition; ///< unit of position
    G4double fUnitTime;     ///< unit of time
    
    G4String fStringSubEventRotateIso; ///< String used to initialize the formula of sub-event isotropic rotation flag
    G4String fStringNParticles;        ///< String used to initialize the formula of number of particles in sub-event
    G4String fStringParticleSkip;      ///< String used to initialize the formula of particle skipping flag
    G4String fStringParticleRotateIso; ///< String used to initialize the formula of particle isotropic rotation flag
    G4String fStringPdg;               ///< String used to initialize the formula of PDG code of particle
    G4String fStringEnergy;            ///< String used to initialize the formula of particle kinetic energy
    G4String fStringMomentumX;         ///< String used to initialize the formula of particle momentum X-component
    G4String fStringMomentumY;         ///< String used to initialize the formula of particle momentum Y-component
    G4String fStringMomentumZ;         ///< String used to initialize the formula of particle momentum Z-component
    G4String fStringPositionX;         ///< String used to initialize the formula of event vertex position X-component in detector coord. syst.
    G4String fStringPositionY;         ///< String used to initialize the formula of event vertex position Y-component in detector coord. syst.
    G4String fStringPositionZ;         ///< String used to initialize the formula of event vertex position Z-component in detector coord. syst.
    G4String fStringTime;              ///< String used to initialize the formula of event time
    G4String fStringPolarizationX;     ///< String used to initialize the formula of particle polarization X-component
    G4String fStringPolarizationY;     ///< String used to initialize the formula of particle polarization Y-component
    G4String fStringPolarizationZ;     ///< String used to initialize the formula of particle polarization Z-component
};

#include <boost/ptr_container/ptr_vector.hpp>

class BxGeneratorTTree::EventConfigTTF {
public:
    EventConfigTTF(TChain*);
    virtual ~EventConfigTTF();
    
    void  AddSubEvent();
    const boost::ptr_vector<SubEventConfigTTF>& GetSubEvents() const { return fSubEvents; }
          boost::ptr_vector<SubEventConfigTTF>& GetSubEvents()       { return fSubEvents; }
    const SubEventConfigTTF& GetSubEvent(size_t i) const { return fSubEvents[i]; }
          SubEventConfigTTF& GetSubEvent(size_t i)       { return fSubEvents[i]; }
    
    Bool_t IsSetEventId() { return fEventIdIsSet; }
    void SetEventId       (const G4String& val) { fStringEventId        = val; fEventIdIsSet = true; }
    void SetEventSkip     (const G4String& val) { fStringEventSkip      = val; }
    void SetEventRotateIso(const G4String& val) { fStringEventRotateIso = val; }
    
    void Initialize();
    
    Long64_t EvalEventId       ();
    Bool_t   EvalEventSkip     ();
    Bool_t   EvalEventRotateIso();
    
    void CheckInOnEntry(G4int entry_number);
    
    void Log();
    
private:
    TChain*       fTreeChain;
    G4bool        fEventIdIsSet;
    
    TTreeFormula* fFormulaEventId;        ///< Formula of event id
    TTreeFormula* fFormulaEventSkip;      ///< Formula of event skipping flag
    TTreeFormula* fFormulaEventRotateIso; ///< Formula of event isotropic rotation flag
    G4String      fStringEventId;         ///< String used to initialize the formula of event id
    G4String      fStringEventSkip;       ///< String used to initialize the formula of event skipping flag
    G4String      fStringEventRotateIso;  ///< String used to initialize the formula of event isotropic rotation flag
    
    boost::ptr_vector<SubEventConfigTTF> fSubEvents;
    
    class TObjectNotifyGroup;
    TObjectNotifyGroup* fNotifyGroup;
};

class BxGeneratorTTree::EventConfigTTF::TObjectNotifyGroup : public std::vector< TObject* >, public TObject {
public:
    TObjectNotifyGroup() : TObject() {};
    virtual ~TObjectNotifyGroup() {};

    virtual Bool_t Notify();
};

#endif
