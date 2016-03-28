//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//Created by D. Franco
//Revised by A. Caminata and S. Marcocci, Sept. 2014

#ifndef BxStackingTTree_h
#define BxStackingTTree_h 1

#include "BxVStackingAction.hh"
#include "BxStackingAction.hh"

#include "G4ClassificationOfNewTrack.hh"
#include "G4UserStackingAction.hh"

#include <bitset>

class BxGeneratorTTree;
//class BxStackingTTreeMessenger;
class G4StackManager;
class G4Track;

//
/** This is the base class of one of the user's optional action classes.
*   This class gives the hooks for G4StackManager which controls the stacks
*   of G4Track objects.
*/

class BxStackingTTree : public BxVStackingAction {
public:
    BxStackingTTree();
    virtual ~BxStackingTTree();

public: // with description
    //---------------------------------------------------------------
    // vitual methods to be implemented by user
    //---------------------------------------------------------------
    //
    virtual G4ClassificationOfNewTrack BxClassifyNewTrack(const G4Track* aTrack);
    //
    //    Reply G4ClassificationOfNewTrack determined by the
    //  newly coming G4Track.
    //
    //    enum G4ClassificationOfNewTrack
    //    {
    //      fUrgent,    // put into the urgent stack
    //      fWaiting,   // put into the waiting stack
    //      fPostpone,  // postpone to the next event
    //      fKill       // kill without stacking
    //    };
    //
    //    The parent_ID of the track indicates the origin of it.
    //                
    //    G4int parent_ID = aTrack->GetParentID();
    //   
    //      parent_ID = 0 : primary particle
    //                > 0 : secondary particle
    //                < 0 : postponed from the previous event
    //
    //---------------------------------------------------------------
    //
    /**
    *    This method is called by G4StackManager when the urgentStack
    *  becomes empty and contents in the waitingStack are transtered
    *  to the urgentStack.
    *    Note that this method is not called at the begining of each
    *  event, but "PrepareNewEvent" is called.
    *
    *    In case re-classification of the stacked tracks is needed,
    *  use the following method to request to G4StackManager.
    *
    *    stackManager->ReClassify();
    *
    *    All of the stacked tracks in the waitingStack will be re-classified 
    *  by "ClassifyNewTrack" method.
    *    To abort current event, use the following method.
    *
    *    stackManager->clear();
    *
    *    Note that this way is valid and safe only for the case it is called
    *  from this user class. The more global way of event abortion is
    *
    *    G4UImanager * UImanager = G4UImanager::GetUIpointer();
    *    UImanager->ApplyCommand("/event/abort");
    */
    virtual void BxNewStage();
    //---------------------------------------------------------------
    //
    /**
    *    This method is called by G4StackManager at the begining of
    *  each event.
    *    Be careful that the urgentStack and the waitingStack of 
    *  G4StackManager are empty at this moment, because this method
    *  is called before accepting primary particles. Also, note that
    *  the postponeStack of G4StackManager may have some postponed
    *  tracks.
    */
    virtual void BxPrepareNewEvent();
    //---------------------------------------------------------------
    
    void SetMode(size_t pos, G4bool val) { fMode.set(pos,val); }
    
private:
    //BxStackingTTreeMessenger  *fMessenger ;
    
    G4int  fCurrentPrimaryNumber;
    BxGeneratorTTree* generator;
    G4bool fIsFirst;
    
    std::bitset<3> fMode; //[0] - gamma from neutron capture, [1] - radioactive decay, [2] - muon decay
};

#endif

