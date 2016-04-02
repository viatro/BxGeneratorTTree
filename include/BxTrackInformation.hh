#ifndef BxTrackInformation_h
#define BxTrackInformation_h 1

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleDefinition.hh"
#include "G4Track.hh"
#include "G4Allocator.hh"
#include "G4VUserTrackInformation.hh"
#include "G4ios.hh"

class BxTrackInformation : public G4VUserTrackInformation {
public:
    BxTrackInformation();
    BxTrackInformation(const G4Track* aTrack);
    BxTrackInformation(const BxTrackInformation* aTrackInfo);
    virtual ~BxTrackInformation();
   
    inline void *operator new(size_t);
    inline void operator delete(void *aTrackInfo);
    inline int operator ==(const BxTrackInformation& right) const { return (this==&right); }

    void Print() const {
        G4cout << "Primary track ID " << fPrimaryTrackID /*<< " at " << fPrimaryPosition*/ << G4endl;
    }

private:
    G4int                 fPrimaryTrackID;
    //G4ParticleDefinition* fPrimaryParticleDefinition;
    //G4ThreeVector         fPrimaryPosition;
    //G4ThreeVector         fPrimaryMomentum;
    //G4double              fPrimaryEnergy;
    //G4double              fPrimaryTime;

public:
    inline G4int GetPrimaryTrackID() const { return fPrimaryTrackID; }
    //inline G4ParticleDefinition* GetPrimaryParticle() const { return fPrimaryParticleDefinition; }
    //inline G4ThreeVector GetPrimaryPosition() const { return fPrimaryPosition; }
    //inline G4ThreeVector GetPrimaryMomentum() const { return fPrimaryMomentum; }
    //inline G4double GetPrimaryEnergy() const { return fPrimaryEnergy; }
    //inline G4double GetPrimaryTime() const { return fPrimaryTime; }
};

extern G4Allocator<BxTrackInformation> aTrackInformationAllocator;

inline void* BxTrackInformation::operator new(size_t) {
    //void* aTrackInfo;
    //aTrackInfo = (void*)aTrackInformationAllocator.MallocSingle();
    //return aTrackInfo;
    return (void*)aTrackInformationAllocator.MallocSingle();
}

inline void BxTrackInformation::operator delete(void *aTrackInfo) {
    aTrackInformationAllocator.FreeSingle((BxTrackInformation*)aTrackInfo);
}

#endif