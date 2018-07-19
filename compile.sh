cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLIBIGL_DIR=/Users/${USER}/Dropbox/Work/code/libigl/ ../
make
# change this to add to path
sudo rm /usr/local/bin/miniviewer
sudo cp miniviewer /usr/local/bin/
cd ../
