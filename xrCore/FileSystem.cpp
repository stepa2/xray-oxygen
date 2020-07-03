//----------------------------------------------------
// file: FileSystem.cpp
//----------------------------------------------------

#include "stdafx.h"
#pragma hdrstop

#include "cderr.h"
#include "commdlg.h"
#include "vfw.h"

EFS_Utils*	xr_EFS	= nullptr;
//----------------------------------------------------
EFS_Utils::EFS_Utils( )
{
}

EFS_Utils::~EFS_Utils()
{
}

xr_string	EFS_Utils::ExtractFileName(const char* src)
{
	string_path name;
	_splitpath	(src,nullptr,nullptr,name,nullptr);
    return xr_string(name);
}

xr_string	EFS_Utils::ExtractFileExt(const char* src)
{
	string_path ext;
	_splitpath	(src,nullptr,nullptr,nullptr,ext);
    return xr_string(ext);
}

xr_string	EFS_Utils::ExtractFilePath(const char* src)
{
	string_path drive,dir;
	_splitpath	(src,drive,dir,nullptr,nullptr);
    return xr_string(drive)+dir;
}

xr_string	EFS_Utils::ExcludeBasePath(const char* full_path, const char* excl_path)
{
    const char* sub		= strstr(full_path,excl_path);
	if (nullptr!=sub) 	return xr_string(sub+xr_strlen(excl_path));
	else	   		return xr_string(full_path);
}

xr_string	EFS_Utils::ChangeFileExt(const char* src, const char* ext)
{
	xr_string	tmp;
	char* src_ext	= strext(src);
    if (src_ext){
	    size_t		ext_pos	= src_ext-src;
        tmp.assign	(src,0,ext_pos);
    }else{
        tmp			= src;
    }
    tmp				+= ext;
    return tmp;
}

xr_string	EFS_Utils::ChangeFileExt(const xr_string& src, const char* ext)
{
	return ChangeFileExt(src.c_str(),ext);
}

//----------------------------------------------------
void MakeFilter(string1024& dest, const char* info, const char* ext)
{
    xr_string res;

    if (ext)
    {
    	res += info;
		res	+= "(";
		res	+= ext;
		res	+= ")|";
		res	+= ext;
		res	+= "|";
        int icnt		= _GetItemCount(ext,';');
        if(icnt>1)
        {
        for(int idx=0; idx<icnt; ++idx)
        {
          string64		buf;
          _GetItem		(ext, idx, buf, ';');
    
          res += info;
          res += "(";
          res += buf;
          res += ")|";
          res += buf;
          res += "|";
        }
      }
    	res += "|";
	}else
    {
    	res = "All files(*.*)|*.*||";
    }
    xr_strcpy(dest, res.c_str());
    
    for(u32 i=0; i<res.size(); ++i)           
    {
    	if(res[i]=='|')
        	dest[i]='\0';
    }
  

}

//------------------------------------------------------------------------------
// start_flt_ext = -1-all 0..n-indices
//------------------------------------------------------------------------------
  
// Vista uses this hook for old-style save dialog
UINT_PTR CALLBACK OFNHookProcOldStyle(HWND , UINT , WPARAM , LPARAM )
{
	// let default hook work on this message
	return 0;
}

bool EFS_Utils::GetOpenNameInternal( const char* initial,  char* buffer, int sz_buf, bool bMulti, const char* offset, int start_flt_ext ) const
{
	VERIFY				(buffer&&(sz_buf>0));
	FS_Path& P			= *FS.get_path(initial);
	string1024 			flt;
	MakeFilter			(flt,P.m_FilterCaption?P.m_FilterCaption:"",P.m_DefExt);

	OPENFILENAME 		ofn;
    std::memset( &ofn, 0, sizeof(ofn) );

    if (xr_strlen(buffer))
    {
        string_path		dr;
        if (!(buffer[0]=='\\' && buffer[1]=='\\')){ // if !network
            _splitpath		(buffer,dr,nullptr,nullptr,nullptr);

            if (0==dr[0])
            {
                string_path		bb;
            	P._update		(bb, buffer);
                xr_strcpy		(buffer, sz_buf, bb);
             }
        }
    }
    ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner 		= GetForegroundWindow();
	ofn.lpstrDefExt 	= P.m_DefExt;
	ofn.lpstrFile 		= buffer;
	ofn.nMaxFile 		= sz_buf;
	ofn.lpstrFilter 	= flt;
	ofn.nFilterIndex 	= start_flt_ext+2;
    ofn.lpstrTitle              = "Open a File";
    string512 path; 
	xr_strcpy				(path,(offset&&offset[0])?offset:P.m_Path);
	ofn.lpstrInitialDir = path;
	ofn.Flags =         OFN_PATHMUSTEXIST	|
                        OFN_FILEMUSTEXIST	|
                        OFN_HIDEREADONLY	|
                        OFN_FILEMUSTEXIST	|
                        OFN_NOCHANGEDIR		|
                        (bMulti?OFN_ALLOWMULTISELECT|OFN_EXPLORER:0);
                        
    ofn.FlagsEx			= OFN_EX_NOPLACESBAR;
  
/*
	unsigned int	dwVersion = GetVersion();
	unsigned int	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	if ( dwWindowsMajorVersion == 6 )
	{
	     	ofn.Flags |= OFN_ENABLEHOOK;
  	     	ofn.lpfnHook = OFNHookProcOldStyle;
	}
*/

	bool bRes 			= !!GetOpenFileName( &ofn );
    if (!bRes)
    {
	    u32 err 		= CommDlgExtendedError();
	    switch(err)
        {
        	case FNERR_BUFFERTOOSMALL:
            	Log("Too many files selected.");
            break;
        }
	}
    if (bRes && bMulti)
    {
    	Msg				("buff=%s",buffer);
		int cnt			= _GetItemCount(buffer,0x0);
        if (cnt>1)
        {
            char 		dir	  [255*255];
            char 		buf	  [255*255];
            char 		fns	  [255*255];

            xr_strcpy		(dir, buffer);
            xr_strcpy		(fns, dir);
            xr_strcat		(fns, "\\");
            xr_strcat		(fns, _GetItem	(buffer,1,buf,0x0));

            for (int i=2; i<cnt; i++)
            {
                xr_strcat	(fns,",");
                xr_strcat	(fns,dir);
                xr_strcat	(fns,"\\");
                xr_strcat	(fns,_GetItem(buffer,i,buf,0x0));
            }
            xr_strcpy		(buffer, sz_buf, fns);
        }
    }
    strlwr				(buffer);
    return 				bRes;
}

bool EFS_Utils::GetSaveName(const char* initial, string_path& buffer, const char* offset, int start_flt_ext)
{
	//	unsigned int	dwVersion = GetVersion();
	//	unsigned int	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));

	FS_Path& P = *FS.get_path(initial);
	string1024 			flt;

	const char* def_ext = P.m_DefExt;
	
	MakeFilter(flt, P.m_FilterCaption ? P.m_FilterCaption : "", def_ext);
	OPENFILENAME ofn;
	std::memset(&ofn, 0, sizeof(ofn));
	if (xr_strlen(buffer)) {
		string_path		dr;
		if (!(buffer[0] == '\\' && buffer[1] == '\\')) { // if !network
			_splitpath(buffer, dr, nullptr, nullptr, nullptr);
			if (0 == dr[0])	P._update(buffer, buffer);
		}
	}
	ofn.hwndOwner = GetForegroundWindow();
	ofn.lpstrDefExt = def_ext;
	ofn.lpstrFile = buffer;
	ofn.lpstrFilter = flt;
	ofn.lStructSize = sizeof(ofn);
	ofn.nMaxFile = sizeof(buffer);
	ofn.nFilterIndex = start_flt_ext + 2;
	ofn.lpstrTitle = "Save a File";
	string512 path; xr_strcpy(path, (offset&&offset[0]) ? offset : P.m_Path);
	ofn.lpstrInitialDir = path;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.FlagsEx = OFN_EX_NOPLACESBAR;


	bool bRes = !!GetSaveFileName(&ofn);

	if (!bRes && CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
		Log("Too many file selected.");

	strlwr(buffer);
	return bRes;
}
//----------------------------------------------------
const char* EFS_Utils::AppendFolderToName(char* tex_name, u32 const tex_name_size, int depth, BOOL full_name)
{
	string256 _fn;
	xr_strcpy(tex_name,tex_name_size,AppendFolderToName(tex_name, _fn, sizeof(_fn), depth, full_name));
	return tex_name;
}

const char* EFS_Utils::AppendFolderToName(const char* src_name, char* dest_name, u32 const dest_name_size, int depth, BOOL full_name)
{
	shared_str tmp = src_name;
    const char* s 	= src_name;
    char* d 	= dest_name;
    int sv_depth= depth;
	for (; *s&&depth; s++, d++){
		if (*s=='_'){depth--; *d='\\';}else{*d=*s;}
	}
	if (full_name){
		*d			= 0;
		if (depth<sv_depth)	xr_strcat(dest_name,dest_name_size,*tmp);
	}else{
		for (; *s; s++, d++) *d=*s;
		*d			= 0;
	}
    return dest_name;
}

const char* EFS_Utils::GenerateName(const char* base_path, const char* base_name, const char* def_ext, char* out_name, u32 const out_name_size)
{
    int cnt = 0;
	string_path fn;
    if (base_name)	
		xr_strconcat(fn, base_path, base_name, def_ext);
	else 			
		xr_sprintf		(fn, sizeof(fn), "%s%02d%s",base_path,cnt++,def_ext);

	while (FS.exist(fn))
	    if (base_name)	
			xr_sprintf	(fn, sizeof(fn),"%s%s%02d%s",base_path,base_name,cnt++,def_ext);
        else 			
			xr_sprintf	(fn, sizeof(fn), "%s%02d%s",base_path,cnt++,def_ext);
    xr_strcpy(out_name,out_name_size,fn);
	return out_name;
}

//#endif
