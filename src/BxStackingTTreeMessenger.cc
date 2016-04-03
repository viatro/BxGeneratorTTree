// -------------------------------------------------- //
/**
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "BxStackingTTreeMessenger.hh"
#include "BxStackingTTree.hh"
#include "BxLogger.hh"

#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"

BxStackingTTreeMessenger::BxStackingTTreeMessenger(BxStackingTTree* stack) : fStacking(stack) {
    fDirectory = new G4UIdirectory("/bx/stack/ttree/");
    fDirectory->SetGuidance("Control of Stacking for TTree event generator");
    
    fModeCmd = new G4UIcmdWithAString("/bx/stack/ttree/mode", this);
    fModeCmd->SetGuidance("");
    
    BxLog(warning) << "BxStackingTTreeMessenger built" << endlog;
}

BxStackingTTreeMessenger::~BxStackingTTreeMessenger() {
    delete fDirectory;
    delete fModeCmd;
}

void BxStackingTTreeMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue) {
    if (cmd == fModeCmd) {
        newValue.toLower();
        if (newValue == "neutron")  fStacking->SetMode(0, true);
        BxLog(routine) << "TTree Stacking \"Mode\" UI-cmd is \"" << newValue << "\"" << endlog;
    }
}