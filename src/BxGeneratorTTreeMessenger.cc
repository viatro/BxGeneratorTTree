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

BxGeneratorTTreeMessenger::BxGeneratorTTreeMessenger(BxGeneratorTTree* gen)
: fGenerator(gen)
{
    fDirectory = new G4UIdirectory("/bx/generator/ttree/");
    fDirectory->SetGuidance("Control of TTree event generator");
    
    fInputFileNameCmd = new G4UIcmdWithAString("/bx/generator/ttree/add_input_file", this);
    fInputFileNameCmd->SetGuidance("Add input ROOT file (name with extension)");
    
    fTreeNameCmd = new G4UIcmdWithAString("/bx/generator/ttree/tree_name", this);
    fTreeNameCmd->SetGuidance("Set Tree(Chain) name");
    
    fFirstEntryCmd = new G4UIcmdWithAnInteger("/bx/generator/ttree/first_entry", this);
    fFirstEntryCmd->SetGuidance("Set first TTree(Chain) entry number to be processed");
    fFirstEntryCmd->SetGuidance("Default:    0");
    
    fNEntriesCmd = new G4UIcmdWithAnInteger("/bx/generator/ttree/n_entries", this);
    fNEntriesCmd->SetGuidance("Set number of TTree(Chain) entries to be processed");
    fNEntriesCmd->SetGuidance("Default:    all");
    
    fVarDirectoryCmd = new G4UIdirectory("/bx/generator/ttree/var/");
    fVarDirectoryCmd->SetGuidance("Control of variables");
    
    fVarEventIdCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/event_id", this);
    fVarEventIdCmd->SetGuidance("");
    
    fVarEventSkipCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/event_skip_if", this);
    fVarEventSkipCmd->SetGuidance("");
    fVarEventSkipCmd->SetGuidance("Default:    0");
    
    fVarParticleSkipCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/particle_skip_if", this);
    fVarParticleSkipCmd->SetGuidance("");
    fVarParticleSkipCmd->SetGuidance("Default:    0");
    
    fVarNParticlesCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/n_particles", this);
    fVarNParticlesCmd->SetGuidance("Number of particles per Tree entry");
    fVarNParticlesCmd->SetGuidance("Default:    1");
    
    fVarSplitCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/split", this);
    fVarSplitCmd->SetGuidance("");
    fVarSplitCmd->SetGuidance("Default:    0");
    
    fVarRotateIsoCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/rotate_iso", this);
    fVarRotateIsoCmd->SetGuidance("");
    fVarRotateIsoCmd->SetGuidance("Default:    0");
    
    fVarPdgCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/pdg", this);
    fVarPdgCmd->SetGuidance("PDG code of particle");
    fVarPdgCmd->SetGuidance("Default:    22");
    
    fVarEkinCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/Ekin", this);
    fVarEkinCmd->SetGuidance("Kinetic energy of particle");
    fVarEkinCmd->SetGuidance("Default:    1 MeV");
    
    fVarMomentumCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/momentum", this);
    fVarMomentumCmd->SetGuidance("Momentum/direction of particle");
    fVarMomentumCmd->SetGuidance("Default:    0 0 1 MeV");
    
    fVarPositionCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/position", this);
    fVarPositionCmd->SetGuidance("");
    fVarPositionCmd->SetGuidance("Default:    0 0 0 m");
    
    fVarTimeCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/time", this);
    fVarTimeCmd->SetGuidance("");
    fVarTimeCmd->SetGuidance("Default:    0 ns");
    
    fVarPolarizationCmd = new G4UIcmdWithAString("/bx/generator/ttree/var/polz", this);
    fVarPolarizationCmd->SetGuidance("Polarization of particle");
    fVarPolarizationCmd->SetGuidance("Default:    0 0 0");
    
    BxLog(routine) << "BxGeneratorTTreeMessenger built" << endlog;
}

BxGeneratorTTreeMessenger::~BxGeneratorTTreeMessenger() {
    delete fDirectory;
    delete fInputFileNameCmd;
    delete fTreeNameCmd;
    delete fFirstEntryCmd;
    delete fNEntriesCmd;
    delete fVarDirectoryCmd;
    delete fVarEventIdCmd;
    delete fVarEventSkipCmd;
    delete fVarParticleSkipCmd;
    delete fVarNParticlesCmd;
    delete fVarSplitCmd;
    delete fVarRotateIsoCmd;
    delete fVarPdgCmd;
    delete fVarEkinCmd;
    delete fVarMomentumCmd;
    delete fVarPositionCmd;
    delete fVarTimeCmd;
    delete fVarPolarizationCmd;
}

void BxGeneratorTTreeMessenger::SetNewValue(G4UIcommand* cmd, G4String newValue) {
    if (cmd == fInputFileNameCmd) {
        fGenerator->AddInputFileName(newValue);
        BxLog(routine) << "Added input ROOT file \"" << newValue << "\"" << endlog;
    } else if (cmd == fTreeNameCmd) {
        fGenerator->SetTreeName(newValue);
        BxLog(routine) << "Tree(Chain) name is \"" << newValue << "\"" << endlog;
    } else if (cmd == fFirstEntryCmd) {
        fGenerator->SetFirstEntry(fFirstEntryCmd->ConvertToInt(newValue));
	    BxLog(routine) << "First entry to be processed is " << newValue  << endlog;
    } else if (cmd == fNEntriesCmd) {
        fGenerator->SetNEntries(fNEntriesCmd->ConvertToInt(newValue));
	    BxLog(routine) << "Number of entries to be processed is " << newValue << (fNEntriesCmd->ConvertToInt(newValue) == 0 ? ". Zero means \"all\"" : "") << endlog;
    } else if (cmd == fVarEventIdCmd) {
        fGenerator->SetVarStringEventId(newValue);
        BxLog(routine) << "Event Id UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarEventSkipCmd) {
        fGenerator->SetVarStringEventSkip(newValue);
        BxLog(routine) << "Event skip UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarParticleSkipCmd) {
        fGenerator->SetVarStringParticleSkip(newValue);
        BxLog(routine) << "Particle skip UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarNParticlesCmd) {
        fGenerator->SetVarStringNParticles(newValue);
        BxLog(routine) << "Number of particles UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarSplitCmd) {
        fGenerator->SetVarStringSplit(newValue);
        BxLog(routine) << "Split UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarRotateIsoCmd) {
        fGenerator->SetVarStringRotateIso(newValue);
        BxLog(routine) << "RotateIso UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarPdgCmd) {
        fGenerator->SetVarStringPdg(newValue);
        BxLog(routine) << "PDG UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarEkinCmd) {
        fGenerator->SetVarStringEkin(newValue);
        BxLog(routine) << "Kinetic energy UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarMomentumCmd) {
        fGenerator->SetVarStringMomentum(newValue);
        BxLog(routine) << "Momentum/direction UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarPositionCmd) {
        fGenerator->SetVarStringPosition(newValue);
        BxLog(routine) << "Position UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarTimeCmd) {
        fGenerator->SetVarStringTime(newValue);
        BxLog(routine) << "Time UI-Cmd is \"" << newValue << "\"" << endlog;
    } else if (cmd == fVarPolarizationCmd) {
        fGenerator->SetVarStringPolarization(newValue);
        BxLog(routine) << "Polarization UI-Cmd is \"" << newValue << "\"" << endlog;
    }
}
