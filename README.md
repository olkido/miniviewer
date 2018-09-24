# miniviewer

A small mesh viewer based on [libigl](http://libigl.github.io/libigl/).

## Compiling
Assuming: A Unix system with CMake >= 3.2 . Tested with CMake 3.12 .

1. Get libigl from [the github repo](https://github.com/libigl/libigl/)
```
git clone --recursive https://github.com/libigl/libigl
```

2. Clone this repo
```
git clone https://github.com/olkido/miniviewer
```

3. Compile using cmake:
```
cd <path_to_miniviewer_repo>
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DLIBIGL_DIR=<path_to_libigl> ../
make
```

## Usage
Open and show a mesh using `./miniviewer <path_to_mesh_file>`

### Supported mesh formats
Anything that `igl::read_triangle_mesh` supports, among which: .obj, .off, .stl, .mesh, .ply, .wrl .

### Setting the scene (=camera)
The camera parameters (=scene) can be passed to the viewer via an .xml file.  Run `./miniviewer --camera <path_to_camera_xml_file> <path_to_mesh_file>` (`-c` can be also used intead of `--camera` ).

The viewer saves the camera parameters in the current directory in an .xml file called camera.xml. This happens whenever something changes in the scene, eg. the mesh moves, or a viewing option is changed. Saving this file and reusing it can be used for controlling the camera.


### Options and Overlays
Run `./miniviewer` to show some options.

Hit 'm' to show mesh size (#vertices and #faces). Also accessible via the 'Show Mesh Stats' button in the viewer UI.

Hit 'c' to show camera parameters. Repeateadly hitting 'c' will show various camera parameters.
Also accessible via the 'Show Scene' viewer UI option.

Further keyboard shortcuts and viewing options are accessible via the standard libigl viewer menu. Some keyboard options are plotted on the terminal upon running the viewer.

The viewer can display several overlays on top of the mesh, including per-vertex / per-face colors, scalar fields (will be converted to colors), lines and per-face vector fields. Some of this remains untested since the last version:)
