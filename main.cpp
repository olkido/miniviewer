#include <igl/is_file.h>
#include <igl/list_to_matrix.h>
#include <igl/matlab_format.h>
#include <igl/readWRL.h>
#include <igl/readOFF.h>
#include <igl/readOBJ.h>
#include <igl/read_triangle_mesh.h>
#include <igl/pathinfo.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>
#include <igl/parula.h>
#include <igl/barycenter.h>
#include <igl/bounding_box_diagonal.h>
#include <igl/xml/serialize_xml.h>
#include <igl/png/readPNG.h>
#include <igl/png/writePNG.h>

#include <iostream>
#include <fstream>
#include <string>

#include "optionparser.h"
#include "Arg.h"

using namespace Eigen;
using namespace std;

// path to mesh file
std::string meshfile;
// folder in which mesh file lives
std::string meshdir;
// mesh name (extracted from path without suffix and directory)
std::string meshname;
// file to which to save a screenshot (if specified)
std::string pngfile;
// boolean, whether to exit right after saving the first screenshot
bool exit_after_png;

// Mesh: vertices and faces
Eigen::MatrixXd V;
Eigen::MatrixXi F;
// Face Barycenters
MatrixXd B;

// Colors (per-vertex or per-face)
Eigen::MatrixXd C;

// Texture coordinates and corner indices
MatrixXd UV;
MatrixXi FUV, FN;

// Per-face vector field and colors
MatrixXd FVF, CFVF;

// Overlay lines and line colors
MatrixXd L,CL;

std::string xmlFile = "camera.xml";

igl::opengl::glfw::Viewer viewer;
igl::opengl::glfw::imgui::ImGuiMenu menu;
bool first_launch = true;

// Pre-allocate buffers for screenshot
int png_height=640;
int png_width=400;
Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic> imR(png_height,png_width), imG(png_height,png_width), imB(png_height,png_width), imA(png_height,png_width);

// Buffers for texture images
Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic> texture_R, texture_G, texture_B, texture_A;


// Camera parameters
struct Camera
{
  Quaternionf trackball_angle;
  Eigen::Vector3f camera_translation;
  Eigen::Vector3f camera_base_translation;
  double camera_base_zoom;
  double camera_zoom;
  bool is_valid = false;

  
  void read_from_xml(const std::string &xml_file)
  {
    Eigen::Vector4f ta;
    igl::xml::deserialize_xml(ta,"trackball_angle",xml_file);
    trackball_angle = Eigen::Quaternionf(ta);
    igl::xml::deserialize_xml(camera_translation,"camera_translation",xml_file);
    igl::xml::deserialize_xml(camera_base_translation,"camera_base_translation",xml_file);
    igl::xml::deserialize_xml(camera_base_zoom,"camera_base_zoom",xml_file);
    igl::xml::deserialize_xml(camera_zoom,"camera_zoom",xml_file);
    is_valid = true;
  }
  
  void write_to_xml(const std::string &xml_file)
  {
    // binary = false, overwrite = true
    Eigen::Vector4f ta;
    ta<< trackball_angle.x(), trackball_angle.y(), trackball_angle.z(), trackball_angle.w();
    igl::xml::serialize_xml(ta,"trackball_angle",xmlFile,false,true);
    // binary = false, overwrite = false
    igl::xml::serialize_xml(camera_translation,"camera_translation",xmlFile,false,false);
    igl::xml::serialize_xml(camera_base_translation,"camera_base_translation",xmlFile,false,false);
    igl::xml::serialize_xml(camera_base_zoom,"camera_base_zoom",xmlFile,false,false);
    igl::xml::serialize_xml(camera_zoom,"camera_zoom",xmlFile,false,false);

  }

  void load_from_viewer()
  {
    trackball_angle = viewer.core.trackball_angle;
    camera_translation = viewer.core.camera_translation;
    camera_base_translation = viewer.core.camera_base_translation;
    camera_base_zoom = viewer.core.camera_base_zoom;
    camera_zoom = viewer.core.camera_zoom;
    is_valid = true;
  }
  
  void save_to_viewer()
  {
    viewer.core.trackball_angle = trackball_angle;
    viewer.core.camera_translation = camera_translation;
    viewer.core.camera_base_translation = camera_base_translation;
    viewer.core.camera_base_zoom = camera_base_zoom;
    viewer.core.camera_zoom =  camera_zoom;
  }
  
  bool is_same_as_viewer()
  {
    return (
            trackball_angle.x() == viewer.core.trackball_angle.x() &&
            trackball_angle.y() == viewer.core.trackball_angle.y() &&
            trackball_angle.z() == viewer.core.trackball_angle.z() &&
            trackball_angle.w() == viewer.core.trackball_angle.w() &&
            camera_translation == viewer.core.camera_translation &&
            camera_base_translation == viewer.core.camera_base_translation &&
            camera_base_zoom == viewer.core.camera_base_zoom &&
            camera_zoom == viewer.core.camera_zoom);
  }
};
Camera camera;




float uv_scale = 1;
bool show_mesh_stats = false;
int show_scene = 0;

#define NUM_SCENE_OPTIONS 4; //no scene, angle, trans, zoom


template <typename DerivedV>
bool readMatrix( const char *filename, Eigen::PlainObjectBase<DerivedV>& mat)
{
  int cols = 0, rows = 0;
  
  // Read numbers from file into buffer.
  std::ifstream infile;
  infile.open(filename);
  if (!infile.is_open())
  {
    mat.setZero(0,0);
    return false;
    //    return Eigen::PlainObjectBase<DerivedV>::Zero(0, 0);
  }
  
  std::vector<double>buff;
  buff.reserve(1e8);
  while (! infile.eof())
  {
    std::string line;
    getline(infile, line);
    
    int temp_cols = 0;
    std::stringstream stream(line);
    while(! stream.eof())
    {
      double val;
      stream >> val;
      buff.push_back(val);
      temp_cols++;
      //      stream >> buff[cols*rows+temp_cols++];
    }
    if (temp_cols == 0)
      continue;
    
    if (cols == 0)
      cols = temp_cols;
    
    rows++;
  }
  
  infile.close();
  
  rows--;
  
  // Populate matrix with numbers.
  mat.resize(rows,cols);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      mat(i,j) = buff[ cols*i+j ];
  
  return true;
};

// Create a texture that hides the integer translation in the parametrization
void line_texture(Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic> &texture_R,
                  Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic> &texture_G,
                  Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic> &texture_B)
{
  unsigned size = 128;
  unsigned size2 = size/2;
  unsigned lineWidth = 3;
  texture_R.setConstant(size, size, 255);
  for (unsigned i=0; i<size; ++i)
    for (unsigned j=size2-lineWidth; j<=size2+lineWidth; ++j)
      texture_R(i,j) = 0;
  for (unsigned i=size2-lineWidth; i<=size2+lineWidth; ++i)
    for (unsigned j=0; j<size; ++j)
      texture_R(i,j) = 0;
  
  texture_G = texture_R;
  texture_B = texture_R;
}

void update_display()
{
  viewer.data().lines            = Eigen::MatrixXd (0,9);
  viewer.data().points           = Eigen::MatrixXd (0,6);
  viewer.data().labels_positions = Eigen::MatrixXd (0,3);
  viewer.data().labels_strings.clear();
  
  // show texture
  if (UV.rows()>0 && FUV.rows() == F.rows())
  {
    viewer.data().set_uv(uv_scale*UV, FUV);
    viewer.data().show_texture = true;
    
    viewer.data().set_texture(texture_R, texture_B, texture_G);
  }
  
  // show per-face vector field
  if (FVF.rows() == F.rows())
  {
    int n = FVF.cols() /3;
    for (int i=0;i <n; ++i)
      viewer.data().add_edges(B, B+FVF.block(0, i*3, FVF.rows(), 3), CFVF.block(0, i*3, FVF.rows(), 3));
    viewer.data().point_size = 5;
    viewer.data().add_points(B, Eigen::RowVector3d::Zero());
  }
  
  // show lines
  if (L.rows()>0)
  {
    int n = L.cols() /6;
    for (int i=0;i <n; ++i)
      viewer.data().add_edges(L.block(0, i*6, L.rows(), 3),
                              L.block(0, i*6+3, L.rows(), 3),
                              CL.block(0, i*3, CL.rows(), 3));
    
  }
  
  // plot mesh size
  int numV = V.rows();
  int numF = F.rows();
  if (show_mesh_stats)
  {
    std::string tag;
    tag = "#V: "+to_string(numV)+", #F: "+to_string(numF);
    viewer.data().add_label(RowVector3d::Zero(), tag);
  }
  
  // plot camera parameters
  if (show_scene>0)
  {
    RowVector3d pos = V.colwise().mean();// RowVector3d::Zero();
    std::string tag;
    if (show_scene==1)
      tag = "trackball_angle: "
      +to_string(viewer.core.trackball_angle.x())
      +", "
      +to_string(viewer.core.trackball_angle.y())
      +", "
      +to_string(viewer.core.trackball_angle.z())
      +", "
      +to_string(viewer.core.trackball_angle.w())
      ;
    else if (show_scene==2)
      tag = "camera_translation: "
      +to_string(viewer.core.camera_translation.x())
      +", "
      +to_string(viewer.core.camera_translation.y())
      +", "
      +to_string(viewer.core.camera_translation.z())
      ;
    else if (show_scene==3)
      tag = "camera_base_zoom: "
      +to_string(viewer.core.camera_base_zoom)
      +" camera_zoom: "
      +to_string(viewer.core.camera_zoom)
      ;
    viewer.data().add_label(pos, tag);
    viewer.data().add_label(pos, tag);
    
  }
}

bool key_down(igl::opengl::glfw::Viewer& viewer,
              unsigned char key, int modifier)
{
  bool ret = false;
  
  if (modifier == IGL_MOD_SUPER)
  {
  }
  else
  {
    if (key == '.')
    {
      uv_scale= std::max<int>(1,(int)uv_scale -1);
      ret = true;
    }
    if (key == '/')
    {
      uv_scale= uv_scale +1;
      ret = true;
    }
    if (key == 'z')
    {
      uv_scale= uv_scale *= 1.1;
      ret = true;
    }
    if (key == 'x')
    {
      uv_scale= uv_scale *= 0.9;
      ret = true;
    }
    if (key == 'm')
    {
      show_mesh_stats = !show_mesh_stats;
      ret = true;
    }
    if (key == 'c')
    {
      show_scene = (show_scene+1)%NUM_SCENE_OPTIONS;
      ret = true;
    }
  }
  update_display();
  return ret;
}

bool pre_draw(igl::opengl::glfw::Viewer& viewer)
{
  if (!camera.is_valid)
    return false;
  if (camera.is_same_as_viewer())
    return false;
  
  camera.save_to_viewer();

  return false;
  
}

void save_scene()
{
  // XML serialization
  camera.write_to_xml(xmlFile);
  
  // (optional) write screenshot to .png
  if (!pngfile.empty())
  {
    // Draw the scene in the buffers
    viewer.core.draw_buffer(viewer.data(),false,imR,imG,imB,imA);
    // Save it to a PNG
    igl::png::writePNG(imR,imG,imB,imA,pngfile);
    if (exit_after_png)
      exit(0);
  }
}

bool post_draw(igl::opengl::glfw::Viewer& viewer)
{
  // store and write out the camera
  camera.load_from_viewer();
  if (first_launch)
  {
    save_scene();
    first_launch = false;
  }
  return false;
}
bool mouse_up(igl::opengl::glfw::Viewer& viewer, int button, int modifier)
{
  save_scene();
  return false;
}

bool mouse_scroll(igl::opengl::glfw::Viewer& viewer, float delta_y)
{
  if(delta_y != 0)
    save_scene();
  return false;
}

template <typename DerivedV>
bool read_argument(const char* file, // filename to read
                   const std::string &default_suffix, // if filename does not exist, will try to read from file [meshname+default_suffix]
                   Eigen::PlainObjectBase<DerivedV>& out,
                   int expected_rows=-1,
                   int expected_cols=-1
                   )
{
  std::string filename;
  if (file!=0)
    filename = std::string(file);
  
  if (filename.empty())
  {
    filename = meshdir +std::string("/")+ meshname + default_suffix;
    cerr<<"parse_arguments(): No file provided. Will try to read from "<< filename<<endl;
  }
  if (!igl::is_file(filename.c_str()))
  {
    cerr<<"parse_arguments(): File" << filename <<" does not exist."<<endl;
    return false;
  }
  Eigen::PlainObjectBase<DerivedV> T;
  if (!readMatrix(filename.c_str(),T))
      return false;
  if (expected_cols!=-1 && T.cols() != expected_cols)
  {
    cerr<<"parse_arguments(): Input matrix should have "<< expected_cols <<" columns."<<endl;
    return false;
  }
  if (expected_rows!=-1 && T.rows() != expected_rows)
  {
    cerr<<"parse_arguments(): Input matrix should have "<< expected_rows <<" rows."<<endl;
    return false;
  }
  out = T;
  return true;
}

bool parse_arguments( std::vector<option::Option> &options)
{
  //Assumes mesh has been read already.
  // Throw an error if both a color and a scalar field is provided
  if (options[COLORS].count() > 0 && options[SCALAR_FIELD].count()>0)
  {
    cerr<<"parse_arguments(): Provide either colors or scalar field, not both."<<endl;
    return false;
  }
  
  // Read per-vertex / per-face colors
  if (options[COLORS].count() > 0)
  {
    cerr<<endl<<"\t"<<"Reading colors ... "<<endl;
    if (!read_argument(options[COLORS].arg, std::string(".colors"), C, -1, 3))
      return false;
    if (C.rows() != F.rows() && C.rows() != V.rows())
    {
      cerr<<"parse_arguments(): Color matrix needs to be of the mesh size (per-vertex or per-face)."<<endl;
      C = Eigen::MatrixXd::Zero(0,0);
      return false;
    }
  }
  
  // Read per-vertex / per-face scalar field
  if (options[SCALAR_FIELD].count() > 0)
  {
    cerr<<endl<<"\t"<<"Reading scalar field ... "<<endl;
    MatrixXd T;
    if (!read_argument(options[SCALAR_FIELD].arg, std::string(".scalars"), T, -1, 1))
      return false;
    if (T.rows() != F.rows() && T.rows() != V.rows())
    {
      cerr<<"parse_arguments(): Scalar field matrix needs to be of the mesh size (per-vertex or per-face)."<<endl;
      C = Eigen::MatrixXd::Zero(0,0);
      return false;
    }
    igl::parula(T, true, C);
  }

  // Read per-face vector field
  // This has to be before vector field colors for matrix size check to work
  if (options[FACE_VECTOR_FIELD].count() > 0)
  {
    cerr<<endl<<"\t"<<"Reading per-face vector field ... "<<endl;
    if (!read_argument(options[FACE_VECTOR_FIELD].arg, std::string(".fvf"), FVF, F.rows(), -1))
      return false;
    if ( FVF.cols() % 3 != 0)
    {
      cerr<<"parse_arguments(): The columns of face vector field matrix need to be dividable by 3."<<endl;
      FVF = Eigen::MatrixXd::Zero(0,0);
      return false;
    }
    // by default, vectors will be painted black
    CFVF.setZero(FVF.rows(),FVF.cols());
  }

  // Read per-face vector field colors
  // This matrix has to have the same number of columns as the vector field (3 numbers per vector for color)
  if (options[FACE_VECTOR_FIELD_COLORS].count() > 0)
  {
    // Throw an error if per-face vector field was not provided
    if (options[FACE_VECTOR_FIELD].count() == 0)
    {
      cerr<<"parse_arguments(): vector field colors provided but vector field was not."<<endl;
      return false;
    }
    cerr<<endl<<"\t"<<"Reading per-face vector field colors... "<<endl;
    if (!read_argument(options[FACE_VECTOR_FIELD_COLORS].arg, std::string(".fvfc"), CFVF, F.rows(), FVF.cols()))
      return false;
  }
  
  //Read texture coordinates
  if (options[UV_COORDS].count() > 0)
  {
    // Throw an error if texture corner indices were not provided
    if (options[UV_INDS].count() == 0)
    {
      cerr<<"parse_arguments(): uv coordinates provided but texture corner indices were not."<<endl;
      return false;
    }
    cerr<<endl<<"\t"<<"Reading uv coordinates... "<<endl;
    if (!read_argument(options[UV_COORDS].arg, std::string(".uv"), UV, -1, 2))
      return false;
  }
  
  //Read texture corner indices
  if (options[UV_INDS].count() > 0)
  {
    // Throw an error if uv coordinates were not provided
    if (options[UV_COORDS].count() == 0)
    {
      cerr<<endl<<"\t"<<"parse_arguments(): texture corner indices provided but uv coordinates were not."<<endl;
      return false;
    }
    cerr<<"Reading uv corner indices... "<<endl;
    if (!read_argument(options[UV_INDS].arg, std::string(".fuv"), FUV, F.rows(), 3))
      return false;
  }

  //Read texture image
  if (options[TEXTURE_IMAGE].count() > 0)
  {
    cerr<<endl<<"\t"<<"Reading texture image... "<<endl;
    std::string filename;
    if (options[TEXTURE_IMAGE].arg!=0)
      filename = std::string(options[TEXTURE_IMAGE].arg);
    
    if (filename.empty())
    {
      filename = meshdir +std::string("/")+ meshname + std::string(".png");
      cerr<<"parse_arguments(): No file provided. Will try to read from "<< filename<<endl;
    }

    if (!igl::is_file(filename.c_str()))
    {
      cerr<<"parse_arguments(): File" << filename <<" does not exist."<<endl;
      return false;
    }
    // Read the PNG
    igl::png::readPNG(filename, texture_R, texture_G, texture_B, texture_A);

  }

  // Read lines - 6 doubles per line (start and end point)
  // This has to be before line colors for matrix size check to work
  if (options[LINES].count() > 0)
  {
    cerr<<endl<<"\t"<<"Reading lines... "<<endl;
    if (!read_argument(options[LINES].arg, std::string(".lines"), L, -1, -1))
      return false;
    if ( L.cols() % 6 != 0)
    {
      cerr<<"parse_arguments(): The columns of line matrix need to be dividable by 6."<<endl;
      L = Eigen::MatrixXd::Zero(0,0);
      return false;
    }
    // by default, plot the lines black
    CL.setZero(L.rows(), L.cols()/2);

  }

  // Read line colors
  // This matrix has to have hald the number of columns as the line matrix (3 numbers per line for color)
  if (options[LINE_COLORS].count() > 0)
  {
    // Throw an error if lines were not provided
    if (options[LINES].count() == 0)
    {
      cerr<<"parse_arguments(): line colors provided but lines were not."<<endl;
      return false;
    }
    cerr<<endl<<"\t"<<"Reading line colors... "<<endl;
    if (!read_argument(options[LINE_COLORS].arg, std::string(".linesc"), CL, L.rows(), L.cols()/2))
      return false;
  }
  
  // Read camera parameters
  if (options[CAMERA].count() > 0)
  {
    camera.read_from_xml(options[CAMERA].arg);
  }

  // Read .png filename for screenshot
  if (options[SAVE_PNG].count() > 0)
  {
    pngfile = options[SAVE_PNG].arg;
    // Read whether to exit right after saving the first screenshot
    exit_after_png = options[EXIT_AFTER_PNG].count()>0;
  }

   
  return true;
}

int main(int argc, char *argv[])
{
  
  argc-=(argc>0); argv+=(argc>0); // skip program name argv[0] if present
  
  option::Stats  stats(usage, argc, argv);
  std::vector<option::Option> options(stats.options_max);
  std::vector<option::Option> buffer(stats.buffer_max);
  option::Parser parse(usage, argc, argv, &options[0], &buffer[0]);
  
  if (parse.error())
  {
    cerr<<"Error parsing options."<<endl;
    return 1;
  }
  
  if (options[HELP] || argc == 0)
  {
    option::printUsage(std::cout, usage);
    return 0;
  }
  
  
  if (options[UNKNOWN].count()>0)
  {
    for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next())
      std::cout << "Unknown option: " << std::string(opt->name,opt->namelen) << "\n";
    option::printUsage(std::cout, usage);
    return 0;
  }

  
  for (int i = 0; i < parse.nonOptionsCount(); ++i)
    std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";
  
  if ( parse.nonOptionsCount() < 1 )
  {
    option::printUsage(std::cout, usage);
    cerr<<"Error: mesh file not provided."<<endl;
    return 0;
  }
  
  std::string meshfile = string(parse.nonOption(0));
  
  // dirname, basename, extension and filename
  std::string base,  ext;
  igl::pathinfo(meshfile,meshdir,base,ext,meshname);
  transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  
  // read mesh
  igl::read_triangle_mesh(meshfile, V, F);
  
  
  igl::barycenter(V, F, B);
  C.setConstant(F.rows(),3,.99);

  if (!parse_arguments(options))
    exit(1);
  
  // Customize the menu
  const auto & default_menu_function = [&]()
  {
    // Draw parent menu content
    menu.draw_viewer_menu();
    
    // Add new group to *existing* viewer menut
    if (ImGui::CollapsingHeader("miniviewer", ImGuiTreeNodeFlags_DefaultOpen))
    {
      // Expose variable directly ...
      if (ImGui::InputFloat("UV scale", &uv_scale, 0, 0, 3))
        update_display();
      
      if (ImGui::Checkbox("Mesh Stats", &show_mesh_stats))
        update_display();
      
      if (ImGui::InputFloat("UV scale", &uv_scale, 0, 0, 3))
        update_display();
      
      if (ImGui::InputInt("Show Scene", &show_scene))
        update_display();

    }
  };
  

  
  // Attach a menu plugin
  viewer.plugins.push_back(&menu);
  // Add content to the default menu window
  menu.callback_draw_viewer_menu = default_menu_function;
  
  viewer.callback_key_pressed = key_down;
  viewer.callback_post_draw = post_draw;
  viewer.callback_pre_draw = pre_draw;
  viewer.callback_mouse_up = mouse_up;
  viewer.callback_mouse_scroll = mouse_scroll;

  viewer.data().clear();
  viewer.data().lines.resize(0,9);
  viewer.data().points.resize(0,6);
  
  viewer.data().set_mesh(V, F);
  viewer.data().set_colors(C);
  viewer.core.background_color<<1.,1.,1.,1.;
  viewer.data().line_color<<173./255,174./255,103./255,1.;
  viewer.data().show_lines = false;
  viewer.data().show_texture = (FUV.rows() == F.rows()) ? true: false;
  viewer.data().face_based = (C.rows() == V.rows()) ? false: true;
  
  update_display();
  viewer.launch();
}
