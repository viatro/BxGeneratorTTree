#include "BxTrackInformation.hh"

G4Allocator<BxTrackInformation> aTrackInformationAllocator;

BxTrackInformation::BxTrackInformation() :
      fPrimaryTrackID(0)
    //, fPrimaryParticleDefinition(0)
    //, fPrimaryPosition(G4ThreeVector(0.,0.,0.))
    //, fPrimaryMomentum(G4ThreeVector(0.,0.,0.))
    //, fPrimaryEnergy(0.)
    //, fPrimaryTime(0.)
    {}

BxTrackInformation::BxTrackInformation(const G4Track* aTrack) :
      fPrimaryTrackID ( aTrack->GetTrackID() )
    //, fPrimaryParticleDefinition ( aTrack->GetDefinition() )
    //, fPrimaryPosition ( aTrack->GetPosition() )
    //, fPrimaryMomentum ( aTrack->GetMomentum() )
    //, fPrimaryEnergy ( aTrack->GetTotalEnergy() )
    //, fPrimaryTime ( aTrack->GetGlobalTime() )
    {}

BxTrackInformation::BxTrackInformation(const BxTrackInformation* aTrackInfo) :
      fPrimaryTrackID ( aTrackInfo->fPrimaryTrackID )
    //, fPrimaryParticleDefinition ( aTrackInfo->fPrimaryParticleDefinition )
    //, fPrimaryPosition ( aTrackInfo->fPrimaryPosition )
    //, fPrimaryMomentum ( aTrackInfo->fPrimaryMomentum )
    //, fPrimaryEnergy ( aTrackInfo->fPrimaryEnergy )
    //, fPrimaryTime ( aTrackInfo->fPrimaryTime )
    {}

BxTrackInformation::~BxTrackInformation(){;}