// -------------------------------------------------- //
/** 
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#include "BxGeneratorTTreeMessenger.hh"
#include "BxGeneratorTTree.hh"
#include "BxLogger.hh"

#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4AnalysisUtilities.hh"

#include <sstream>

BxGeneratorTTreeMessenger::BxGeneratorTTreeMessenger(BxGeneratorTTree* gen)
: fGenerator(gen)
{
    fDirectory = new G4UIdirectory("/bx/generator/ttree/");
    fDirectory->SetGuidance("Control of TTree event generator");
    
    fAddTreeCmd = new G4UIcmdWithAString("/bx/generator/ttree/add_tree", this);
    fAddTreeCmd->SetGuidance("Add ROOT TTree (TTree name and path to ROOT file with extension");
    
    fSetAliasCmd = new G4UIcmdWithAString("/bx/generator/ttree/set_alias", this);
    fSetAliasCmd->SetGuidance("Set alias for TTree formula (see TTree::SetAlias)");
    
    fFirstEntryCmd = new G4UIcmdWithAnInteger("/bx/generator/ttree/first_entry", this);
    fFirstEntryCmd->SetGuidance("Set first TTree(Chain) entry number to be processed");
    fFirstEntryCmd->SetGuidance("Default:    0");
    
    fNEntriesCmd = new G4UIcmdWithAnInteger("/bx/generator/ttree/n_entries", this);
    fNEntriesCmd->SetGuidance("Set number of TTree(Chain) entries to be processed");
    fNEntriesCmd->SetGuidance("Default:    all");
    
    fLogPrimariesInfoCmd = new G4UIcmdWithABool("/bx/generator/ttree/log_primaries_info", this);
    fLogPrimariesInfoCmd->SetGuidance("Log primaries info (only in '/bxlog trace' mode)");
    fLogPrimariesInfoCmd->SetGuidance("Default:    1");
    
    fSavePrimariesInfoCmd = new G4UIcmdWithABool("/bx/generator/ttree/save_primaries_info", this);
    fSavePrimariesInfoCmd->SetGuidance("Save primaries mctruth info to output file");
    fSavePrimariesInfoCmd->SetGuidance("Default:    1");
    
    fEventIdCmd = new G4UIcmdWithAString("/bx/generator/ttree/event_id", this);
    fEventIdCmd->SetGuidance("Event id");
    
    fEventSkipCmd = new G4UIcmdWithAString("/bx/generator/ttree/event_skip_if", this);
    fEventSkipCmd->SetGuidance("Event skipping flag");
    fEventSkipCmd->SetGuidance("Default:    0");
    
    fEventRotateIsoCmd = new G4UIcmdWithAString("/bx/generator/ttree/event_rotate_iso", this);
    fEventRotateIsoCmd->SetGuidance("Event isotropic rotation flag");
    fEventRotateIsoCmd->SetGuidance("Default:    0");
    
    fAddSubEventCmd = new G4UIcmdWithoutParameter("/bx/generator/ttree/add_sub_event", this);
    fAddSubEventCmd->SetGuidance("Add another set of particles from the same entry");
    
    fSubEventRotateIsoCmd = new G4UIcmdWithAString("/bx/generator/ttree/sub_event_rotate_iso", this);
    fSubEventRotateIsoCmd->SetGuidance("Sub-event isotropic rotation flag");
    fSubEventRotateIsoCmd->SetGuidance("Default:    0");
    
    fNParticlesCmd = new G4UIcmdWithAString("/bx/generator/ttree/n_particles", this);
    fNParticlesCmd->SetGuidance("Number of particles per Tree entry");
    fNParticlesCmd->SetGuidance("Default:    1");
    
    fParticleSkipCmd = new G4UIcmdWithAString("/bx/generator/ttree/particle_skip_if", this);
    fParticleSkipCmd->SetGuidance("Particle skipping flag");
    fParticleSkipCmd->SetGuidance("Default:    0");
    
    fParticleRotateIsoCmd = new G4UIcmdWithAString("/bx/generator/ttree/particle_rotate_iso", this);
    fParticleRotateIsoCmd->SetGuidance("Particle isotropic rotation flag");
    fParticleRotateIsoCmd->SetGuidance("Default:    0");
    
    fPdgCmd = new G4UIcmdWithAString("/bx/generator/ttree/pdg", this);
    fPdgCmd->SetGuidance("Particle PDG code");
    fPdgCmd->SetGuidance("Default:    22");
    
    fEnergyCmd = new G4UIcmdWithAString("/bx/generator/ttree/energy", this);
    fEnergyCmd->SetGuidance("Particle kinetic energy");
    fEnergyCmd->SetGuidance("Default:    1 MeV");
    
    fMomentumCmd = new G4UIcmdWithAString("/bx/generator/ttree/momentum", this);
    fMomentumCmd->SetGuidance("Particle momentum/direction");
    fMomentumCmd->SetGuidance("Default:    0 0 1 MeV");
    
    fPositionCmd = new G4UIcmdWithAString("/bx/generator/ttree/position", this);
    fPositionCmd->SetGuidance("Particle coordinates");
    fPositionCmd->SetGuidance("Default:    0 0 0 m");
    
    fTimeCmd = new G4UIcmdWithAString("/bx/generator/ttree/time", this);
    fTimeCmd->SetGuidance("Particle time");
    fTimeCmd->SetGuidance("Default:    0 ns");
    
    fPolarizationCmd = new G4UIcmdWithAString("/bx/generator/ttree/polarization", this);
    fPolarizationCmd->SetGuidance("Particle polarization");
    fPolarizationCmd->SetGuidance("Default:    0 0 0");
    
    BxLog(routine) << "BxGeneratorTTreeMessenger built" << endlog;
}

BxGeneratorTTreeMessenger::~BxGeneratorTTreeMessenger() {
    delete fDirectory;
    delete fAddTreeCmd;
    delete fSetAliasCmd;
    delete fFirstEntryCmd;
    delete fNEntriesCmd;
    delete fLogPrimariesInfoCmd;
    delete fSavePrimariesInfoCmd;
    delete fEventIdCmd;
    delete fEventSkipCmd;
    delete fEventRotateIsoCmd;
    delete fSubEventRotateIsoCmd;
    delete fNParticlesCmd;
    delete fParticleSkipCmd;
    delete fParticleRotateIsoCmd;
    delete fPdgCmd;
    delete fEnergyCmd;
    delete fMomentumCmd;
    delete fPositionCmd;
    delete fTimeCmd;
    delete fPolarizationCmd;
}

void BxGeneratorTTreeMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue) {
    G4String cmdName = "/ttree/" + cmd->GetCommandName();
    if (cmd == fAddTreeCmd) {
        std::vector<G4String> tokens;
        G4Analysis::Tokenize(newValue, tokens);
        if (tokens.size() != 2) {
            LogCmd(cmdName, newValue, 0, WrongTokensNumber);
            BxLog(fatal) << "FATAL " << endlog;
        }
        fGenerator->AddTree(tokens[0], tokens[1]);
        BxLog(routine) << "BxGeneratorTTreeMessenger: added TTree \"" << tokens[0] << "\" from ROOT file \"" << tokens[1] << "\"" << endlog;
    } else if (cmd == fSetAliasCmd) {
        std::vector<G4String> tokens;
        G4Analysis::Tokenize(newValue, tokens);
        if (tokens.size() != 2) {
            LogCmd(cmdName, newValue, 0, WrongTokensNumber);
            BxLog(fatal) << "FATAL " << endlog;
        }
        fGenerator->SetAlias(tokens[0], tokens[1]);
        BxLog(routine) << "BxGeneratorTTreeMessenger: added alias \"" << tokens[0] << "\" for formula \"" << tokens[1] << "\"" << endlog;
    } else if (cmd == fFirstEntryCmd) {
        fGenerator->SetFirstEntry(fFirstEntryCmd->ConvertToInt(newValue));
	    BxLog(routine) << "BxGeneratorTTreeMessenger: first entry to be processed is " << newValue  << endlog;
    } else if (cmd == fNEntriesCmd) {
        G4int value = fNEntriesCmd->ConvertToInt(newValue);
        fGenerator->SetNEntries(value);
	    BxLog(routine) << "BxGeneratorTTreeMessenger: number of entries to be processed is " << value << (value == 0 ? ". Zero means \"all\"" : "") << endlog;
    } else if (cmd == fLogPrimariesInfoCmd) {
        G4bool value = fLogPrimariesInfoCmd->ConvertToBool(newValue);
        fGenerator->SetLogPrimariesInfo(value);
	    BxLog(routine) << "BxGeneratorTTreeMessenger: log primaries info? " << (value ? "Yes" : "No") << endlog;
    } else if (cmd == fSavePrimariesInfoCmd) {
        G4bool value = fSavePrimariesInfoCmd->ConvertToBool(newValue);
        fGenerator->SetSavePrimariesInfo(value);
	    BxLog(routine) << "BxGeneratorTTreeMessenger: save primaries mctruth info? " << (value ? "Yes" : "No") << endlog;
    } else if (cmd == fEventIdCmd) {
        fGenerator->GetEventConfigTTF()->SetEventId(newValue);
        LogCmd(cmdName, newValue, 0, Standard);
    } else if (cmd == fEventSkipCmd) {
        fGenerator->GetEventConfigTTF()->SetEventSkip(newValue);
        LogCmd(cmdName, newValue, 0, Standard);
    } else if (cmd == fEventRotateIsoCmd) {
        fGenerator->GetEventConfigTTF()->SetEventRotateIso(newValue);
        LogCmd(cmdName, newValue, 0, Standard);
    } else {
        G4int sub_event_number = fGenerator->GetEventConfigTTF()->GetSubEvents().size() - 1;
        if (cmd == fAddSubEventCmd) {
            fGenerator->GetEventConfigTTF()->AddSubEvent();
            BxLog(routine) << "Added SubEvent #" << sub_event_number + 1 << " for ConfigTTF" << endlog;
        } else if (cmd == fSubEventRotateIsoCmd) {
            fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetSubEventRotateIso(newValue);
            LogCmd(cmdName, newValue, sub_event_number, Standard);
        } else if (cmd == fNParticlesCmd) {
            fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetNParticles(newValue);
            LogCmd(cmdName, newValue, sub_event_number, Standard);
        } else if (cmd == fParticleSkipCmd) {
            fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetParticleSkip(newValue);
            LogCmd(cmdName, newValue, sub_event_number, Standard);
        } else if (cmd == fParticleRotateIsoCmd) {
            fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetParticleRotateIso(newValue);
            LogCmd(cmdName, newValue, sub_event_number, Standard);
        } else if (cmd == fPdgCmd) {
            fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPdg(newValue);
            LogCmd(cmdName, newValue, sub_event_number, Standard);
        } else {
            std::vector<G4String> tokens;
            G4Analysis::Tokenize(newValue, tokens);
            G4int ntokens = tokens.size();
            if (cmd == fEnergyCmd) {
                if (ntokens < 1 || ntokens > 2) {
                    LogCmd(cmdName, newValue, sub_event_number, WrongTokensNumber);
                    BxLog(fatal) << "FATAL " << endlog;
                } else if (ntokens == 2) {
                    if (G4UIcommand::CategoryOf(tokens[1]) != "Energy") {
                        LogCmd(cmdName, newValue, sub_event_number, WrongUnit);
                        BxLog(fatal) << "FATAL " << endlog;
                    }
                    fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetEnergyUnit(G4UIcommand::ValueOf(tokens[1]));
                }
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetEnergy(tokens[0]);
                LogCmd(cmdName, newValue, sub_event_number, Standard);
            } else if (cmd == fMomentumCmd) {
                if (ntokens < 3 || ntokens > 4) {
                    LogCmd(cmdName, newValue, sub_event_number, WrongTokensNumber);
                    BxLog(fatal) << "FATAL " << endlog;
                } else if (ntokens == 4) {
                    if (G4UIcommand::CategoryOf(tokens[3]) != "Energy") {
                        LogCmd(cmdName, newValue, sub_event_number, WrongUnit);
                        BxLog(fatal) << "FATAL " << endlog;
                    }
                    fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetMomentumUnit(G4UIcommand::ValueOf(tokens[3]));
                }
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetMomentumX(tokens[0]);
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetMomentumY(tokens[1]);
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetMomentumZ(tokens[2]);
                LogCmd(cmdName, newValue, sub_event_number, Standard);
            } else if (cmd == fPositionCmd) {
                if (ntokens < 3 || ntokens > 4) {
                    LogCmd(cmdName, newValue, sub_event_number, WrongTokensNumber);
                    BxLog(fatal) << "FATAL " << endlog;
                } else if (ntokens == 4) {
                    if (G4UIcommand::CategoryOf(tokens[3]) != "Length") {
                        LogCmd(cmdName, newValue, sub_event_number, WrongUnit);
                        BxLog(fatal) << "FATAL " << endlog;
                    }
                    fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPositionUnit(G4UIcommand::ValueOf(tokens[3]));
                }
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPositionX(tokens[0]);
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPositionY(tokens[1]);
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPositionZ(tokens[2]);
                LogCmd(cmdName, newValue, sub_event_number, Standard);
            } else if (cmd == fTimeCmd) {
                if (ntokens < 1 || ntokens > 2) {
                    LogCmd(cmdName, newValue, sub_event_number, WrongTokensNumber);
                    BxLog(fatal) << "FATAL " << endlog;
                } else if (ntokens == 2) {
                    if (G4UIcommand::CategoryOf(tokens[1]) != "Time") {
                        LogCmd(cmdName, newValue, sub_event_number, WrongUnit);
                        BxLog(fatal) << "FATAL " << endlog;
                    }
                    fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetTimeUnit(G4UIcommand::ValueOf(tokens[1]));
                }
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetTime(tokens[0]);
                LogCmd(cmdName, newValue, sub_event_number, Standard);
            } else if (cmd == fPolarizationCmd) {
                if (ntokens != 3) {
                    LogCmd(cmdName, newValue, sub_event_number, WrongTokensNumber);
                    BxLog(fatal) << "FATAL " << endlog;
                }
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPolarizationX(tokens[0]);
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPolarizationY(tokens[1]);
                fGenerator->GetEventConfigTTF()->GetSubEvents().back().SetPolarizationZ(tokens[2]);
                LogCmd(cmdName, newValue, sub_event_number, Standard);
            }
        }
    }
}

void BxGeneratorTTreeMessenger::LogCmd(const G4String& cmdName, const G4String& cmdValue, G4int subEventNumber, LogMode mode) {
    std::stringstream ss;
    ss << " ";
    if (subEventNumber > 0) ss << "subEvent #" << subEventNumber << ": ";
    if (mode == Standard) BxLog(routine) << ss.str() << cmdName << "  command is set to  \"" << cmdValue << "\"" << endlog;
    else if (mode == WrongTokensNumber) BxLog(error) << ss.str() << cmdName << "  command has wrong tokens number!" << endlog;
    else if (mode == WrongUnit) BxLog(error) << ss.str() << cmdName << "  command has wrong unit name or category!" << endlog;
}