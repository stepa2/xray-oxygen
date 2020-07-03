#include "stdafx.h"
#include "Dosimeter.h"
#include "UI/UIDosimeter.h"

void CDosimeter::Load(LPCSTR section)
{
    CHudItemObject::Load(section);

    m_sounds.LoadSound(section, "snd_draw", "sndShow");
    m_sounds.LoadSound(section, "snd_holster", "sndHide");
}

void CDosimeter::shedule_Update(u32 dt)
{
    CHudItemObject::shedule_Update(dt);

    if (!IsWorking())
        return;

    Position().set(H_Parent()->Position());
}

void CDosimeter::UpdateAf()
{
}

void CDosimeter::CreateUI()
{
    R_ASSERT(nullptr == m_ui);
    m_ui = xr_new<CUIDosimeter>();
    ui().construct(this);
}

CUIDosimeter& CDosimeter::ui()
{
    return *((CUIDosimeter*)m_ui);
}

bool CDosimeter::render_item_3d_ui_query()
{
    return IsWorking();
}

void CDosimeter::render_item_3d_ui()
{
    R_ASSERT(HudItemData());
    CCustomDetector::render_item_3d_ui();
    ui().Draw();
    //	Restore cull mode
    UIRender->CacheSetCullMode(IUIRender::cmCCW);
}