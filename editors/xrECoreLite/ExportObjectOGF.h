/*
* Authors:
* Date of creation:
* Description:
* Copyright:
*/

#ifndef ExportObjectOGFH
#define ExportObjectOGFH

#include "PropSlimTools.h"
//---------------------------------------------------------------------------
const int clpOGFMX = 48, clpOGFMY = 16, clpOGFMZ = 48;
//---------------------------------------------------------------------------
// refs
class CEditableObject;
class CEditableMesh;
class CSurface;
class CInifile;

struct SOGFVert
{
	Fvector P;
	Fvector N;
	Fvector2 UV;
	Fvector T;
	Fvector B;

	SOGFVert()
	{
		P.set(0, 0, 0);
		N.set(0, 1, 0);
		UV.set(0.f, 0.f);
		T.set(0, 0, 0);
		B.set(0, 0, 0);
	}

	void set(const Fvector& p, const Fvector& n, const Fvector2& uv)
	{
		P.set(p);
		N.set(n);
		UV.set(uv);
	}

	BOOL similar_pos(SOGFVert& V)
	{
		return P.similar(V.P, EPS_L);
	}

	BOOL similar(SOGFVert& V)
	{
		if (!P.similar(V.P, EPS_L))
			return 0;

		if (!UV.similar(V.UV, EPS_S))
			return 0;

		if (!N.similar(V.N, EPS_L))
			return 0;

		return 1;
	}
};

struct SOGFFace
{
	WORD v[3];
};

using OGFVertVec = xr_vector<SOGFVert>;
using OGFFaceVec = xr_vector<SOGFFace>;

class CObjectOGFCollectorPacked
{
public:
	//	Fobb			m_OBB;
	Fbox m_Box;

	OGFVertVec m_Verts;
	OGFFaceVec m_Faces;

	// Progressive
	ArbitraryList<VIPM_SWR>	m_SWR;// The records of the collapses.

	Fvector m_VMmin, m_VMscale;
	U32Vec m_VM[clpOGFMX + 1][clpOGFMY + 1][clpOGFMZ + 1];
	Fvector	m_VMeps;

	u16	VPack(SOGFVert& V);
	void ComputeBounding();
	void OptimizeTextureCoordinates();
public:
	CObjectOGFCollectorPacked(const Fbox &bb, int apx_vertices, int apx_faces);
	void CalculateTB();
	void MakeProgressive();
	inline bool check(SOGFFace& F)
	{
		if ((F.v[0] == F.v[1]) || (F.v[0] == F.v[2]) || (F.v[1] == F.v[2]))
			return false;

		/*        for (OGFFaceIt f_it=m_Faces.begin(); f_it!=m_Faces.end(); f_it++){
					// Test for 6 variations
					if ((f_it->v[0]==F.v[0]) && (f_it->v[1]==F.v[1]) && (f_it->v[2]==F.v[2])) return false;
					if ((f_it->v[0]==F.v[0]) && (f_it->v[2]==F.v[1]) && (f_it->v[1]==F.v[2])) return false;
					if ((f_it->v[2]==F.v[0]) && (f_it->v[0]==F.v[1]) && (f_it->v[1]==F.v[2])) return false;
					if ((f_it->v[2]==F.v[0]) && (f_it->v[1]==F.v[1]) && (f_it->v[0]==F.v[2])) return false;
					if ((f_it->v[1]==F.v[0]) && (f_it->v[0]==F.v[1]) && (f_it->v[2]==F.v[2])) return false;
					if ((f_it->v[1]==F.v[0]) && (f_it->v[2]==F.v[1]) && (f_it->v[0]==F.v[2])) return false;
				}
		*/

		return true;
	}
	inline bool			add_face(SOGFVert& v0, SOGFVert& v1, SOGFVert& v2)
	{
		if (v0.P.similar(v1.P, EPS) || v0.P.similar(v2.P, EPS) || v1.P.similar(v2.P, EPS))
		{
			Msg("! Degenerate face found. Removed.");
			return true;
		}
		SOGFFace F;
		u16 v;
		v = VPack(v0);
		if (0xffff == v)
			return false; F.v[0] = v;

		v = VPack(v1);
		if (0xffff == v)
			return false; F.v[1] = v;

		v = VPack(v2);
		if (0xffff == v)
			return false; F.v[2] = v;

		if (check(F))
			m_Faces.push_back(F);
		else
		{
			Msg("! Duplicate(degenerate) face found. Removed.");
			return true;
		}

		return true;
	}
	inline OGFVertVec& getV_Verts() { return m_Verts; }
	inline OGFFaceVec& getV_Faces() { return m_Faces; }
	inline SOGFVert* getVert() { return m_Verts.data(); }
	inline int getVS() { return int(m_Verts.size()); }
	inline int getTS() { return int(m_Faces.size()); }
};
//----------------------------------------------------
using COGFCPVec = xr_vector<CObjectOGFCollectorPacked*>;

class ECORE_API CExportObjectOGF
{
	struct SSplit
	{
		Fbox apx_box;

		COGFCPVec m_Parts;
		CObjectOGFCollectorPacked* m_CurrentPart;

		Fbox m_Box;
		CSurface* m_Surf;

		// Progressive
		void AppendPart(int apx_vertices, int apx_faces);
		void SavePart(IWriter& F, CObjectOGFCollectorPacked* part);
		void Save(IWriter& F, int& chunk_id);

		void CalculateTB()
		{
			for (auto it = m_Parts.begin(); it != m_Parts.end(); ++it)
				(*it)->CalculateTB();
		}

		void MakeProgressive();
		SSplit(CSurface* surf, const Fbox& bb);
		~SSplit();
		void ComputeBounding()
		{
			m_Box.invalidate();
			for (auto it = m_Parts.begin(); it != m_Parts.end(); ++it)
			{
				CObjectOGFCollectorPacked* part = *it;
				part->ComputeBounding();
				m_Box.merge(part->m_Box);
			}
		}
	};

	using SplitVec = xr_vector<SSplit*>;
	SplitVec m_Splits;
	CEditableObject* m_Source;
	Fbox m_Box;
	//----------------------------------------------------
	//	void 	ComputeOBB			(Fobb &B, FvectorVec& V);
	SSplit*	FindSplit(CSurface* surf);
	void ComputeBounding()
	{
		m_Box.invalidate();
		for (auto it = m_Splits.begin(); it != m_Splits.end(); ++it)
		{
			(*it)->ComputeBounding();
			m_Box.merge((*it)->m_Box);
		}
	}
	bool PrepareMESH(CEditableMesh* mesh);
	bool Prepare(bool gen_tb, CEditableMesh* mesh);

public:
	CExportObjectOGF(CEditableObject* object);
	~CExportObjectOGF();
	bool Export(IWriter& F, bool gen_tb = true, CEditableMesh* mesh = NULL);
	bool ExportAsSimple(IWriter& F);
	bool ExportAsWavefrontOBJ(IWriter& F, LPCSTR fn);
};

#endif
