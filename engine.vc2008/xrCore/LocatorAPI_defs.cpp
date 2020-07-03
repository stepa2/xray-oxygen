#include "stdafx.h"
#pragma hdrstop

#include "LocatorAPI_defs.h"
#pragma warning(disable:4995)
#include <io.h>
#include <direct.h>
#include <fcntl.h>
#include <sys\stat.h>
#pragma warning(default:4995)

//////////////////////////////////////////////////////////////////////
// FS_File
//////////////////////////////////////////////////////////////////////
FS_File::FS_File(xr_string nm, long sz, time_t modif,unsigned attr)	{set(nm,sz,modif,attr);}
FS_File::FS_File(xr_string nm)										{set(nm,0,0,0);}
FS_File::FS_File(const _FINDDATA_T& f)								{set(f.name,f.size,f.time_write,(f.attrib&_A_SUBDIR)?flSubDir:0);}
FS_File::FS_File(xr_string nm, const _FINDDATA_T& f)				{set(nm,f.size,f.time_write,(f.attrib&_A_SUBDIR)?flSubDir:0);}

void FS_File::set(xr_string nm, long sz, time_t modif,unsigned attr)
{
	name		= nm;		xr_strlwr	(name);
	size		= sz;
	time_write	= modif;
	attrib		= attr;
}

//////////////////////////////////////////////////////////////////////
// FS_Path
//////////////////////////////////////////////////////////////////////
FS_Path::FS_Path	(const char* _Root, const char* _Add, const char* _DefExt, const char* _FilterCaption, u32 flags)
{
	xr_string temp2 = _Root;
    //Giperion: fs_root can goes without trailing slash, add one, if we miss that on root
	xr_string::FixSlashes(temp2);
    //Giperion end

	if (_Add)
	{
		temp2.append(_Add);
	}

	if (!temp2.empty())
	{
		if (temp2[temp2.size() - 1] != '\\')
		{
			temp2.push_back('\\');
		}
	}

	string_path finalPath;
	ExpandEnvironmentStrings(temp2.c_str(), finalPath, sizeof(finalPath));

	m_Path			= xr_strlwr(xr_strdup(finalPath));
	m_DefExt		= _DefExt?xr_strlwr(xr_strdup(_DefExt)):nullptr;
	m_FilterCaption	= _FilterCaption?xr_strlwr(xr_strdup(_FilterCaption)):nullptr;
	m_Add			= _Add?xr_strlwr(xr_strdup(_Add)):nullptr;
	m_Root			= _Root?xr_strlwr(xr_strdup(_Root)):nullptr;
    m_Flags.assign	(flags);
}

FS_Path::~FS_Path	()
{
	xr_free	(m_Root);
	xr_free	(m_Path);
	xr_free	(m_Add);
	xr_free	(m_DefExt);
	xr_free	(m_FilterCaption);
}

void	FS_Path::_set	(const char* add)
{
	// m_Add
	R_ASSERT		(add);
	xr_free			(m_Add);
	m_Add			= xr_strlwr(xr_strdup(add));

	// m_Path
	string_path		temp;
	xr_strconcat	(temp,m_Root,m_Add);
	if (temp[xr_strlen(temp)-1]!='\\') xr_strcat(temp,"\\");
	xr_free			(m_Path);
	m_Path			= xr_strlwr(xr_strdup(temp));
}

void	FS_Path::_set_root	(const char* root)
{
	string_path		temp;
	xr_strcpy		( temp, root );
	if (m_Root[0] && m_Root[xr_strlen(m_Root)-1]!='\\') xr_strcat(temp,"\\");
	xr_free			(m_Root);
	m_Root			= xr_strlwr(xr_strdup(temp));

	// m_Path
	xr_strconcat (temp,m_Root,m_Add ? m_Add : "");
	if (*temp && temp[xr_strlen(temp)-1]!='\\') xr_strcat(temp,"\\");
	xr_free			(m_Path);
	m_Path			= xr_strlwr(xr_strdup(temp));
}

const char* FS_Path::_update(string_path& dest, const char* src)const
{
	R_ASSERT			(dest);
    R_ASSERT			(src);
	string_path			temp;
	xr_strcpy			(temp, sizeof(temp), src);
	xr_strconcat		(dest, m_Path, temp);
	return xr_strlwr	(dest);
}

void FS_Path::rescan_path_cb	()
{
	m_Flags.set(flNeedRescan,true);
    FS.m_Flags.set(CLocatorAPI::flNeedRescan,true);
}

bool XRCORE_API PatternMatch(const char* s, const char* mask)
{
	const char* cp=nullptr;
	const char* mp=nullptr;
	for (; *s&&*mask!='*'; mask++,s++) if (*mask!=*s&&*mask!='?') return false;
	for (;;) {
		if (!*s) { while (*mask=='*') mask++; return !*mask; }
		if (*mask=='*') { if (!*++mask) return true; mp=mask; cp=s+1; continue; }
		if (*mask==*s||*mask=='?') { mask++, s++; continue; }
		mask=mp; s=cp++;
	}
}

