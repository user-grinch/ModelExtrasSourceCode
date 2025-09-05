#include "pch.h"
#include "ganghands.h"
#include "texmgr.h"
#include <CPedModelInfo.h>

void GangHands::Initialize() {
    static bool alreadyLoaded = false;
    plugin::Events::initGameEvent += []
    {
        if (!alreadyLoaded) {
            alreadyLoaded = true;

            RwTexture *handsBlack = TextureMgr::Get("hands_black");
            RwTexture *handsWhite = TextureMgr::Get("hands_white");

            if (handsBlack && handsWhite)
            {
                injector::MakeInline<0x59EF79, 0x59EF82>([&](injector::reg_pack& regs)
                {
                    CPed* ped = *(CPed**)(regs.esp + 0x1C + 0x8);
                    CPedModelInfo* pedModelInfo = (CPedModelInfo*)CModelInfo::GetModelInfo(ped->m_nModelIndex);

                    if (pedModelInfo->m_nPedType == ePedType::PED_TYPE_GANG1 || pedModelInfo->m_nPedType == ePedType::PED_TYPE_GANG2)
                    {
                        // normally black people
                        regs.eax = (uint32_t)handsBlack;
                    }
                    else
                    {
                        //normally white people
                        regs.eax = (uint32_t)handsWhite;
                    }
                });
            }
        }
    };
}