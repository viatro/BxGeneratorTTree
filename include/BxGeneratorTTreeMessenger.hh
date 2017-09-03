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
class G4UIcmdWithABool;
class G4UIcmdWithoutParameter;

///Messenger for BxGeneratorTTree
class BxGeneratorTTreeMessenger: public G4UImessenger {

    public:
        /// Constructor
        BxGeneratorTTreeMessenger(BxGeneratorTTree*);
        /// Destructor
        virtual ~BxGeneratorTTreeMessenger();
        
        void SetNewValue(G4UIcommand*, G4String);
            
    private:
        //TODO: with C++11 change to enum class
        enum LogMode { Standard, WrongTokensNumber, WrongUnit };
        
        void LogCmd(const G4String&, const G4String&, G4int, LogMode);
        
        BxGeneratorTTree*        fGenerator;
        G4UIdirectory*       	 fDirectory;
        G4UIcmdWithAString*  	 fAddTreeCmd;
		G4UIcmdWithAString*  	 fSetAliasCmd;
        G4UIcmdWithAnInteger*	 fFirstEntryCmd;
        G4UIcmdWithAnInteger*	 fNEntriesCmd;
        G4UIcmdWithABool*	     fLogPrimariesInfoCmd;
        G4UIcmdWithABool*	     fSavePrimariesInfoCmd;
        
        G4UIcmdWithAString*      fEventIdCmd;
        G4UIcmdWithAString*      fEventSkipCmd;
        G4UIcmdWithAString*      fEventRotateIsoCmd;
        
        G4UIcmdWithoutParameter* fAddSubEventCmd;
        
        G4UIcmdWithAString*      fSubEventRotateIsoCmd;
        G4UIcmdWithAString*      fNParticlesCmd;
        G4UIcmdWithAString*      fParticleSkipCmd;
        G4UIcmdWithAString*      fParticleRotateIsoCmd;
        G4UIcmdWithAString*      fPdgCmd;
        G4UIcmdWithAString*      fEnergyCmd;
        G4UIcmdWithAString*      fMomentumCmd;
        G4UIcmdWithAString*      fPositionCmd;
        G4UIcmdWithAString*      fTimeCmd;
        G4UIcmdWithAString*      fPolarizationCmd;
};

#endif