cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLIBIGL_DIR=$1 ../
make
# change this to add to path
sudo rm /usr/local/bin/miniviewer
sudo cp miniviewer /usr/local/bin/
cd ../
