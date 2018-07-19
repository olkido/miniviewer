function meshplot(V,F,data)

if ~exist('data','var')
    data = struct;
end
if (size(V,2)==2)
    V = [V,zeros(size(V,1),1)];
end
directory = '~/Documents/MATLAB/';
tmpfilename = [directory, 'tmp.off'];
writeOFF(tmpfilename,V,F);

%% scalar function 
has_scalar = false;
if (isfield( data,'S') && ~isempty(data.S))
    fileS = [directory, 'tmp_S.txt'];
    dlmwrite(fileS,data.S,'delimiter',' ');
    has_scalar = true;
end

%% per-face vector field and colors
has_fvf = false;
if (isfield( data,'FVF') && ~isempty(data.FVF))
    fileFVF = [directory, 'tmp_FVF.txt'];
    dlmwrite(fileFVF,data.FVF,'delimiter',' ');
    if (~isfield( data,'FVF_C') || isempty(data.FVF_C))
        data.FVF_C = zeros(size(data.FVF));
        data.FVF_C(:,1:3:end) = 1;
    end
    fileFVF_C = [directory, 'tmp_FVF_C.txt'];
    dlmwrite(fileFVF_C,data.FVF_C,'delimiter',' ');
    has_fvf = true;
end

%% texture coordinates
has_texture = false;
if (isfield( data,'TEXTURE_UV') && ~isempty(data.TEXTURE_UV))
    fileTEXTURE_UV = [directory, 'tmp_TEXTURE_UV.txt'];
    dlmwrite(fileTEXTURE_UV,data.TEXTURE_UV,'delimiter',' ');
    if (~isfield( data,'TEXTURE_FUV') || isempty(data.TEXTURE_FUV))
        data.TEXTURE_FUV = F;
    end
    fileTEXTURE_FUV = [directory, 'tmp_TEXTURE_FUV.txt'];
    dlmwrite(fileTEXTURE_FUV,data.TEXTURE_FUV-1,'delimiter',' ');
    has_texture = true;
end

%% lines and colors
has_lines = false;
if (isfield( data,'LINES') && ~isempty(data.LINES))
    fileLINES = [directory, 'tmp_LINES.txt'];
    dlmwrite(fileLINES,data.LINES,'delimiter',' ');
    if (~isfield( data,'LINES_C') || isempty(data.LINES_C))
        data.LINES_C = zeros(size(data.LINES,1),size(data.LINES,2)/2);
        data.LINES_C(:,1:3:end) = 1;
    end
    fileLINES_C = [directory, 'tmp_LINES_C.txt'];
    dlmwrite(fileLINES_C,data.LINES_C,'delimiter',' ');
    has_lines = true;
end


%% scene, in the following order : trackball_angle (x,y,z,w), model_translation (x,y,z), camera_zoom, model_zoom
has_scene = false;
if (isfield( data,'TANGLE') && ~isempty(data.TANGLE)) || ...
    (isfield( data,'TTRANS') && ~isempty(data.TTRANS)) || ...
    (isfield( data,'CZOOM') && ~isempty(data.CZOOM)) || ...
    (isfield( data,'MZOOM') && ~isempty(data.MZOOM))
    has_scene = true;
end
if has_scene
    data.SCENE = [data.TANGLE, data.TTRANS, data.CZOOM, data.MZOOM];
    fileSCENE = [directory, 'tmp_SCENE.txt'];
    dlmwrite(fileSCENE,data.SCENE,'delimiter',' ');
end

command = ['/usr/local/bin/miniviewer ',tmpfilename];
if has_scalar
    command = [command, ' -colors ',fileS];
end
if has_fvf
    command = [command, ' -fvf ',fileFVF,' ',fileFVF_C];
end
if has_lines
    command = [command, ' -lines ',fileLINES,' ',fileLINES_C];
end
if has_texture
    command = [command, ' -texcoords ',fileTEXTURE_UV,' ',fileTEXTURE_FUV];
end
if has_scene
    command = [command, ' -scene ',fileSCENE];
end

command = [command,' &'];

system(command);
pause(.5)
system(['rm ',tmpfilename]);
if has_scalar
    system(['rm ',fileS]);
end
if has_fvf
    system(['rm ',fileFVF]);
    system(['rm ',fileFVF_C]);
end
if has_lines
    system(['rm ',fileLINES]);
    system(['rm ',fileLINES_C]);
end
if has_texture
    system(['rm ',fileTEXTURE_UV]);
    system(['rm ',fileTEXTURE_FUV]);
end
if has_scene
    system(['rm ',fileSCENE]);
end
end
