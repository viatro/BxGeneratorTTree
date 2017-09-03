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
#include "G4AnalysisUtilities.hh"
#include "G4ParticleTable.hh"
#include "G4IonTable.hh"
#include "G4ParticleDefinition.hh"

#include <cstdlib>

BxStackingTTreeMessenger::BxStackingTTreeMessenger(BxStackingTTree* stack)
: fStacking(stack)
{
    fDirectory = new G4UIdirectory("/bx/stack/ttree/");
    fDirectory->SetGuidance("Control of Stacking for TTree event generator");
    
    fModeCmd = new G4UIcmdWithAString("/bx/stack/ttree/mode", this);
    fModeCmd->SetGuidance("Set stacking mode");
    
    fBlackListCmd = new G4UIcmdWithAString("/bx/stack/ttree/black_list", this);
    fBlackListCmd->SetGuidance("Kill particles by their pdg codes or names");
    
    BxLog(routine) << "BxStackingTTreeMessenger built" << endlog;
}

BxStackingTTreeMessenger::~BxStackingTTreeMessenger() {
    delete fDirectory;
    delete fModeCmd;
}

void BxStackingTTreeMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue) {
    G4String cmdName = "/ttree/" + cmd->GetCommandName();
    newValue.toLower();
    std::vector<G4String> tokens;
    G4Analysis::Tokenize(newValue, tokens);
    if (cmd == fModeCmd) {
        BxLog(routine) << cmdName << "  command is set to  \"" << newValue << "\"" << endlog;
        if (tokens[0] == "all") fStacking->SetMode(true);
        else if (tokens[0] == "none" || tokens[0] == "") fStacking->SetMode(false);
        else {
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i] == "neutron" || tokens[i] == "1") fStacking->SetMode(0, true);
                else if (tokens[i] == "ra_decay" || tokens[i] == "2") fStacking->SetMode(1, true);
                else if (tokens[i] == "muon" || tokens[i] == "3") fStacking->SetMode(2, true);
                else if (tokens[i] == "decay" || tokens[i] == "4") fStacking->SetMode(3, true);
                else BxLog(warning) << cmdName << ":  unknown parameter \"" << tokens[i] << "\" ! Skipping." << endlog;
            }
        }
    } else if (cmd == fBlackListCmd) {
        BxLog(routine) << cmdName << "  command is set to  \"" << newValue << "\"" << endlog;
        for (size_t i = 0; i < tokens.size(); ++i) {
            char* end;
            G4int pdg_code = strtol(tokens[i].data(), &end, 10);
            G4bool isInteger = (!*end) ? true : false;
            if (isInteger) {
                if (!G4ParticleTable::GetParticleTable()->FindParticle(pdg_code)) {
                    if (!G4IonTable::GetIonTable()->GetIon(pdg_code)) {
                        BxLog(warning) << cmdName << ":  unknown particle with PDG code \"" << pdg_code << "\" ! Skipping." << endlog;
                        continue;
                    }
                }
                fStacking->AddParticleToBlackList(pdg_code);
            } else {
                G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(tokens[i]);
                if (!particle) {
                    BxLog(warning) << cmdName << ":  unknown particle with name \"" << tokens[i] << "\" ! Skipping." << endlog;
                    continue;
                }
                fStacking->AddParticleToBlackList(particle->GetPDGEncoding());
            }
        }
    }
}
