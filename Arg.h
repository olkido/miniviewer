//
//  Arg.h
//  harmonifyMe
//
//  Created by Olga Diamanti on 25/05/16.
//
//

#ifndef Arg_h
#define Arg_h



struct Arg: public option::Arg
{
  static void printError(const char* msg1, const option::Option& opt, const char* msg2)
  {
    fprintf(stderr, "%s", msg1);
    fwrite(opt.name, opt.namelen, 1, stderr);
    fprintf(stderr, "%s", msg2);
  }
  
  static option::ArgStatus Unknown(const option::Option& option, bool msg)
  {
    if (msg) printError("Unknown option '", option, "'\n");
    return option::ARG_ILLEGAL;
  }
  
  static option::ArgStatus Required(const option::Option& option, bool msg)
  {
    if (option.arg != 0)
      return option::ARG_OK;
    
    if (msg) printError("Option '", option, "' requires an argument\n");
    return option::ARG_ILLEGAL;
  }
  
  static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
  {
    if (option.arg != 0 && option.arg[0] != 0)
      return option::ARG_OK;
    
    if (msg) printError("Option '", option, "' requires a non-empty argument\n");
    return option::ARG_ILLEGAL;
  }
  
  static option::ArgStatus Numeric(const option::Option& option, bool msg)
  {
    char* endptr = 0;
    if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
    if (endptr != option.arg && *endptr == 0)
      return option::ARG_OK;
    
    if (msg) printError("Option '", option, "' requires a numeric argument\n");
    return option::ARG_ILLEGAL;
  }
};


enum  optionIndex { UNKNOWN, HELP, COLORS, SCALAR_FIELD, FACE_VECTOR_FIELD, FACE_VECTOR_FIELD_COLORS, UV_COORDS, UV_INDS, TEXTURE_IMAGE, LINES, LINE_COLORS, CAMERA, SAVE_PNG, EXIT_AFTER_PNG};
const option::Descriptor usage[] =
{
  {UNKNOWN, 0, "", "",Arg::None, "USAGE: miniviewer [options] mesh_file\n\n"
    "Options:" },
  {HELP, 0,"", "help",Arg::None, "  --help  \tPrint usage and exit." },
  {CAMERA, 0,"c","camera",Arg::Required, "  --camera, -c  \tFilename (xml) including camera pose parameters." },
  {COLORS, 0,"", "colors", Arg::Optional, "  --colors  \tPlot per-vertex or per-face colors on top of mesh. If an additional filename argument is provided, colors will be read from there. Otherwise, will read from <meshname>.colors file. The read matrix needs to have 3 columns." },
  {SCALAR_FIELD, 0,"", "scalar", Arg::Optional, "  --scalar  \tPlot per-vertex or per-face scalar field on top of mesh, normalized into colors. If an additional filename argument is provided, scalars will be read from there. Otherwise, will read from <meshname>.scalars file. The read matrix needs to have 1 column." },
  {FACE_VECTOR_FIELD, 0,"","fvf",Arg::Optional, "  --fvf  \tPlot per-face vector field on top of mesh. If an additional filename argument is provided, the vector field will be read from there. Otherwise, will read from <meshname>.fvf file. The read matrix needs to have as many rows as mesh faces, and its columns need to be divisable by 3." },
  {FACE_VECTOR_FIELD_COLORS, 0,"","fvf_colors",Arg::Optional, "  --fvf_colors  \tColor the per-face vector field. If an additional filename argument is provided, vector field colors will be read from there. Otherwise, will read from <meshname>.fvfc file. The dimensions of this matrix need to match the dimension of the vector field." },
  {UV_COORDS, 0,"","uv",Arg::Optional, "  --uv  \tRead uv coordinates for texturing. If an additional filename argument is provided, uv coordinates will be read from there. Otherwise, will read from <meshname>.uv file. The read matrix needs to have 2 columns. If --uv is on, corner texture indices also need to be provided (option --fuv)." },
  {UV_INDS, 0,"","fuv",Arg::Optional, "  --fuv  \tRead per-corner texture indices for texturing. If an additional filename argument is provided, texture indices will be read from there. Otherwise, will read from <meshname>.fuv file. The read matrix needs to have as many rows as mesh faces, and 3 columns. If --fuv is on, uv texture coordinates also need to be provided (option --uv)." },
  {TEXTURE_IMAGE, 0,"","texture",Arg::Optional, "  --texture  \tRead a texture image. Only .png images are supported. If an additional filename argument is provided, texture image will be read from there. Otherwise, will read from <meshname>.png file. " },
  {LINES, 0,"","lines",Arg::Optional, "  --lines  \tPlot lines on top of mesh. If an additional filename argument is provided, the lines will be read from there. Otherwise, will read from <meshname>.lines file. Each line is described by its start and end points (6 numbers). The columns of the read matrix need to be divisable by 6." },
  {LINE_COLORS, 0,"","line_colors",Arg::Optional, "  --line_colors  \tColor the lines. If an additional filename argument is provided, line colors will be read from there. Otherwise, will read from <meshname>.linesc file. The rows of this matrix need to match the rows of the vector field, and the number of columns needs to be divisable by 3 (3 numbers per color) . If --linesc is on, lines also need to be provided (option --lines)" },
  {SAVE_PNG, 0,"","png",Arg::Required, "  --png  \tFilename to which to save a .png screenshot." },
  {EXIT_AFTER_PNG, 0,"x","--exit",Arg::None, "  --exit, -x  \tExit after saving the first .png screenshot." },
  {UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
    "  example --unknown -- --this_is_no_option\n"
    "  example -unk --plus -ppp file1 file2\n" },
  {0,0,0,0,0,0}
};

#endif /* Arg_h */
