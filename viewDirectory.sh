#!/bin/bash
# Runs miniviewer on all mesh files of a provided format (.obj, .off, .stl, etc)
# with a camera whose parameters are given in a filename
# in an input directory, and saves screenshots  in an output directory.
# Screenshots will have the same names as the meshes.
echo " "
echo "*** USAGE: sh viewDirectory.sh <input_directory> <mesh suffix> <camera_file> <output_directory> ***"
echo "*** NOTE: mesh suffix SHOULD NOT contain a dot . ***"
echo " "

num_args=$#
if [ $num_args -lt 4 ]
then
  echo "Wrong number of arguments."
  exit 1
fi

echo " "
echo " -- Input --"
in_directory=$1
echo in_directory : ["$in_directory"]
suffix=$2
echo suffix : ["$suffix"]
camera_file=$3
echo camera_file : ["$camera_file"]
out_directory=$4
echo out_directory : ["$out_directory"]
remaining_args="${@:5}"
echo remaining_args : ["$remaining_args"]
echo " -----------"
echo " "

if [ $num_args -lt 4 ]
then
  echo "Wrong number of arguments."
fi
for meshfile in $in_directory/*.$suffix
do
  # Isolate the filepath from the mesh file name.
  # Find last occurence of /
  # https://www.thegeekstuff.com/2010/07/bash-string-manipulation/
  filepath=${meshfile%/*}
  # Instead: get length of file path
  filelen=${#filepath}
  # Starting point for file name in full file will be length of filepath +1 for /
  # +1 because it is 1-based indexing
  s=$(expr $filelen + 2)
  # Then, isolate the part of the filepath from s to the last character
  # This will be the filename with the .pdf extension
  file_nopath=$(echo "$meshfile"| cut -c $s-)
  # Get filename without the suffix
  filelen=${#file_nopath}
  suffixlen=${#suffix}
  # Find the last character before the .pdf extension
  # 4 is the length of the string ".pdf"
  e=$(expr $filelen - $suffixlen - 1)
  # Then, isolate the part of the filepath from start to that last character
  file_noext=$(echo "$file_nopath"| cut -c 1-$e)

  # name of .png screenshot
  pngfile="$out_directory"/$file_noext.png
  echo ["$meshfile"] : ["$pngfile"]

  ./build/miniviewer  -c "$camera_file" --png "$pngfile"  $remaining_args "$meshfile"
  # # start and kill process
  # # https://stackoverflow.com/questions/22867130/bash-script-to-start-process-wait-random-kill-process-restart/22867414
  # # not using it, instead, kill from inside the viewer
  # last_pid=$!
  # sleep 5
  # kill -KILL $last_pid
done
