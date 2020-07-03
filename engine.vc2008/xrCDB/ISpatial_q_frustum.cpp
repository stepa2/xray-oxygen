#include "stdafx.h"
#include "ISpatial.h"
#include "frustum.h"

extern Fvector	c_spatial_offset[8];

class	walker
{
public:
	u32				mask;
	const CFrustum*		F;
	ISpatial_DB*	space;
public:
	walker					(ISpatial_DB*	_space, u32 _mask, const CFrustum* _F)
	{
		mask	= _mask;
		F		= _F;
		space	= _space;
	}
	void		walk		(ISpatial_NODE* N, Fvector& n_C, float n_R, u32 fmask)
	{
		// box
		float	n_vR	=		2*n_R;
		Fbox	BB;		BB.set	(n_C.x-n_vR, n_C.y-n_vR, n_C.z-n_vR, n_C.x+n_vR, n_C.y+n_vR, n_C.z+n_vR);
		if		(F->testAABB(BB.data(),fmask) == fcvNone)	return;

		// test items
		for (ISpatial* S : N->items)
		{
			if ((S->spatial.type & mask) == 0)	continue;

			Fvector&		sC = S->spatial.sphere.P;
			float			sR = S->spatial.sphere.R;
			u32				tmask = fmask;
			if (fcvNone == F->testSphere(sC, sR, tmask))	continue;

			space->q_result->push_back(S);
		}

		// recurse
		float	c_R		= n_R/2;
		for (u32 octant = 0; octant < 8; octant++)
		{
			if (N->children[octant] == nullptr)	continue;

			Fvector		c_C;			c_C.mad	(n_C,c_spatial_offset[octant],c_R);
			walk						(N->children[octant],c_C,c_R,fmask);
		}
	}
};

void	ISpatial_DB::q_frustum		(xr_vector<ISpatial*>& R, u32 _o, u32 _mask, const CFrustum& _frustum)	
{
	xrCriticalSectionGuard guard(cs);
	q_result			= &R;
	q_result->clear();
	walker W(this, _mask, &_frustum); 
	W.walk(m_root, m_center, m_bounds, _frustum.getMask()); 
}
