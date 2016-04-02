#!/bin/bash

offline_dir="`readlink -e $1`"
g4bx2_dir=${offline_dir}/bxmc/g4bx2

rsync -hhruP --exclude '*.sh' --exclude '.git' --exclude '.gitignore' --exclude 'README.md' ${BXUSERS}/viatro/BxGeneratorTTree/ ${g4bx2_dir}/

cd ${g4bx2_dir}/src
if ! grep -q 'BxGeneratorTTree' BxPrimaryGeneratorActionMessenger.cc; then
	sed -i '0,/#include / s//#include "BxGeneratorTTree.hh"\n&/' BxPrimaryGeneratorActionMessenger.cc 
	sed -i '/fSelectCmd->SetCandidates/ i\  candidates += " TTree";' BxPrimaryGeneratorActionMessenger.cc
	sed -i '0,/(newValue ==/ s// (newValue == "TTree") {\
		BxOutputVertex::Get()->SetGenerator(50);\
		fGeneratorPrimary->SetBxGenerator(new BxGeneratorTTree);\
	} else if &/' BxPrimaryGeneratorActionMessenger.cc
fi
grep -q 'if (evt->IsAborted())  return;' BxEventAction.cc || sed -i '/BxEventAction::EndOfEventAction/ a\  if (evt->IsAborted())  return;' BxEventAction.cc
grep -q '(theVertexStructure.theUsers).clear();' BxOutputVertex.cc || sed -i '/(theVertexStructure.theDeposits).clear();/ a\  (theVertexStructure.theUsers).clear();' BxOutputVertex.cc
grep -q 'theVertexStructure.NUsers      = 0;' BxOutputVertex.cc || sed -i '/theVertexStructure.NDaughters      = 0;/ a\  theVertexStructure.NUsers      = 0;'  BxOutputVertex.cc
grep -q 'theVertexStructure.NDeposits      = 0;' BxOutputVertex.cc || sed -i '/theVertexStructure.NDaughters      = 0;/ a\  theVertexStructure.NDeposits      = 0;'  BxOutputVertex.cc
grep -q 'theVertexStructure.MuNPE	     = 0;' BxOutputVertex.cc || sed -i '/theVertexStructure.NPE	     = 0;/ a\  theVertexStructure.MuNPE	     = 0;'  BxOutputVertex.cc
grep -q 'theVertexStructure.NMuPhotons	     = 0;' BxOutputVertex.cc || sed -i '/theVertexStructure.NPhotons	     = 0;/ a\  theVertexStructure.NMuPhotons	     = 0;'  BxOutputVertex.cc

cd ${g4bx2_dir}/include
grep -q 'G4bool                        fIsAppliedSetEventID;' BxOutputVertex.hh || sed -i '/G4bool                        fWriteDeposits;/ i\    G4bool                        fIsAppliedSetEventID;' BxOutputVertex.hh
grep -q 'fIsAppliedSetEventID = true;' BxOutputVertex.hh || sed -i '/theVertexStructure.EventID	    = val;/ s//& fIsAppliedSetEventID = true;/' BxOutputVertex.hh
grep -q 'G4bool IsAppliedSetEventID()' BxOutputVertex.hh || sed -i '/G4bool GetWriteDeposits()/i\    G4bool IsAppliedSetEventID()         { return fIsAppliedSetEventID; }\n' BxOutputVertex.hh 
cd ${g4bx2_dir}/src
grep -q 'fIsAppliedSetEventID = false' BxOutputVertex.cc || sed -i '/BxOutputVertex::ClearAll()/ a\  fIsAppliedSetEventID = false;' BxOutputVertex.cc
grep -q 'if ( ! BxOutputVertex::Get()->IsAppliedSetEventID() )' BxOutputVertex.cc || sed -i '/BxOutputVertex::Get()->SetEventID(evt->GetEventID());/ s//if ( ! BxOutputVertex::Get()->IsAppliedSetEventID() )  &/' BxEventAction.cc

cd ${g4bx2_dir}/include
grep -q 'GetBxGenerator' BxPrimaryGeneratorAction.hh || sed -i '/SetBxGenerator/ a\    BxVGenerator* GetBxGenerator() const {return generator;}' BxPrimaryGeneratorAction.hh
cd ${g4bx2_dir}/src
if ! grep -q 'BxGeneratorTTree' BxStackingActionMessenger.cc; then
	sed -i '/#include "BxStackingNeutron.hh"/ a\#include "BxStackingTTree.hh"' BxStackingActionMessenger.cc
	sed -i '/fSelectCmd->SetCandidates/ i\  candidates += " TTree";' BxStackingActionMessenger.cc
	sed -i '/newValue == \"Neutron\"/ a\    else if (newValue == "TTree")  fStacking->SetBxStackingAction(new BxStackingTTree);' BxStackingActionMessenger.cc
fi

cd ${g4bx2_dir}/include
grep -q 'PreUserTrackingAction' BxTrackingAction.hh || sed -i '/PostUserTrackingAction/ i\    virtual void PreUserTrackingAction(const G4Track*);' BxTrackingAction.hh
cd ${g4bx2_dir}/src
if ! grep -q 'BxTrackInformation' BxTrackingAction.cc; then
	sed -i '/#include "BxTrackingAction.hh"/ a\#include "BxTrackInformation.hh"' BxTrackingAction.cc
	sed -i '/#include "G4Track.hh"/ a\#include "G4TrackVector.hh"' BxTrackingAction.cc
	grep -q '::PreUserTrackingAction' BxTrackingAction.cc || sed -i '/::PostUserTrackingAction/ i\::PreUserTrackingAction\n' BxTrackingAction.cc
	sed -i '/::PreUserTrackingAction/ c\void BxTrackingAction::PreUserTrackingAction(const G4Track* aTrack) {'   BxTrackingAction.cc
	sed -i '/::PreUserTrackingAction/ a\
    if (aTrack->GetParentID() == 0 && aTrack->GetUserInformation() == 0) {\
        BxTrackInformation* anInfo = new BxTrackInformation(aTrack);\
        G4Track* theTrack = (G4Track*)aTrack;\
        theTrack->SetUserInformation(anInfo);\
    }\n}\n' BxTrackingAction.cc
	sed -i '/::PostUserTrackingAction/ a\
    G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();\
    if (secondaries) {\
        BxTrackInformation* info = (BxTrackInformation*)aTrack->GetUserInformation();\
        size_t nSeco = secondaries->size();\
        if (nSeco > 0) {\
            for (size_t i = 0; i < nSeco; ++i) {\
                BxTrackInformation* infoNew = new BxTrackInformation(info);\
                (*secondaries)[i]->SetUserInformation(infoNew);\
            }\
        }\
    }' BxTrackingAction.cc
fi

cd ${g4bx2_dir}
grep -q 'TreePlayer' CMakeLists.txt || sed -i 's/\${ROOT_LIBRARIES}/& TreePlayer/g' CMakeLists.txt
grep -q 'ttree.mac' CMakeLists.txt || sed -i '/gun.mac/ a\ttree.mac' CMakeLists.txt
source please_source_me

cd ${g4bx2_dir}/build
cmake -DGeant4_DIR=${BXSOFTWARE}/geant4/geant4.10.00.p02/ ../. &&
make -j 8

#TEMPORARY! Until bugfix
cd ${offline_dir}/Echidna/event
if ! grep -q 'false && write_flag > 1' BxEvent.cc; then
	sed -i '/write_flag > 1/ s//false \&\& &/' BxEvent.cc
	sed -i '/false && write_flag > 1/ i\        for (int j = 0; j < e.get_frame(i).get_n_users(); j++)  users.push_back(BxMcTruthUser(i+1, e.get_frame(i).get_user(j)));' BxEvent.cc
fi
cd ${offline_dir}/Echidna
#make clean
. /opt/exp_software/borexino/root32/v5-34-24/bin/thisroot.sh
make -j 8

