function meshplot(V,F,S,FVF,FVF_C,LINES,LINES_C,TEXTURE_UV,TEXTURE_FUV)
if (size(V,2)==2)
    V = [V,zeros(size(V,1),1)];
end
directory = '/Users/olkido/Documents/MATLAB/';
tmpfilename = [directory, 'tmp.off'];
writeOFF(tmpfilename,V,F);
if (~exist('S','var') ||isempty(S))
    S =[];
else
    fileS = [directory, 'tmp_S.txt'];
    dlmwrite(fileS,S,'delimiter',' ');
end
if (~exist('FVF','var') ||isempty(FVF))
    FVF =[];
else
    fileFVF = [directory, 'tmp_FVF.txt'];
    dlmwrite(fileFVF,FVF,'delimiter',' ');
    if (~exist('FVF_C','var') ||isempty(FVF_C))
        FVF_C = zeros(size(FVF));
        FVF_C(:,1:3:end) = 1;
    end
    fileFVF_C = [directory, 'tmp_FVF_C.txt'];
    dlmwrite(fileFVF_C,FVF_C,'delimiter',' ');
end

if (~exist('TEXTURE_UV','var') ||isempty(TEXTURE_UV))
    TEXTURE_UV =[];
else
    fileTEXTURE_UV = [directory, 'tmp_TEXTURE_UV.txt'];
    dlmwrite(fileTEXTURE_UV,TEXTURE_UV,'delimiter',' ');
    if (~exist('TEXTURE_FUV','var') ||isempty(TEXTURE_FUV))
        TEXTURE_FUV = F;
    end
    fileTEXTURE_FUV = [directory, 'tmp_TEXTURE_FUV.txt'];
    dlmwrite(fileTEXTURE_FUV,TEXTURE_FUV-1,'delimiter',' ');
end


if (~exist('LINES','var') ||isempty(LINES))
    LINES =[];
else
    fileLINES = [directory, 'tmp_LINES.txt'];
    dlmwrite(fileLINES,LINES,'delimiter',' ');
    if (~exist('LINES_C','var') ||isempty(LINES_C))
        LINES_C = zeros(size(LINES,1),size(LINES,2)/2);
        LINES_C(:,1:3:end) = 1;
    end
    fileLINES_C = [directory, 'tmp_LINES_C.txt'];
    dlmwrite(fileLINES_C,LINES_C,'delimiter',' ');
end

command = ['/usr/local/bin/miniviewer ',tmpfilename];
if ~isempty(S)
    command = [command, ' -colors ',fileS];
end
if ~isempty(FVF)
    command = [command, ' -fvf ',fileFVF,' ',fileFVF_C];
end
if ~isempty(LINES)
    command = [command, ' -lines ',fileLINES,' ',fileLINES_C];
end
if ~isempty(TEXTURE_UV)
    command = [command, ' -texcoords ',fileTEXTURE_UV,' ',fileTEXTURE_FUV];
end
command = [command,' &']

system(command);
pause(.5)
system(['rm ',tmpfilename]);
if ~isempty(S)
    system(['rm ',fileS]);
end
if ~isempty(FVF)
    system(['rm ',fileFVF]);
end
if ~isempty(LINES)
    system(['rm ',fileLINES]);
end
end
