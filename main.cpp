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
#include <igl/jet.h>
#include <igl/barycenter.h>
#include <igl/bounding_box_diagonal.h>

#include <iostream>
#include <fstream>
#include <string>

using namespace Eigen;
using namespace std;
Eigen::MatrixXd V;
Eigen::MatrixXi F;
Eigen::MatrixXd C;
MatrixXd UV, N, FVF, CFVF;
MatrixXi FUV, FN;
MatrixXd B,L,CL;
Quaternionf trackball_angle;
Eigen::Vector3f model_translation;
double camera_zoom;
double model_zoom;
bool has_scene = false;

igl::opengl::glfw::Viewer viewer;
igl::opengl::glfw::imgui::ImGuiMenu menu;

#define MAXBUFSIZE  ((int) 1e6)

float uv_scale = 1;
bool show_mesh_stats = false;
int show_scene = 0;
#define NUM_SCENE_OPTIONS 4; //no scene, angle, trans, zoom


void update_display();

MatrixXd readMatrix(const char *filename)
{
  int cols = 0, rows = 0;
  double buff[MAXBUFSIZE];
  
  // Read numbers from file into buffer.
  ifstream infile;
  infile.open(filename);
  if (!infile.is_open())
    return MatrixXd::Zero(0, 0);
  
  while (! infile.eof())
  {
    string line;
    getline(infile, line);
    
    int temp_cols = 0;
    stringstream stream(line);
    while(! stream.eof())
      stream >> buff[cols*rows+temp_cols++];
    
    if (temp_cols == 0)
      continue;
    
    if (cols == 0)
      cols = temp_cols;
    
    rows++;
  }
  
  infile.close();
  
  rows--;
  
  // Populate matrix with numbers.
  MatrixXd result(rows,cols);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      result(i,j) = buff[ cols*i+j ];
  
  return result;
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


/*
 
 bool init_viewer(igl::opengl::glfw::Viewer& viewer)
 {
 // Generate menu
 viewer.screen->performLayout();
 
 return false;
 };
 
 */
void update_display()
{
  double diag = igl::bounding_box_diagonal(V);
  
  viewer.data().lines            = Eigen::MatrixXd (0,9);
  viewer.data().points           = Eigen::MatrixXd (0,6);
  viewer.data().labels_positions = Eigen::MatrixXd (0,3);
  viewer.data().labels_strings.clear();
  if (UV.rows()>0 && FUV.rows() == F.rows())
  {
    //        double minU = UV.col(0).minCoeff();
    //        double maxU = UV.col(0).maxCoeff();
    //        double minV = UV.col(1).minCoeff();
    //        double maxV = UV.col(1).maxCoeff();
    //        UV.col(0) << (UV.col(0).array() - minU) / (maxU-minU);
    //        UV.col(1) << (UV.col(1).array() - minV) / (maxV-minV);
    viewer.data().set_uv(uv_scale*UV, FUV);
    viewer.data().show_texture = true;
    
    Eigen::Matrix<unsigned char,Eigen::Dynamic,Eigen::Dynamic> texture_R, texture_G, texture_B;
    line_texture(texture_R, texture_G, texture_B);
    viewer.data().set_texture(texture_R, texture_B, texture_G);
  }
  
  if (FVF.rows() == F.rows())
  {
    int n = FVF.cols() /3;
    for (int i=0;i <n; ++i)
      viewer.data().add_edges(B, B+FVF.block(0, i*3, FVF.rows(), 3), CFVF.block(0, i*3, FVF.rows(), 3));
    viewer.data().point_size = 5;
    viewer.data().add_points(B, Eigen::RowVector3d::Zero());
  }
  
  if (L.rows()>0)
  {
    int n = L.cols() /6;
    for (int i=0;i <n; ++i)
      viewer.data().add_edges(L.block(0, i*6, L.rows(), 3),
                              L.block(0, i*6+3, L.rows(), 3),
                              CL.block(0, i*3, CL.rows(), 3));
    
  }
  
  
  int numV = V.rows();
  int numF = F.rows();
  if (show_mesh_stats)
  {
    std::string tag;
    tag = "#V: "+to_string(numV)+", #F: "+to_string(numF);
    viewer.data().add_label(RowVector3d::Zero(), tag);
  }
  
  
  
  if (show_scene>0)
  {
    ofstream ofs;
    ofs.open("/Users/olkido/Dropbox/Work/code/mine/miniviewer/scene.txt");
    
    ofs<<"----- CAMERA PARAMETERS -----"<<endl;
    VectorXd t(Vector4d (viewer.core.trackball_angle.x(),
                         viewer.core.trackball_angle.y(),
                         viewer.core.trackball_angle.z(),
                         viewer.core.trackball_angle.w()
                         ));
    ofs<<igl::matlab_format(t.transpose().eval(),"TANGLE")<<endl;
    ofs<<igl::matlab_format(viewer.core.model_translation.transpose().eval(), "TTRANS")<<endl;
    ofs<<" CZOOM = [" <<endl<<viewer.core.camera_zoom<<endl<<"];"<<endl;
    ofs<<" MZOOM = [" <<endl<<viewer.core.model_zoom<<endl<<"];"<<endl;
    
    ofs<<" viewer.core.model_zoom_uv = " <<viewer.core.model_zoom_uv<<endl;
    ofs<<" viewer.core.model_translation_uv = " <<igl::matlab_format(viewer.core.model_translation_uv.transpose().eval())<<endl;
    ofs<<" viewer.core.orthographic = " <<viewer.core.orthographic<<endl;
    ofs<<" viewer.core.camera_eye = " <<igl::matlab_format(viewer.core.camera_eye.transpose().eval())<<endl;
    ofs<<" viewer.core.camera_up = " <<igl::matlab_format(viewer.core.camera_up.transpose().eval())<<endl;
    ofs<<" viewer.core.camera_center = " <<igl::matlab_format(viewer.core.camera_center.transpose().eval())<<endl;
    ofs<<" viewer.core.camera_view_angle = " <<viewer.core.camera_view_angle<<endl;
    ofs<<" viewer.core.camera_dnear = " <<viewer.core.camera_dnear<<endl;
    ofs<<" viewer.core.camera_dfar = " <<viewer.core.camera_dfar<<endl;
    MatrixXf mv(viewer.core.model * viewer.core.view);
    ofs<<igl::matlab_format(mv.topLeftCorner(3, 3).eval(),"mv_r")<<endl;
    ofs<<igl::matlab_format(mv.topRightCorner(3, 1).eval(),"mv_t")<<endl;
    ofs.close();
    
    
    RowVector3d pos = V.colwise().mean();// RowVector3d::Zero();
    std::string tag;
    if (show_scene==1)
      tag = "TANGLE: "
      +to_string(viewer.core.trackball_angle.x())
      +", "
      +to_string(viewer.core.trackball_angle.y())
      +", "
      +to_string(viewer.core.trackball_angle.z())
      +", "
      +to_string(viewer.core.trackball_angle.w())
      ;
    else if (show_scene==2)
      tag = "TRANS: "
      +to_string(viewer.core.model_translation.x())
      +", "
      +to_string(viewer.core.model_translation.y())
      +", "
      +to_string(viewer.core.model_translation.z())
      ;
    else if (show_scene==3)
      tag = "CZOOM: "
      +to_string(viewer.core.camera_zoom)
      +" MZOOM: "
      +to_string(viewer.core.model_zoom)
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


bool parse_arguments(int argc, char *argv[])
{
  
  int current_argument = 2;
  std::string argument;
  while (current_argument < argc)
  {
    argument = std::string(argv[current_argument]);
    if (argument.compare("-colors")==0)
    {
      current_argument ++;
      if (current_argument>=argc)
      {
        cerr<<"colors: Filename for colors not provided"<<endl;
        return false;
      }
      argument = std::string(argv[current_argument]);
      MatrixXd T = readMatrix(argument.c_str());
      if (T.cols() == 3)
        C = T;
      else
        igl::jet(T, true, C);
      
    }
    else if (argument.compare("-fvf")==0)
    {
      current_argument ++;
      if (current_argument>=argc)
      {
        cerr<<"fvf: Filename for face-based vector field not provided"<<endl;
        return false;
      }
      argument = std::string(argv[current_argument]);
      MatrixXd T = readMatrix(argument.c_str());
      FVF = T;
      CFVF.setZero(FVF.rows(),3);
      
      current_argument ++;
      if (current_argument>=argc)
        cerr<<"FVF: Filename for colors not provided"<<endl;
      else
      {
        argument = std::string(argv[current_argument]);
        CFVF = readMatrix(argument.c_str());
        if ((CFVF.cols() != FVF.cols()) || (CFVF.rows() != FVF.rows()))
          cerr<<"Colors for vector field is the wrong size"<<endl;
      }
    }
    else if (argument.compare("-texcoords")==0)
    {
      current_argument ++;
      if (current_argument>=argc)
      {
        cerr<<"scene: Filename for texture coordinates not provided"<<endl;
        return false;
      }
      argument = std::string(argv[current_argument]);
      UV = readMatrix(argument.c_str());
      
      current_argument ++;
      if (current_argument>=argc)
      {
        cerr<<"texcoords: Filename for corner indices not provided"<<endl;
        return false;
      }
      argument = std::string(argv[current_argument]);
      MatrixXd FUV_d = readMatrix(argument.c_str());
      FUV = FUV_d.cast<int>();
      if ((FUV.cols() != F.cols()) || (FUV.rows() != F.rows()))
      {
        cerr<<"Texture corner indices are of wrong size"<<endl;
        return false;
      }
    }
    else if (argument.compare("-lines")==0)
    {
      current_argument ++;
      if (current_argument>=argc)
      {
        cerr<<"lines: Filename for lines not provided"<<endl;
        return false;
      }
      argument = std::string(argv[current_argument]);
      MatrixXd T = readMatrix(argument.c_str());
      L = T;
      CL.setZero(L.rows(),3);
      
      current_argument ++;
      if (current_argument>=argc)
        cerr<<"lines: Filename for line colors not provided"<<endl;
      else
      {
        argument = std::string(argv[current_argument]);
        T = readMatrix(argument.c_str());
        if ((T.cols()*2 != L.cols()) || (T.rows() != L.rows()))
          cerr<<"Colors for lines is the wrong size"<<endl;
        CL = T;
      }
      
    }
    else if (argument.compare("-scene")==0)
    {
      has_scene = true;
      current_argument ++;
      if (current_argument>=argc)
      {
        cerr<<"scene: Filename for scene parameters not provided"<<endl;
        return false;
      }
      argument = std::string(argv[current_argument]);
      ifstream ifs;
      ifs.open(argument.c_str(),std::ifstream::in);
      if (!ifs.is_open())
      {
        cerr<<"scene: Filename for scene parameters cannot be opened"<<endl;
        return false;
      }
      double x,y,z,w;
      ifs>>x;
      ifs>>y;
      ifs>>z;
      ifs>>w;
      trackball_angle=Quaternionf(w,x,y,z);
      ifs>>x;
      ifs>>y;
      ifs>>z;
      model_translation<<x,y,z;
      ifs>>x;
      camera_zoom= x;
      ifs>>x;
      model_zoom= x;
      ifs.close();
    }
    
    current_argument ++;
  }
  
  return true;
}

int main(int argc, char *argv[])
{
  if (argc<2)
  {
    cerr<<"miniViewer requires at least one argument (the mesh file)"<<endl;
    return 0;
  }
  
  // dirname, basename, extension and filename
  std::string dir,  base,  ext,  name;
  igl::pathinfo(std::string(argv[1]),dir,base,ext,name);
  transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  
  if(ext == "obj")
    igl::readOBJ(argv[1], V, UV, N, F, FUV, FN);
  else if(ext == "off")
    igl::readOFF(argv[1], V, F);
  else
    igl::read_triangle_mesh(argv[1], V, F);
  
  
  igl::barycenter(V, F, B);
  C.setConstant(F.rows(),3,.99);

  if (!parse_arguments(argc, argv))
    exit(1);
  
  update_display();
  
  // Customize the menu
  const auto & default_menu_function = [&]()
  {
    // Draw parent menu content
    menu.draw_viewer_menu();
    
    // Add new group to *existing* viewer menut
    if (ImGui::CollapsingHeader("New Group", ImGuiTreeNodeFlags_DefaultOpen))
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
  
//  // Draw additional windows
//  menu.callback_draw_custom_window = my_menu_function;
//
//  
  viewer.callback_key_pressed = key_down;
  
  viewer.data().clear();
  viewer.data().lines.resize(0,9);
  viewer.data().points.resize(0,6);
  
  viewer.data().set_mesh(V, F);
  viewer.data().set_colors(C);
  viewer.core.background_color<<1.,1.,1.,1.;
  viewer.data().line_color<<173./255,174./255,103./255,1.;
  viewer.data().show_lines = false;
  viewer.data().show_texture = false;
  viewer.data().face_based = false;
  
  
  //    viewer.core.view.setIdentity();
  if(has_scene)
  {
    viewer.core.trackball_angle = trackball_angle;
    viewer.core.model_translation = model_translation;
    viewer.core.camera_zoom= camera_zoom;
    viewer.core.model_zoom=  model_zoom;
  }
  
  update_display();
  viewer.launch();
}
