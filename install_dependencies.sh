mkdir -p $HOME/.cache && cd $HOME/.cache
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.7.1/protobuf-cpp-3.7.1.tar.gz
tar -xvf protobuf-cpp-3.7.1.tar.gz
cd protobuf-3.7.1
./configure
make -j $(nproc)
make check
sudo make install
sudo ldconfig
cd $HOME/.cache
# Use another method to install libzmq
# echo "deb http://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/Debian_9.0/ ./" >> /etc/apt/sources.list
# wget https://download.opensuse.org/repositories/network:/messaging:/zeromq:/release-stable/Debian_9.0/Release.key -O- | sudo apt-key add
# sudo apt-get install libzmq3-dev
wget https://github.com/zeromq/libzmq/releases/download/v4.3.1/zeromq-4.3.1.tar.gz
tar -xvf zeromq-4.3.1.tar.gz
cd zeromq-4.3.1
./autogen.sh && ./configure && make -j $(nproc)
make check && sudo make install && sudo ldconfig