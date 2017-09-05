// -------------------------------------------------- //
/** 
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#ifndef BxStackingTTree_h
#define BxStackingTTree_h 1

#include "TFile.h"
#include "TTree.h"

#include "BxVStackingAction.hh"
#include "BxStackingAction.hh"

#include "G4ClassificationOfNewTrack.hh"
#include "G4UserStackingAction.hh"

#include <bitset>
#include <set>
#include <map>

class BxGeneratorTTree;
class BxStackingTTreeMessenger;
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
    //
    
    void SetMode(size_t pos, G4bool val) { fMode.set(pos,val); }
    void SetMode(G4bool val) { val ? fMode.set() : fMode.reset(); }
    
    void SetKillMode(size_t pos, G4bool val) { fKillMode.set(pos,val); }
    void SetKillMode(G4bool val) { val ? fKillMode.set() : fKillMode.reset(); }
    
    void AddParticleToBlackList(G4int pdg_code) { fBlackListPdg.insert(pdg_code); }
    void AddProcessToBlackList(const G4String& processName) { fBlackListProcess.insert(processName); }
    
private:
    G4int GetPrimaryParentID(G4int trackID);
    void PostponeTrack(const G4Track* aTrack, G4int status);
    
private:
    BxGeneratorTTree*         fGenerator;
    BxStackingTTreeMessenger* fMessenger;
    G4bool                    fIsFirst;
    std::bitset<4>            fMode; //[0] - gamma from neutron capture, [1] - radioactive decay, [2] - muon decay, [3] - decay
    std::bitset<4>            fKillMode; //[0] - gamma from neutron capture, [1] - radioactive decay, [2] - muon decay, [3] - decay
    std::set<G4int>           fBlackListPdg;
    std::set<G4String>        fBlackListProcess;
    G4double                  fEkinMaxMuonDecay;
    
    std::map<G4int, G4int>    fTrackParentIDs; // <TrackID, ParentID>
    std::map<G4int, G4int>    fTrackPDGs;      // <TrackID, PDGEncoding>
    std::map<G4int, G4double> fTrackTimes;     // <TrackID, GlobalTime>
    
    struct MuMinusHelper {
        G4int    parentID;
        G4int    trackID;
        G4double time;
        
        void Set(G4int _parentID, G4int _trackID, G4double _time) {
            parentID = _parentID;
            trackID  = _trackID ;
            time     = _time    ;
        }
    };
    MuMinusHelper fAugerElectron;
    
    G4double fRadNucleiLifetimeThreshold;
};

#endif

