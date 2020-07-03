#include "stdafx.h"
#include "mu_model_light_threads.h"
#include "xrface.h"
#include "xrMU_Model.h"
#include "xrMU_Model_Reference.h"
#include "xrlc_globaldata.h"
#include "mu_model_light.h"

CThreadManager mu_base(ProxyStatus, ProxyProgress);
CThreadManager mu_secondary(ProxyStatus, ProxyProgress);
#define		MU_THREADS	4
// mu-light
bool mu_models_local_calc_lightening = false;

xrCriticalSection		mu_models_local_calc_lightening_wait_lock;
void WaitMuModelsLocalCalcLightening()
{
	while(true)
	{
		bool complited = false;
		Sleep(1000);
		mu_models_local_calc_lightening_wait_lock.Enter();
		complited = mu_models_local_calc_lightening;

		mu_models_local_calc_lightening_wait_lock.Leave();
		if(complited)
			break;
	}
}

void SetMuModelsLocalCalcLighteningCompleted()
{
	xrCriticalSectionGuard guard(mu_models_local_calc_lightening_wait_lock);
	mu_models_local_calc_lightening = true;
}

class CMULight : public CThread
{
	u32			low;
	u32			high;
public:
	CMULight(u32 ID, u32 _low, u32 _high) : CThread(ID, ProxyMsg) { thMessages = FALSE; low = _low; high = _high; }

	virtual void Execute()
	{
		// Priority
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
		Sleep(0);

		// Light references
		for (u32 m = low; m<high; m++)
		{

			inlc_global_data()->mu_refs()[m]->calc_lighting();
			thProgress = (float(m - low) / float(high - low));
		}
	}
};


//void LC_WaitRefModelsNet();
class CMUThread : public CThread
{
public:
	CMUThread(u32 ID) : CThread(ID, ProxyMsg)
	{
		thMessages = FALSE;
	}
	virtual void Execute()
	{
		// Priority
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
		Sleep(0);

		// Light models
		for (u32 m = 0; m<inlc_global_data()->mu_models().size(); m++)
		{
			inlc_global_data()->mu_models()[m]->calc_materials();
			inlc_global_data()->mu_models()[m]->calc_lighting();
		}

		SetMuModelsLocalCalcLighteningCompleted();

		// Light references
		u32	stride = inlc_global_data()->mu_refs().size() / MU_THREADS;
		u32	last = inlc_global_data()->mu_refs().size() - stride * (MU_THREADS - 1);
		u32 threads = MU_THREADS;
		get_intervals(MU_THREADS, inlc_global_data()->mu_refs().size(), threads, stride, last);

		for (u32 thID = 0; thID<threads; thID++)
			mu_secondary.start(xr_new<CMULight>(thID, thID*stride, thID*stride + stride));
		if (last > 0)
			mu_secondary.start(xr_new<CMULight>(threads, threads*stride, threads*stride + last));
	}
};

void run_mu_base()
{
	mu_base.start(xr_new<CMUThread>(0));
}

void wait_mu_base_thread()
{
	mu_base.wait(500);
}
void wait_mu_secondary_thread()
{
	mu_secondary.wait(500);
}