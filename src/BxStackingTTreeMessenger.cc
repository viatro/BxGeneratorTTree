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
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4AnalysisUtilities.hh"

BxStackingTTreeMessenger::BxStackingTTreeMessenger(BxStackingTTree* stack) : fStacking(stack) {
    fDirectory = new G4UIdirectory("/bx/stack/ttree/");
    fDirectory->SetGuidance("Control of Stacking for TTree event generator");
    
    fModeCmd = new G4UIcmdWithAString("/bx/stack/ttree/mode", this);
    fModeCmd->SetGuidance("");
    
    BxLog(routine) << "BxStackingTTreeMessenger built" << endlog;
}

BxStackingTTreeMessenger::~BxStackingTTreeMessenger() {
    delete fDirectory;
    delete fModeCmd;
}

void BxStackingTTreeMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue) {
    if (cmd == fModeCmd) {
        newValue.toLower();
        std::vector<G4String> parameters;
        G4Analysis::Tokenize(newValue, parameters);
        
        if (parameters[0] == "all") fStacking->SetMode(true);
        else if (parameters[0] == "none" || parameters[0] == "") fStacking->SetMode(false);
        else {
            for (size_t i = 0; i < parameters.size(); ++i) {
                if (parameters[i] == "neutron") fStacking->SetMode(0, true);
                else if (parameters[i] == "ra_decay") fStacking->SetMode(1, true);
                else if (parameters[i] == "muon") fStacking->SetMode(2, true);
            }
        }
        BxLog(routine) << "TTree Stacking \"Mode\" UI-cmd is \"" << newValue << "\"" << endlog;
    }
}