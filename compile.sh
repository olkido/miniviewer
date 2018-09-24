cd build
echo "Usage: sh compile.sh <path_to_libigl>"
echo "Path to libigl: $1"
cmake -DCMAKE_BUILD_TYPE=Release -DLIBIGL_DIR=$1 ../
make
# change this to add to path
sudo rm /usr/local/bin/miniviewer
sudo cp miniviewer /usr/local/bin/
cd ../
