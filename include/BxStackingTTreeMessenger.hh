// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#ifndef BxStackingTTreeMessenger_h
#define BxStackingTTreeMessenger_h 1

#include "G4UImessenger.hh"

class BxStackingTTree;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;

///Messenger for BxStackingTTree
class BxStackingTTreeMessenger: public G4UImessenger {

    public:
        ///constructor
        BxStackingTTreeMessenger(BxStackingTTree*);
        ///destructor
        virtual ~BxStackingTTreeMessenger();
        
        void SetNewValue(G4UIcommand*, G4String);
            
    private:
        BxStackingTTree*              fStacking;
        G4UIdirectory*       	      fDirectory;
        G4UIcmdWithAString*  	      fModeCmd;
};

#endif