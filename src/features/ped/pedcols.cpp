#include "pch.h"
#include "pedcols.h"
#include <vector>
#include <RenderWare.h>
#include <rw/rpworld.h>
#include <rw/rwplcore.h>
#include "datamgr.h"


#define RwRGBAGetRGB(a) (*(DWORD *)&(a) & 0xFFFFFF)
std::unordered_map<CPed*, std::vector<std::pair<void *, int>>> store;

void PedColors::SetEditableMaterials(RpClump *pClump) {
	RpClumpForAllAtomics(pClump, [](RpAtomic * pAtomic, void *data) {
		if (rwObjectGetFlags(pAtomic) & rpATOMICRENDER) {
			RpGeometryForAllMaterials(pAtomic->geometry, [](RpMaterial *pMaterial, void* data) {
				if (m_pCurrentPed) {
					int idx;
					auto &data = m_PedData.Get(m_pCurrentPed);
					switch (RwRGBAGetRGB(pMaterial->color))
					{
					case 0x00FF3C:
						idx = 0;
						break;
					case 0xAF00FF:
						idx = 1;
						break;
					case 0xFFFF00:
						idx = 2;
						break;
					case 0xFF00FF:
						idx = 3;
						break;
					default:
						return pMaterial;
					}
					store[m_pCurrentPed].push_back(std::make_pair(&pMaterial->color, *reinterpret_cast<int *>(&pMaterial->color)));
					pMaterial->color.red = data.m_Colors[idx].r;
					pMaterial->color.green = data.m_Colors[idx].g;
					pMaterial->color.blue = data.m_Colors[idx].b;
				}

				return pMaterial;
			}, NULL);

			pAtomic->geometry->flags |= rpGEOMETRYMODULATEMATERIALCOLOR;
		}
		return pAtomic;
	}, NULL);
}

void PedData::Init(CPed *pPed) {
	uint32_t model = pPed->m_nModelIndex;
	auto jsonData = DataMgr::Get(model);
	if (jsonData.contains("pedcols")) {
		const std::vector<std::string> keys = { "primary", "secondary", "tertiary", "quaternary" };
		for (size_t i = 0; i < keys.size(); ++i) {
			const auto& colArray = jsonData["pedcols"][keys[i]];
			if (!colArray.empty()) {
				const auto& rgb = colArray[plugin::RandomNumberInRange<size_t>(0, colArray.size() - 1)];

				if (rgb.size() == 3) {
					m_Colors[i] = CRGBA(rgb[0], rgb[1], rgb[2]);
				}
			}
		}
		m_bUsingPedCols = true;
	}
}

void PedColors::Initialize() {
	plugin::Events::pedSetModelEvent.after += [](CPed *pPed, int model) {
		auto &data = m_PedData.Get(pPed);
		if (!data.m_bInitialized) {
			data.Init(pPed);
			data.m_bInitialized = true;
		}
	};

	plugin::Events::pedRenderEvent.before += [](CPed *pPed) {
		auto &data = m_PedData.Get(pPed);
		if (data.m_bUsingPedCols) {
			m_pCurrentPed = pPed;
			SetEditableMaterials(pPed->m_pRwClump);
		}
	};

	plugin::Events::pedRenderEvent.after += [](CPed *pPed) {
		for (auto &e : store[pPed]) {
			*static_cast<int *>(e.first) = e.second;
		}
		store[pPed].clear();
	};
}