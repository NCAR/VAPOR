#!/bin/bash

set -eo pipefail

pip3 install gdown
sudo mkdir -p /usr/local/VAPOR-Deps
sudo chmod 777 /usr/local/VAPOR-Deps
cd /usr/local/VAPOR-Deps
gdown https://drive.google.com/uc?id=1Q-IXlP_OgZSXsWKmT-smyrW9xnR-dUfg
cd /usr/local/VAPOR-Deps
tar xf 2019-Aug-Darwin.tar.xz -C /usr/local/VAPOR-Deps
chmod -R 777 /usr/local/VAPOR-Deps
brew install cmake
brew install llvm
