#include "stdafx.h"
#include "wallmark_manager.h"
#include "Level.h"
#include "../xrEngine/gamemtllib.h"
#include "../xrphysics/CalculateTriangle.h"
#include "../xrEngine/profiler.h"
#ifdef DEBUG
#include "phdebug.h"
#endif

CWalmarkManager::CWalmarkManager()
{
}
CWalmarkManager::~CWalmarkManager()
{
	Clear();
}
void CWalmarkManager::Clear()
{
	m_wallmarks->clear();
}

void CWalmarkManager::AddWallmark(const Fvector& dir, const Fvector& start_pos, 
								  float range, float wallmark_size,
								  IWallMarkArray& wallmarks_vector,int t)
{
	CDB::TRI*	pTri	= Level().ObjectSpace.GetStaticTris()+t;//result.element;
	SGameMtl*	pMaterial = GMLib.GetMaterialByIdx(pTri->material);

	if(pMaterial->Flags.is(SGameMtl::flBloodmark))
	{
		//вычислить нормаль к пораженной поверхности
		Fvector*	pVerts	= Level().ObjectSpace.GetStaticVerts();

		//вычислить точку попадания
		Fvector end_point;
		end_point.set(0,0,0);
		end_point.mad(start_pos, dir, range);

		if (!wallmarks_vector.empty())
		{		
			::Render->add_StaticWallmark(&wallmarks_vector, end_point, wallmark_size, pTri, pVerts);
		}
	}
}

void CWalmarkManager::PlaceWallmarks( const Fvector& start_pos)
{
	m_pos = start_pos;
	Load("explosion_marks");

	Device.seqParallel.emplace_back(this,&CWalmarkManager::StartWorkflow);
}

float Distance (const Fvector& rkPoint, const Fvector rkTri[3], float& pfSParam, float& pfTParam, Fvector& closest, Fvector& dir);

void CWalmarkManager::StartWorkflow()
{
	LPCSTR				sect				= "explosion_marks";
	float				m_trace_dist		= pSettings->r_float(sect,"dist");
	float				m_wallmark_size		= pSettings->r_float(sect,"size");
	u32					max_wallmarks_count = pSettings->r_u32(sect,"max_count");


	XRC.box_options							(0);
	XRC.box_query							(Level().ObjectSpace.GetStaticModel(),m_pos,Fvector().set(m_trace_dist,m_trace_dist,m_trace_dist));

	CDB::TRI*		T_array					= Level().ObjectSpace.GetStaticTris();
	Fvector*		V_array					= Level().ObjectSpace.GetStaticVerts();
	CDB::RESULT*	R_begin                 = XRC.r_begin();
	CDB::RESULT*    R_end                   = XRC.r_end();
	u32				wm_count	= 0;


	u32 _ray_test		= 0;
	u32 _tri_not_plane	= 0;
	u32 _not_dist		= 0;

	for (CDB::RESULT* Res=R_begin; Res!=R_end; ++Res)
	{
		if(wm_count >= max_wallmarks_count) break;
		
		Fvector						end_point;
		Fvector						pdir;
		float						pfSParam;
		float						pfTParam;

		Fvector						_tri[3];

		CDB::TRI*		_t			= T_array + Res->id;

		_tri[0]						= V_array[_t->verts[0]];
		_tri[1]						= V_array[_t->verts[1]];
		_tri[2]						= V_array[_t->verts[2]];

		float dist					= Distance (m_pos, _tri, pfSParam, pfTParam, end_point, pdir);
		float test					= dist-EPS_L;
		
		if(test>0.f)
		{
			if(Level().ObjectSpace.RayTest(m_pos, pdir, test, collide::rqtStatic, nullptr, m_owner))
			{
				++_ray_test;
				continue;
			}
		}

		if( fis_zero(pfSParam) || fis_zero(pfTParam) || fsimilar(pfSParam,1.0f) || fsimilar(pfTParam,1.0f)  )
		{
			++_tri_not_plane;
			continue;
		}

		if(dist <= m_trace_dist )
		{
			::Render->add_StaticWallmark(&*m_wallmarks, end_point, m_wallmark_size, _t, V_array);
			++wm_count;
		}
		else ++_not_dist;

	}
}

void CWalmarkManager::Load (LPCSTR section)
{
	//кровавые отметки на стенах
	LPCSTR wallmarks_name = pSettings->r_string(section, "wallmarks"); 
	m_wallmarks->AppendMark(wallmarks_name);
}

float Distance (const Fvector& rkPoint, const Fvector rkTri[3], float& pfSParam, float& pfTParam, Fvector& closest, Fvector& dir)
{
	Fvector kDiff; kDiff.sub(rkTri[0], rkPoint);

	Fvector Edge0; Edge0.sub(rkTri[1], rkTri[0]);
	Fvector Edge1; Edge1.sub(rkTri[2], rkTri[0]);
    float fA00 = Edge0.square_magnitude();

    float fA01 = Edge0.dotproduct(Edge1);
    float fA11 = Edge1.square_magnitude();
    float fB0 = kDiff.dotproduct(Edge0);
	float fB1 = kDiff.dotproduct(Edge1);
    float fC = kDiff.square_magnitude();

    float fDet = _abs(fA00*fA11-fA01*fA01);

    float fS = fA01*fB1-fA11*fB0;
    float fT = fA01*fB0-fA00*fB1;
    float fSqrDist;

    if ( fS + fT <= fDet )
    {
        if ( fS < 0.0f )
        {
            if ( fT < 0.0f )  // region 4
            {
                if ( fB0 < 0.0f )
                {
                    fT = 0.0f;
                    if ( -fB0 >= fA00 )
                    {
                        fS = 1.0f;
                        fSqrDist = fA00+2.0f*fB0+fC;
                    }
                    else
                    {
                        fS = -fB0/fA00;
                        fSqrDist = fB0*fS+fC;
                    }
                }
                else
                {
                    fS = 0.0f;
                    if ( fB1 >= 0.0f )
                    {
                        fT = 0.0f;
                        fSqrDist = fC;
                    }
                    else if ( -fB1 >= fA11 )
                    {
                        fT = 1.0f;
                        fSqrDist = fA11+2.0f*fB1+fC;
                    }
                    else
                    {
                        fT = -fB1/fA11;
                        fSqrDist = fB1*fT+fC;
                    }
                }
            }
            else  // region 3
            {
                fS = 0.0f;
                if ( fB1 >= 0.0f )
                {
                    fT = 0.0f;
                    fSqrDist = fC;
                }
                else if ( -fB1 >= fA11 )
                {
                    fT = 1.0f;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fT = -fB1/fA11;
                    fSqrDist = fB1*fT+fC;
                }
            }
        }
        else if ( fT < 0.0f )  // region 5
        {
            fT = 0.0f;
            if ( fB0 >= 0.0f )
            {
                fS = 0.0f;
                fSqrDist = fC;
            }
            else if ( -fB0 >= fA00 )
            {
                fS = 1.0f;
                fSqrDist = fA00+2.0f*fB0+fC;
            }
            else
            {
                fS = -fB0/fA00;
                fSqrDist = fB0*fS+fC;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            float fInvDet = 1.0f/fDet;
            fS *= fInvDet;
            fT *= fInvDet;
            fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
        }
    }
    else
    {
        float fTmp0, fTmp1, fNumer, fDenom;

        if ( fS < 0.0f )  // region 2
        {
            fTmp0 = fA01 + fB0;
            fTmp1 = fA11 + fB1;
            if ( fTmp1 > fTmp0 )
            {
                fNumer = fTmp1 - fTmp0;
                fDenom = fA00-2.0f*fA01+fA11;
                if ( fNumer >= fDenom )
                {
                    fS = 1.0f;
                    fT = 0.0f;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else
                {
                    fS = fNumer/fDenom;
                    fT = 1.0f - fS;
                    fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                        fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                }
            }
            else
            {
                fS = 0.0f;
                if ( fTmp1 <= 0.0f )
                {
                    fT = 1.0f;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else if ( fB1 >= 0.0f )
                {
                    fT = 0.0f;
                    fSqrDist = fC;
                }
                else
                {
                    fT = -fB1/fA11;
                    fSqrDist = fB1*fT+fC;
                }
            }
        }
        else if ( fT < 0.0f )  // region 6
        {
            fTmp0 = fA01 + fB1;
            fTmp1 = fA00 + fB0;
            if ( fTmp1 > fTmp0 )
            {
                fNumer = fTmp1 - fTmp0;
                fDenom = fA00-2.0f*fA01+fA11;
                if ( fNumer >= fDenom )
                {
                    fT = 1.0f;
                    fS = 0.0f;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fT = fNumer/fDenom;
                    fS = 1.0f - fT;
                    fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                        fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                }
            }
            else
            {
                fT = 0.0f;
                if ( fTmp1 <= 0.0f )
                {
                    fS = 1.0f;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else if ( fB0 >= 0.0f )
                {
                    fS = 0.0f;
                    fSqrDist = fC;
                }
                else
                {
                    fS = -fB0/fA00;
                    fSqrDist = fB0*fS+fC;
                }
            }
        }
        else  // region 1
        {
            fNumer = fA11 + fB1 - fA01 - fB0;
            if ( fNumer <= 0.0f )
            {
                fS = 0.0f;
                fT = 1.0f;
                fSqrDist = fA11+2.0f*fB1+fC;
            }
            else
            {
                fDenom = fA00-2.0f*fA01+fA11;
                if ( fNumer >= fDenom )
                {
                    fS = 1.0f;
                    fT = 0.0f;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else
                {
                    fS = fNumer/fDenom;
                    fT = 1.0f - fS;
                    fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                        fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                }
            }
        }
    }

    pfSParam = fS;
    pfTParam = fT;

	closest.mad			(rkTri[0], Edge0, fS).mad(Edge1, fT);
	
	dir.sub				(closest, rkPoint);
	dir.normalize_safe	();
    return _sqrt		(_abs(fSqrDist));
}