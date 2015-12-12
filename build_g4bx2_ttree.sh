#!/bin/bash

offline_dir="`readlink -e $1`"

rsync -hhruP --exclude '.git' --exclude '.gitignore' --exclude 'README.md' ${BXUSERS}/viatro/BxGeneratorTTree/ ${offline_dir}/bxmc/g4bx2/

cd ${offline_dir}/bxmc/g4bx2/src
sed -i '0,/#include /s//#include "BxGeneratorTTree.hh"\n&/' ./BxPrimaryGeneratorActionMessenger.cc
sed -i '/fSelectCmd->SetCandidates/ i\candidates += " TTree";' ./BxPrimaryGeneratorActionMessenger.cc
sed -i '0,/(newValue ==/s// (newValue == "TTree") {\
    BxOutputVertex::Get()->SetGenerator(50);\
    fGeneratorPrimary->SetBxGenerator(new BxGeneratorTTree);\
    } else if &/'    ./BxPrimaryGeneratorActionMessenger.cc

cd ${offline_dir}/bxmc/g4bx2
grep "TreePlayer" ./CMakeLists.txt || sed -i "s/\${ROOT_LIBRARIES}/& TreePlayer/g" ./CMakeLists.txt
source please_source_me

cd ${offline_dir}/bxmc/g4bx2/build
cmake -DGeant4_DIR=${BXSOFTWARE}/geant4/geant4.10.00.p02/ ../. &&
make -j 4

