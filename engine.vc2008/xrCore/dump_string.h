#pragma once
XRCORE_API xr_string get_string(bool v);
XRCORE_API xr_string get_string(const Fvector& v);
XRCORE_API xr_string get_string(const Fmatrix& dop);
XRCORE_API xr_string get_string(const Fbox &box);

XRCORE_API xr_string dump_string(const char* name, const Fvector &v);
XRCORE_API xr_string dump_string(const char* name, const Fmatrix &form);
XRCORE_API void dump(const char* name, const Fmatrix &form);
XRCORE_API void dump(const char* name, const Fvector &v);
