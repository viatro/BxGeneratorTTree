#!/bin/bash

offline_dir="`readlink -e $1`"
g4bx2_dir=${offline_dir}/bxmc/g4bx2

rsync -hhruP --exclude '.git' --exclude '.gitignore' --exclude 'README.md' ${BXUSERS}/viatro/BxGeneratorTTree/ ${g4bx2_dir}/

cd ${g4bx2_dir}/src
if ! grep -q "BxGeneratorTTree" BxPrimaryGeneratorActionMessenger.cc; then
	sed -i '0,/#include /s//#include "BxGeneratorTTree.hh"\n&/' BxPrimaryGeneratorActionMessenger.cc 
	sed -i '/fSelectCmd->SetCandidates/ i\candidates += " TTree";' BxPrimaryGeneratorActionMessenger.cc
	sed -i '0,/(newValue ==/s// (newValue == "TTree") {\n\t\tBxOutputVertex::Get()->SetGenerator(50);\n\t\tfGeneratorPrimary->SetBxGenerator(new BxGeneratorTTree);\n\t} else if &/' BxPrimaryGeneratorActionMessenger.cc
fi
grep -q "if (evt->IsAborted())  return;" BxEventAction.cc || sed -i "/BxEventAction::EndOfEventAction/ a\  if (evt->IsAborted())  return;" BxEventAction.cc

cd ${g4bx2_dir}
grep -q "TreePlayer" CMakeLists.txt || sed -i "s/\${ROOT_LIBRARIES}/& TreePlayer/g" CMakeLists.txt
grep -q "ttree.mac" CMakeLists.txt || sed -i '/gun.mac/ a\ttree.mac' CMakeLists.txt
source please_source_me

cd ${g4bx2_dir}/build
cmake -DGeant4_DIR=${BXSOFTWARE}/geant4/geant4.10.00.p02/ ../. &&
make

