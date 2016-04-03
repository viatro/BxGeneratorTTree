// -------------------------------------------------- //
/** 
 * AUTHOR: V. Atroshchenko
 * CONTACT: victor.atroshchenko@lngs.infn.it
*/
// -------------------------------------------------- //

#ifndef BxGeneratorTTreeMessenger_h
#define BxGeneratorTTreeMessenger_h 1

#include "G4UImessenger.hh"

class BxGeneratorTTree;
class G4UIcommand;
class G4UIdirectory;
class G4UIcmdWithAnInteger;
class G4UIcmdWithAString;

///Messenger for BxGeneratorTTree
class BxGeneratorTTreeMessenger: public G4UImessenger {

    public:
        ///constructor
        BxGeneratorTTreeMessenger(BxGeneratorTTree*);
        ///destructor
        virtual ~BxGeneratorTTreeMessenger();
        
        void SetNewValue(G4UIcommand*, G4String);
            
    private:
        BxGeneratorTTree*       fGenerator;
        G4UIdirectory*       	fDirectory;
        G4UIcmdWithAString*  	fInputFileNameCmd;
		G4UIcmdWithAString*  	fTreeNameCmd;
        G4UIcmdWithAnInteger*	fFirstEntryCmd;
        G4UIcmdWithAnInteger*	fNEntriesCmd;
        
        G4UIdirectory*          fVarDirectoryCmd;
        G4UIcmdWithAString*     fVarEventIdCmd;
        G4UIcmdWithAString*     fVarEventSkipCmd;
        G4UIcmdWithAString*     fVarParticleSkipCmd;
        G4UIcmdWithAString*     fVarNParticlesCmd;
        G4UIcmdWithAString*     fVarSplitCmd;
        G4UIcmdWithAString*     fVarRotateIsoCmd;
        G4UIcmdWithAString*     fVarPdgCmd;
        G4UIcmdWithAString*     fVarEkinCmd;
        G4UIcmdWithAString*     fVarMomentumCmd;
        G4UIcmdWithAString*     fVarPositionCmd;
        G4UIcmdWithAString*     fVarTimeCmd;
        G4UIcmdWithAString*     fVarPolarizationCmd;
};

#endif