// -------------------------------------------------- //
/**
 * AUTHOR: adapted from http://geant4.slac.stanford.edu/Tips/event/1.html by V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "BxTrackInformation.hh"

G4ThreadLocal G4Allocator<BxTrackInformation>* aTrackInformationAllocator = 0;

BxTrackInformation::BxTrackInformation()
: fPrimaryTrackID(0)
//, fParentPDGEncoding(0)
{}

BxTrackInformation::BxTrackInformation(const G4Track* aTrack)
: fPrimaryTrackID ( aTrack->GetTrackID() )
//, fParentPDGEncoding ( aTrack->GetParticleDefinition()->GetPDGEncoding() )
{}

BxTrackInformation::BxTrackInformation(const BxTrackInformation* aTrackInfo)
: fPrimaryTrackID ( aTrackInfo->fPrimaryTrackID )
//, fParentPDGEncoding ( aTrackInfo->fParentPDGEncoding )
{}

BxTrackInformation::~BxTrackInformation() {}
