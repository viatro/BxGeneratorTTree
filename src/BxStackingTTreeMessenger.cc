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

BxStackingTTreeMessenger::BxStackingTTreeMessenger(BxStackingTTree* stack) : fStacking(stack) {
    fDirectory = new G4UIdirectory("/bx/stack/ttree/");
    fDirectory->SetGuidance("Control of Stacking for TTree event generator");
    
    fModeCmd = new G4UIcmdWithAString("/bx/stack/ttree/mode", this);
    fModeCmd->SetGuidance("");
    
    fTimeCutCmd = new G4UIcmdWithADoubleAndUnit("/bx/stack/ttree/time_cut", this);
    fTimeCutCmd->SetDefaultValue(0.);
    fTimeCutCmd->SetUnitCategory("Time");
    fTimeCutCmd->SetDefaultUnit("ns");
    
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
        else if (newValue == "ra_decay")   fStacking->SetMode(1, true);
        else if (newValue == "muon")   fStacking->SetMode(2, true);
        BxLog(routine) << "TTree Stacking \"Mode\" UI-cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fTimeCutCmd) {
        fStacking->SetTimeCut(fTimeCutCmd->ConvertToDimensionedDouble(newValue));
    }
}