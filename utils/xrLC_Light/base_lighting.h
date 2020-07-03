#pragma once

#include "r_light.h"

class IReader;

class XRLC_LIGHT_API base_lighting
{
public:
    xr_vector<R_Light>		rgb;		// P,N	
    xr_vector<R_Light>		hemi;		// P,N	
    xr_vector<R_Light>		sun;		// P

    xr_vector<DWORD>	rgbIndexes;
    xr_vector<DWORD>	sunIndexes;
    xr_vector<DWORD>	hemiIndexes;

    void					select(xr_vector<R_Light>& dest, xr_vector<R_Light>& src, Fvector& P, float R);
    //Index based - for new system
    void					select(xr_vector<DWORD>& dest, xr_vector<R_Light>& src, Fvector& P, float R);
    void					select(base_lighting& from, Fvector& P, float R);
    void					read(IReader	&r);
    void					write(IWriter	&w) const;
};
