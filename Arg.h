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


enum  optionIndex { UNKNOWN, HELP, COLORS, SCALAR_FIELD, FACE_VECTOR_FIELD, UV_COORDS, UV_INDS, LINES, LINE_COLORS, CAMERA};
const option::Descriptor usage[] =
{
  {UNKNOWN, 0, "", "",Arg::None, "USAGE: smallMeshParser [options] input_directory output_directory\n\n"
    "Options:" },
  {HELP, 0,"", "help",Arg::None, "  --help  \tPrint usage and exit." },
  {COLORS, 0,"", "colors", Arg::Required, "  --colors  \tFilename including per-vertex / per-face colors." },
  {SCALAR_FIELD, 0,"", "scalar", Arg::Required, "  --scalar  \tFilename including per-vertex / per-face scalar values." },
  {FACE_VECTOR_FIELD, 0,"","fvf",Arg::Required, "  --fvf  \tFilename including per-face vector field." },
  {UV_COORDS, 0,"","uv",Arg::Required, "  --uv  \tFilename including uv texture coordinates. If provided, corner texture indices also need to be provided (option --fuv)." },
  {UV_INDS, 0,"","fuv",Arg::Required, "  --fuv  \tFilename including corner texture indices. If provided, uv texture coordinates also need to be provided (option --uv)." },
  {LINES, 0,"","lines",Arg::Required, "  --lines  \tFilename including line start and end points." },
  {LINE_COLORS, 0,"","line_colors",Arg::Required, "  --line_colors  \tFilename including line colors. If provided, line start and end points also need to be provided (option --lines)." },
  {CAMERA, 0,"c","camera",Arg::Required, "  --camera, -c  \tFilename including camera pose parameters, in the order : trackball_angle, camera_translation, camera_base_zoom, camera_zoom." },
  {UNKNOWN, 0, "", "",Arg::None, "\nExamples:\n"
    "  example --unknown -- --this_is_no_option\n"
    "  example -unk --plus -ppp file1 file2\n" },
  {0,0,0,0,0,0}
};

#endif /* Arg_h */
