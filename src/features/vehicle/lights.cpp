#include "pch.h"
#include "lights.h"
#include <CClock.h>
#include "defines.h"
#include <CShadows.h>
#include <eVehicleClass.h>
#include <CCutsceneMgr.h>
#include <rwcore.h>
#include <rpworld.h>
#include "spotlights.h"
#include "../audiomgr.h"
#include <CWeather.h>
#include <CCoronas.h>
#include "../../enums/vehdummy.h"
#include "datamgr.h"
#include "core/colors.h"
#include <CPointLights.h>
#include <CVisibilityPlugins.h>

// flags
bool gbGlobalIndicatorLights = false;
bool gbGlobalReverseLights = false;
float gfGlobalCoronaSize = 0.3f;
int gGlobalCoronaIntensity = 80;
int gGlobalShadowIntensity = 80;
CVector2D shdwOffset = {0.0f, 0.9f};
CVector2D headlightOffset = {0.0f, shdwOffset.y + 0.5f};
CVector2D headlightSz = {4.0f, 8.0f};

bool IsNightTime()
{
	return CClock::GetIsTimeInRange(20, 6);
}

bool IsTailLightOn(CVehicle *pVeh)
{
	return IsNightTime() || pVeh->m_nOverrideLights == eLightOverride::ForceLightsOn || pVeh->m_nVehicleFlags.bLightsOn;
}

bool IsEngineOff(CVehicle *pVeh)
{
	return !pVeh->m_nVehicleFlags.bEngineOn || pVeh->m_nVehicleFlags.bEngineBroken;
}

int GetStrobeIndex(CVehicle *pVeh, RpMaterial *pMat) {
	return pMat->color.blue;
}

bool IsOkAtomicVisible(RwFrame* frame)
{
	if (!rwLinkListEmpty(&frame->objectList))
    {
        RwObjectHasFrame *atomic;

        RwLLLink *current = rwLinkListGetFirstLLLink(&frame->objectList);
        RwLLLink *end = rwLinkListGetTerminator(&frame->objectList);

        do
        {
            atomic = rwLLLinkGetData(current, RwObjectHasFrame, lFrame);
            bool isOkAtomic = (CVisibilityPlugins::GetAtomicId((RpAtomic*)atomic) & 3) == 1; // 1 = Ok, 2 = Damaged, 3 = None

			if (isOkAtomic) {
				return atomic->object.flags & rpATOMICRENDER;
			}

            current = rwLLLinkGetNext(current);
        } while (current != end);
    }

    return true;
}

// Indicator lights
static uint64_t delay;

CVector2D GetCarPathLinkPosition(CCarPathLinkAddress &address)
{
	if (address.m_nAreaId != -1 && address.m_nCarPathLinkId != -1 && ThePaths.m_pPathNodes[address.m_nAreaId])
	{
		return CVector2D(static_cast<float>(ThePaths.m_pNaviNodes[address.m_nAreaId][address.m_nCarPathLinkId].m_vecPosn.x) / 8.0f,
						 static_cast<float>(ThePaths.m_pNaviNodes[address.m_nAreaId][address.m_nCarPathLinkId].m_vecPosn.y) / 8.0f);
	}
	return CVector2D(0.0f, 0.0f);
}

void DrawGlobalLight(CVehicle *pVeh, bool isRear, bool isLeft, CRGBA col, std::string texture = "indicator",
					 CVector2D shdwSz = {1.0F, 1.0F}, CVector2D shdwOffset = {0.0F, 0.0F})
{
	if (pVeh->m_nVehicleSubClass == VEHICLE_AUTOMOBILE)
	{
		CAutomobile *car = reinterpret_cast<CAutomobile *>(pVeh);
		bool broken = (isLeft && car->m_damageManager.GetLightStatus(eLights::LIGHT_FRONT_LEFT)) || (!isLeft && car->m_damageManager.GetLightStatus(eLights::LIGHT_FRONT_RIGHT));
		if (broken)
		{
			return;
		}
	}

	int dummyIdx = (isRear) ? 1 : 0;

	CVehicleModelInfo *pInfo = reinterpret_cast<CVehicleModelInfo *>(CModelInfo__ms_modelInfoPtrs[pVeh->m_nModelIndex]);
	CVector posn = pInfo->m_pVehicleStruct->m_avDummyPos[dummyIdx];

	if (posn.x == 0.0f)
	{
		posn.x = 0.15f;
	}

	if (isLeft)
	{
		posn.x *= -1.0f;
	}

	int dummyId = dummyIdx + (isLeft ? 0 : 2);
	float dummyAngle = isRear ? 180.0f : 0.0f;

	CRGBA shadowColor = {col.r, col.g, col.b, static_cast<unsigned char>(gGlobalShadowIntensity)};
	RenderUtil::RegisterShadow(pVeh, posn, shadowColor, dummyAngle, isRear ? eDummyPos::Rear : eDummyPos::Front, texture, shdwSz, shdwOffset);

	CRGBA coronaColor = {col.r, col.g, col.b, static_cast<unsigned char>(gGlobalShadowIntensity)};
	int coronaId = reinterpret_cast<uintptr_t>(pVeh) + 255 * isRear + 128 * isLeft + col.r + col.g + col.b;
	RenderUtil::RegisterCoronaWithAngle(pVeh, coronaId, posn, coronaColor, dummyAngle, 180.0f, gfGlobalCoronaSize);
}

inline float GetZAngleForPoint(CVector2D const &point)
{
	float angle = CGeneral::GetATanOfXY(point.x, point.y) * 57.295776f - 90.0f;
	while (angle < 0.0f)
		angle += 360.0f;
	return angle;
}

void Lights::Initialize()
{
	m_bEnabled = true;
	patch::Nop(0x6E2722, 19);	  // CVehicle::DoHeadLightReflection
	patch::SetUChar(0x6E1A22, 0); // CVehicle::DoTailLightEffect

	injector::MakeInline<0x6E2870, 0x6E2870+6>([](injector::reg_pack &regs) {
		CVehicle *pVeh = reinterpret_cast<CVehicle*>(regs.esi);
		if (pVeh && pVeh->m_nVehicleSubClass == VEHICLE_AUTOMOBILE)
		{
			CAutomobile *pAutomobile = reinterpret_cast<CAutomobile *>(pVeh);
			if (pAutomobile->m_damageManager.GetDoorStatus(eDoors::BOOT))
			{
				regs.eax = 0;
			}
		} else {
			regs.eax = 1;
		}
	});

	plugin::Events::initGameEvent += []()
	{
		gbGlobalIndicatorLights = gConfig.ReadBoolean("VEHICLE_FEATURES", "StandardLights_GlobalIndicatorLights", false);
		gbGlobalReverseLights = gConfig.ReadBoolean("VEHICLE_FEATURES", "StandardLights_GlobalReverseLights", false);
		
		gfGlobalCoronaSize = gConfig.ReadFloat("VISUAL", "LightCoronaSize", 0.3f);
		gGlobalShadowIntensity = gConfig.ReadInteger("VISUAL", "LightShadowIntensity", 220);
		gGlobalCoronaIntensity = gConfig.ReadInteger("VISUAL", "LightCoronaIntensity", 250);
	};

	Events::vehicleDtorEvent += [](CVehicle *pVeh)
	{
		m_Dummies.erase(pVeh);
	};


	ModelInfoMgr::RegisterMaterial([](CVehicle *pVeh, RpMaterial *pMat){
		if (!m_bEnabled) {
			return eLightType::UnknownLight;
		}
		// Headlights
		CRGBA matCol = *reinterpret_cast<CRGBA *>(RpMaterialGetColor(pMat));
		matCol.a = 255;
		if (matCol == VEHCOL_HEADLIGHT_LEFT) {
			return eLightType::HeadLightLeft;
		} else if (matCol == VEHCOL_HEADLIGHT_RIGHT) {
			return eLightType::HeadLightRight;
		}
		// Taillights
		else if (matCol == VEHCOL_TAILLIGHT_LEFT) {
			return eLightType::TailLightLeft;
		} else if (matCol == VEHCOL_TAILLIGHT_RIGHT) {
			return eLightType::TailLightRight;
		}
		// Reverse Lights
		else if (matCol == VEHCOL_REVERSELIGHT_LEFT) {
			return eLightType::ReverseLightLeft;
		}
		else if (matCol == VEHCOL_REVERSELIGHT_RIGHT) {
			return eLightType::ReverseLightRight;
		}
		// Brake Lights
		else if (matCol == VEHCOL_BRAKELIGHT_LEFT) {
			return eLightType::BrakeLightLeft;
		}
		else if (matCol == VEHCOL_BRAKELIGHT_RIGHT) {
			return eLightType::BrakeLightRight;
		}
		// All Day Lights
		else if (matCol == VEHCOL_ALLDAYLIGHT_1 || matCol == VEHCOL_ALLDAYLIGHT_2) {
			return eLightType::AllDayLight;
		}
		// Day Lights
		else if (matCol == VEHCOL_DAYLIGHT_1 || matCol == VEHCOL_DAYLIGHT_2) {
			return eLightType::DayLight;
		}
		// Night Lights
		else if (matCol == VEHCOL_NIGHTLIGHT_1 || matCol == VEHCOL_NIGHTLIGHT_2) {
			return eLightType::NightLight;
		}
		// Fog Lights
		else if (matCol == VEHCOL_FOGLIGHT_LEFT) {
			return eLightType::FogLightLeft;
		}
		else if (matCol == VEHCOL_FOGLIGHT_RIGHT) {
			return eLightType::FogLightRight;
		}
		// Sidelights
		else if (matCol == VEHCOL_SIDELIGHT_LEFT) {
			return eLightType::SideLightLeft;
		} else if (matCol == VEHCOL_SIDELIGHT_RIGHT) {
			return eLightType::SideLightRight;
		}
		// STT Lights
		else if (matCol == VEHCOL_STTLIGHT_LEFT) {
			return eLightType::STTLightLeft;
		} else if (matCol == VEHCOL_STTLIGHT_RIGHT) {
			return eLightType::STTLightRight;
		}
		// NA Brake Lights
		else if (matCol == VEHCOL_NABRAKE_LEFT) {
			return eLightType::NABrakeLightLeft;
		} else if (matCol == VEHCOL_NABRAKE_RIGHT) {
			return eLightType::NABrakeLightRight;
		}
		// Spot and Strobe Lights
		else if (matCol == VEHCOL_SPOTLIGHT) {
			return eLightType::SpotLight;
		} else if (matCol == VEHCOL_STROBELIGHT) {
			return eLightType::StrobeLight;
		}
		// Indicator Lights (Left)
		if (matCol == VEHCOL_INDICATOR_LEFT_REAR) {
			return eLightType::IndicatorLightLeftRear;
		}
		else if (matCol == VEHCOL_INDICATOR_LEFT_SIDE) {
			return eLightType::IndicatorLightLeftMiddle;
		}
		else if (matCol == VEHCOL_INDICATOR_LEFT_FRONT) {
			return eLightType::IndicatorLightLeftFront;
		}
		// Indicator Lights (Right)
		else if (matCol == VEHCOL_INDICATOR_RIGHT_REAR) {
			return eLightType::IndicatorLightRightRear;
		}
		else if (matCol == VEHCOL_INDICATOR_RIGHT_SIDE) {
			return eLightType::IndicatorLightRightMiddle;
		}
		else if (matCol == VEHCOL_INDICATOR_RIGHT_FRONT) {
			return eLightType::IndicatorLightRightFront;
		}
		// If no match is found
		return eLightType::UnknownLight;
	});

	ModelInfoMgr::RegisterDummy([](CVehicle *pVeh, RwFrame *frame) {
		std::string name = GetFrameNodeName(frame);
		VehicleDummyConfig c = {
			.pVeh = pVeh,
			.frame = frame,
			.corona = {
				.color = {255, 255, 255, static_cast<unsigned char>(gGlobalCoronaIntensity)}
			}
		};

		auto &dummies = m_Dummies[pVeh];

		if (name.starts_with("fogl") && (STR_FOUND(name, "_l") || STR_FOUND(name, "_r"))) {
			c.dummyType = eDummyPos::Front;
			c.lightType = STR_FOUND(name, "_l") ? eLightType::FogLightLeft : eLightType::FogLightRight;
			c.corona.color = c.shadow.color = {255, 255, 255, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional;
		}
		else if (name.starts_with("rev") && (STR_FOUND(name, "_l") || STR_FOUND(name, "_r"))) {
			c.dummyType = eDummyPos::Rear;
			c.lightType = STR_FOUND(name, "_l") ? eLightType::ReverseLightLeft : eLightType::ReverseLightRight;
			c.corona.color = c.shadow.color = {255, 255, 255, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional;
		}
		else if (name.starts_with("breakl") && (STR_FOUND(name, "_l") || STR_FOUND(name, "_r"))) {
			c.dummyType = eDummyPos::Rear;
			c.lightType = STR_FOUND(name, "_l") ? eLightType::BrakeLightLeft : eLightType::BrakeLightRight;
			c.corona.color = c.shadow.color = {240, 0, 0, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional;
		}
		else if (name.starts_with("light_day")) {
			c.lightType = eLightType::DayLight;
			c.dummyType = eDummyPos::Front;
		}
		else if (name.starts_with("light_nigh")) {
			c.lightType = eLightType::NightLight;
			c.dummyType = eDummyPos::Front;
		}
		else if (auto d = Util::GetDigitsAfter(name, "strobe_light")) {
			c.lightType = eLightType::StrobeLight;
			c.dummyType = eDummyPos::Front;
			c.dummyIdx = d.value();
		}
		else if (auto d = Util::GetCharsAfterPrefix(name, "sidelight_", 1)) {
			if (d == "L") {
				c.lightType = eLightType::SideLightLeft;
				c.dummyType = eDummyPos::Left;
			} else {
				c.lightType = eLightType::SideLightRight;
				c.dummyType = eDummyPos::Right;
			}
		}
		else if (auto d = Util::GetCharsAfterPrefix(name, "sttlight_", 1)) {
			c.lightType = (d == "L") ? eLightType::STTLightLeft : eLightType::STTLightRight;
			c.dummyType = eDummyPos::Rear;
			c.corona.color = c.shadow.color = {240, 0, 0, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional;
		}
		else if (auto d = Util::GetCharsAfterPrefix(name, "nabrakelight_", 1)) {
			c.lightType = (d == "L") ? eLightType::NABrakeLightLeft : eLightType::NABrakeLightRight;
			c.dummyType = eDummyPos::Rear;
			c.corona.color = c.shadow.color = {240, 0, 0, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional;
		}
		else if (name.starts_with("spotlight_light")) {
			c.lightType = eLightType::SpotLight;
		}
		else if (name.starts_with("light_allday")) {
			c.lightType = eLightType::AllDayLight;
			c.dummyType = eDummyPos::Front;
		}
		else if (name.starts_with("taillights")) {
			c.dummyType = eDummyPos::Rear;
			c.lightType = eLightType::TailLightLeft;
			c.corona.color = c.shadow.color = {250, 0, 0, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional; 				
			c.shadow.render = name != "taillights2";
			c.mirroredX = true;
			dummies[c.lightType].push_back(new VehicleDummy(c));
			c.mirroredX = false;
			c.lightType = eLightType::TailLightRight;
		}
		else if (name.starts_with("headlights")) {
			c.dummyType = eDummyPos::Front;
			c.lightType = eLightType::HeadLightLeft;
			c.corona.color = c.shadow.color = {250, 250, 250, static_cast<unsigned char>(gGlobalCoronaIntensity)};
			c.corona.lightingType = eLightingMode::Directional;
			c.shadow.render = name != "headlights2";
			c.mirroredX = true;
			dummies[c.lightType].push_back(new VehicleDummy(c));
			c.mirroredX = false;
			c.lightType = eLightType::HeadLightRight;
		}
		else if (name.starts_with("turnl_") || name.starts_with("indicator_")) {
			auto d = Util::GetCharsAfterPrefix(name, "turnl_", 2);
			if (!d) d = Util::GetCharsAfterPrefix(name, "indicator_", 2);
			if (d) {
				bool isLeft = (d.value()[0] == 'L');
				c.corona.color = c.shadow.color = {255, 128, 0, static_cast<unsigned char>(gGlobalCoronaIntensity)};
				c.corona.lightingType = eLightingMode::Directional;

				switch (d.value()[1]) {
					case 'F':
						c.lightType = isLeft ? eLightType::IndicatorLightLeftFront : eLightType::IndicatorLightRightFront;
						c.dummyType = eDummyPos::Front;
						break;
					case 'R':
						c.lightType = isLeft ? eLightType::IndicatorLightLeftRear : eLightType::IndicatorLightRightRear;
						c.dummyType = eDummyPos::Rear;
						break;
					case 'M':
						c.lightType = isLeft ? eLightType::IndicatorLightLeftMiddle : eLightType::IndicatorLightRightMiddle;
						c.dummyType = isLeft ? eDummyPos::Right : eDummyPos::Left;
						break;
				}
			}
		}
		else {
			return;
		}

		dummies[c.lightType].push_back(new VehicleDummy(c));
	});


	Events::processScriptsEvent += []()
	{
		size_t timestamp = CTimer::m_snTimeInMilliseconds;
		if ((timestamp - delay) > 500)
		{
			delay = timestamp;
			indicatorsDelay = !indicatorsDelay;
		}

		CVehicle *pVeh = FindPlayerVehicle(-1, false);
		if (pVeh && pVeh->m_nOverrideLights != eLightOverride::ForceLightsOff && !IsEngineOff(pVeh))
		{
			static size_t prev = 0;
			static uint32_t fogLightKey = gConfig.ReadInteger("KEYS", "FogLightKey", VK_J);
			if (KeyPressed(fogLightKey) && IsDummyAvail(pVeh, {eLightType::FogLightLeft, eLightType::FogLightRight}))
			{
				size_t now = CTimer::m_snTimeInMilliseconds;
				if (now - prev > 500.0f)
				{
					VehData &data = m_VehData.Get(pVeh);
					data.m_bFogLightsOn = !data.m_bFogLightsOn;
					prev = now;
					AudioMgr::PlaySwitchSound(pVeh);
				}
			}

			static uint32_t longLightKey = gConfig.ReadInteger("KEYS", "LongLightKey", VK_G);
			if (KeyPressed(longLightKey) && pVeh->m_nVehicleFlags.bLightsOn)
			{
				size_t now = CTimer::m_snTimeInMilliseconds;
				if (now - prev > 500.0f)
				{
					VehData &data = m_VehData.Get(pVeh);
					data.m_bLongLightsOn = !data.m_bLongLightsOn;
					prev = now;
					AudioMgr::PlaySwitchSound(pVeh);
				}
			}
		}
	};

	Events::vehicleRenderEvent += [](CVehicle *pVeh) {
		if (pVeh->m_nVehicleFlags.bLightsOn && !CModelInfo::IsTrailerModel(pVeh->m_nModelIndex))
		{
			if (pVeh->m_renderLights.m_bLeftFront || pVeh->m_renderLights.m_bRightFront) {
			}
		}
	};

	Events::processScriptsEvent += []() {
		auto pool = CPools::ms_pVehiclePool;

		for (CVehicle *pVeh : pool) {
			if (pVeh->m_pDriver == FindPlayerPed()) {
				continue;
			}
			CVector vehPos = pVeh->GetPosition();
			CVector camPos = TheCamera.GetPosition();

			if (DistanceBetweenPoints(vehPos, camPos) < 50.0f) {
				RenderHeadlights(pVeh, false);
			}
		}
	};

	ModelInfoMgr::RegisterRender([](CVehicle *pControlVeh)
								{
		int model = pControlVeh->m_nModelIndex;
		
		// skip directly processing trailers
		if (CModelInfo::IsTrailerModel(model)) {
			return;
		}

		CVehicle *pTowedVeh = pControlVeh;
		
		if (pControlVeh->m_pTrailer)
		{
			pTowedVeh = pControlVeh->m_pTrailer;
		}

		if (pControlVeh->m_nOverrideLights == eLightOverride::ForceLightsOff || pControlVeh->ms_forceVehicleLightsOff )
		{
			return;
		}
		
		if (pControlVeh->m_nOverrideLights == eLightOverride::ForceLightsOn) {
			pControlVeh->m_nVehicleFlags.bLightsOn = true;
			pControlVeh->m_renderLights.m_bLeftFront = true;
			pControlVeh->m_renderLights.m_bRightFront = true;
			if (pTowedVeh) {
				pTowedVeh->m_nVehicleFlags.bLightsOn = true;
				pTowedVeh->m_renderLights.m_bLeftRear = true;
				pTowedVeh->m_renderLights.m_bRightRear = true;
			}
		}

		VehData &data = m_VehData.Get(pControlVeh);
		eIndicatorState indState = data.m_nIndicatorState;

		// Fix for park car alarm lights
		if (pControlVeh->m_fHealth == 0 || (IsEngineOff(pControlVeh) && pControlVeh->m_nOverrideLights != eLightOverride::ForceLightsOn))
		{
			return;
		}

		CAutomobile *pAutoMobile = reinterpret_cast<CAutomobile *>(pControlVeh);
		RenderLights(pControlVeh, pTowedVeh, eLightType::AllDayLight);
		RenderLights(pControlVeh, pTowedVeh, eLightType::StrobeLight);
		RenderLights(pControlVeh, pTowedVeh, eLightType::SideLightLeft);
		RenderLights(pControlVeh, pTowedVeh, eLightType::SideLightRight);
		
		if (IsNightTime())
		{
			RenderLights(pControlVeh, pTowedVeh, eLightType::NightLight);
		}
		else
		{
			RenderLights(pControlVeh, pTowedVeh, eLightType::DayLight);
		}
		
		if (data.m_bFogLightsOn)
		{
			RenderLights(pControlVeh, pTowedVeh, eLightType::FogLightLeft, true, "foglight", {3.0f, 7.0f}, headlightOffset);
			RenderLights(pControlVeh, pTowedVeh, eLightType::FogLightRight, true, "foglight", {3.0f, 7.0f}, headlightOffset);
		}

		bool isBike = CModelInfo::IsBikeModel(pControlVeh->m_nModelIndex);

		if (pControlVeh->m_pDriver == FindPlayerPed()) {
			RenderHeadlights(pControlVeh);
		}

		if (SpotLights::IsEnabled(pControlVeh))
		{
			RenderLights(pControlVeh, pTowedVeh, eLightType::SpotLight, false);
		}


		std::string shdwName = (isBike ? "taillight_bike" : "taillight");
		CVector2D shdwSz = {1.0f, 1.5f};
		if (isBike || CModelInfo::IsCarModel(pControlVeh->m_nModelIndex))
		{
			bool isRevlightSupportedByModel = IsDummyAvail(pTowedVeh, {eLightType::ReverseLightLeft, eLightType::ReverseLightRight});

			bool reverseLightsOn = !isBike && (isRevlightSupportedByModel || gbGlobalReverseLights) && pControlVeh->m_nCurrentGear == 0 && (Util::GetVehicleSpeed(pControlVeh) >= 0.001f) && pControlVeh->m_pDriver;
			if (reverseLightsOn)
			{
				if (isRevlightSupportedByModel)
				{
					RenderLights(pControlVeh, pTowedVeh, eLightType::ReverseLightLeft, true, shdwName, shdwSz, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::ReverseLightRight, true, shdwName, shdwSz, shdwOffset);
				}
				else
				{
					DrawGlobalLight(pTowedVeh, true, true, {240, 240, 240, 0}, shdwName, shdwSz, shdwOffset);
					DrawGlobalLight(pTowedVeh, true, false, {240, 240, 240, 0}, shdwName, shdwSz, shdwOffset);
				}
			}

			bool sttInstalled = IsDummyAvail(pTowedVeh, {eLightType::STTLightLeft, eLightType::STTLightRight});
			// taillights/ brakelights
			if (pControlVeh->m_fBreakPedal && pControlVeh->m_pDriver)
			{
				if (sttInstalled) {
					RenderLights(pControlVeh, pTowedVeh, eLightType::STTLightLeft, true, shdwName, shdwSz, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::STTLightRight, true, shdwName, shdwSz, shdwOffset);
				} else {
					if (IsDummyAvail(pTowedVeh, {eLightType::BrakeLightLeft, eLightType::BrakeLightRight}))
					{
						RenderLights(pControlVeh, pTowedVeh, eLightType::BrakeLightLeft, true, shdwName, shdwSz, shdwOffset);
						RenderLights(pControlVeh, pTowedVeh, eLightType::BrakeLightRight, true, shdwName, shdwSz, shdwOffset);
					}
					else if (IsDummyAvail(pTowedVeh, {eLightType::TailLightLeft, eLightType::TailLightRight}))
					{
						RenderLights(pControlVeh, pTowedVeh, eLightType::TailLightLeft, true, shdwName, shdwSz, shdwOffset);
						RenderLights(pControlVeh, pTowedVeh, eLightType::TailLightRight, true, shdwName, shdwSz, shdwOffset);
					}
				}

				if (indState != eIndicatorState::BothOn)
				{
					if (indState != eIndicatorState::LeftOn)
					{
						RenderLights(pControlVeh, pTowedVeh, eLightType::NABrakeLightLeft, true, shdwName, shdwSz, shdwOffset);
					}

					if (indState != eIndicatorState::RightOn)
					{
						RenderLights(pControlVeh, pTowedVeh, eLightType::NABrakeLightRight, true, shdwName, shdwSz, shdwOffset);
					}
				}
			}

			if (IsTailLightOn(pControlVeh))
			{
				if (sttInstalled) {
					RenderLights(pControlVeh, pTowedVeh, eLightType::STTLightLeft, true, shdwName, shdwSz, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::STTLightRight, true, shdwName, shdwSz, shdwOffset);
				}
				else {
					if (IsDummyAvail(pTowedVeh, {eLightType::TailLightLeft, eLightType::TailLightRight}))
					{
						RenderLights(pControlVeh, pTowedVeh, eLightType::TailLightLeft, !sttInstalled, shdwName, shdwSz, shdwOffset);
						RenderLights(pControlVeh, pTowedVeh, eLightType::TailLightRight, !sttInstalled, shdwName, shdwSz, shdwOffset);
					}
					else if (IsDummyAvail(pTowedVeh, {eLightType::BrakeLightLeft, eLightType::BrakeLightRight}))
					{
						RenderLights(pControlVeh, pTowedVeh, eLightType::BrakeLightLeft, !sttInstalled, shdwName, shdwSz, shdwOffset);
						RenderLights(pControlVeh, pTowedVeh, eLightType::BrakeLightRight, !sttInstalled, shdwName, shdwSz, shdwOffset);
					}
				}
			}
		}

			// Indicator Lights
			if (!gbGlobalIndicatorLights && !IsDummyAvail(pControlVeh, INDICATOR_LIGHTS_TYPE))
			{
				return;
			}

			if (CCutsceneMgr::ms_running || TheCamera.m_bWideScreenOn)
			{
				return;
			}

			if (pControlVeh->m_pDriver == FindPlayerPed() &&
				(pControlVeh->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || pControlVeh->m_nVehicleSubClass == VEHICLE_BIKE || pControlVeh->m_nVehicleSubClass == VEHICLE_QUAD || pControlVeh->m_nVehicleSubClass == VEHICLE_MTRUCK))
			{
				static uint32_t indicatorNoneKey = gConfig.ReadInteger("KEYS", "IndicatorLightNoneKey", VK_SHIFT);
				static uint32_t indicatorLeftKey = gConfig.ReadInteger("KEYS", "IndicatorLightLeftKey", VK_Z);
				static uint32_t indicatorRightKey = gConfig.ReadInteger("KEYS", "IndicatorLightRightKey", VK_C);
				static uint32_t indicatorBothKey = gConfig.ReadInteger("KEYS", "IndicatorLightBothKey", VK_X);

				if (KeyPressed(indicatorNoneKey))
				{
					data.m_nIndicatorState = eIndicatorState::Off;
					delay = 0;
					indicatorsDelay = false;
				}

				if (KeyPressed(indicatorLeftKey))
				{
					data.m_nIndicatorState = eIndicatorState::LeftOn;
				}

				if (KeyPressed(indicatorRightKey))
				{
					data.m_nIndicatorState = eIndicatorState::RightOn;
				}

				if (KeyPressed(indicatorBothKey))
				{
					data.m_nIndicatorState = eIndicatorState::BothOn;
				}
			}
			else if (pControlVeh->m_pDriver)
			{
				data.m_nIndicatorState = eIndicatorState::Off;
				CVector2D prevPoint = GetCarPathLinkPosition(pControlVeh->m_autoPilot.m_nPreviousPathNodeInfo);
				CVector2D currPoint = GetCarPathLinkPosition(pControlVeh->m_autoPilot.m_nCurrentPathNodeInfo);
				CVector2D nextPoint = GetCarPathLinkPosition(pControlVeh->m_autoPilot.m_nNextPathNodeInfo);

				float angle = GetZAngleForPoint(nextPoint - currPoint) - GetZAngleForPoint(currPoint - prevPoint);
				while (angle < 0.0f)
					angle += 360.0f;
				while (angle > 360.0f)
					angle -= 360.0f;

				if (angle >= 30.0f && angle < 180.0f)
					data.m_nIndicatorState = eIndicatorState::LeftOn;
				else if (angle <= 330.0f && angle > 180.0f)
					data.m_nIndicatorState = eIndicatorState::RightOn;

				if (data.m_nIndicatorState == eIndicatorState::Off)
				{
					if (pControlVeh->m_autoPilot.m_nCurrentLane == 0 && pControlVeh->m_autoPilot.m_nNextLane == 1)
						data.m_nIndicatorState = eIndicatorState::RightOn;
					else if (pControlVeh->m_autoPilot.m_nCurrentLane == 1 && pControlVeh->m_autoPilot.m_nNextLane == 0)
						data.m_nIndicatorState = eIndicatorState::LeftOn;
				}
			}

			if (!indicatorsDelay || indState == eIndicatorState::Off)
			{
				return;
			}

			// global turn lights
			if (gbGlobalIndicatorLights && !IsDummyAvail(pControlVeh, INDICATOR_LIGHTS_TYPE) && !IsDummyAvail(pControlVeh, {eLightType::STTLightLeft, eLightType::STTLightRight}))
			{
				if ((pControlVeh->m_nVehicleSubClass == VEHICLE_AUTOMOBILE || pControlVeh->m_nVehicleSubClass == VEHICLE_BIKE || pControlVeh->m_nVehicleSubClass == VEHICLE_QUAD) &&
					(pControlVeh->GetVehicleAppearance() == VEHICLE_APPEARANCE_AUTOMOBILE || pControlVeh->GetVehicleAppearance() == VEHICLE_APPEARANCE_BIKE) &&
					pControlVeh->m_nVehicleFlags.bEngineOn && pControlVeh->m_fHealth > 0 && !pControlVeh->m_nVehicleFlags.bIsDrowning && !pControlVeh->m_pAttachedTo)
				{
					if (DistanceBetweenPoints(TheCamera.m_vecGameCamPos, pControlVeh->GetPosition()) < 150.0f)
					{
						if (indState == eIndicatorState::BothOn || indState == eIndicatorState::RightOn)
						{
							DrawGlobalLight(pControlVeh, false, false, {255, 128, 0, 0}, "indicator", {1.0f, 1.0f}, shdwOffset);
							DrawGlobalLight(pTowedVeh, true, false, {255, 128, 0, 0}, "indicator", {1.0f, 1.0f}, shdwOffset);
						}
						if (indState == eIndicatorState::BothOn || indState == eIndicatorState::LeftOn)
						{
							DrawGlobalLight(pControlVeh, false, true, {255, 128, 0, 0}, "indicator", {1.0f, 1.0f}, shdwOffset);
							DrawGlobalLight(pTowedVeh, true, true, {255, 128, 0, 0}, "indicator", {1.0f, 1.0f}, shdwOffset);
						}
					}
				}
			}
			else
			{
				if (indState == eIndicatorState::BothOn || indState == eIndicatorState::LeftOn)
				{
					RenderLights(pControlVeh, pTowedVeh, eLightType::IndicatorLightLeftFront, true, "indicator", {1.0f, 1.0f}, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::IndicatorLightLeftMiddle, true, "indicator", {1.0f, 1.0f}, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::IndicatorLightLeftRear, true, "indicator", {1.0f, 1.0f}, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::STTLightLeft, true, shdwName, shdwSz, shdwOffset, true);
				}

				if (indState == eIndicatorState::BothOn || indState == eIndicatorState::RightOn)
				{
					RenderLights(pControlVeh, pTowedVeh, eLightType::IndicatorLightRightFront, true, "indicator", {1.0f, 1.0f}, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::IndicatorLightRightMiddle, true, "indicator", {1.0f, 1.0f}, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::IndicatorLightRightRear, true, "indicator", {1.0f, 1.0f}, shdwOffset);
					RenderLights(pControlVeh, pTowedVeh, eLightType::STTLightRight, true, shdwName, shdwSz, shdwOffset, true);
				}
			}
			if (indState == eIndicatorState::BothOn || indState == eIndicatorState::LeftOn)
			{
				RenderLights(pControlVeh, pTowedVeh, eLightType::NABrakeLightLeft, true, "indicator", {1.0f, 1.0f}, shdwOffset);
			}

			if (indState == eIndicatorState::BothOn || indState == eIndicatorState::RightOn)
			{
				RenderLights(pControlVeh, pTowedVeh, eLightType::NABrakeLightRight, true, "indicator", {1.0f, 1.0f}, shdwOffset);
			} 
		}
	);
};

void Lights::RenderLight(CVehicle *pVeh, eLightType state, bool shadows, std::string texture, CVector2D sz, CVector2D offset, bool highlight)
{
	int id = static_cast<int>(state) * 1000;
	bool litMats = true;
	if (IsDummyAvail(pVeh, state))
	{
		for (auto e : m_Dummies[pVeh][state])
		{
			const VehicleDummyConfig& c = e->GetRef();
			RwFrame *parent = RwFrameGetParent(e->Get().frame);
			bool atomicCheck = e->GetRef().lightType != eLightType::HeadLightLeft 
								&& e->GetRef().lightType != eLightType::HeadLightRight 
								&& !IsOkAtomicVisible(parent);

			if (atomicCheck || (c.dummyType == eDummyPos::Rear && pVeh->m_pTrailer))
			{
				litMats = false;
				break;
			}

			if (state == eLightType::StrobeLight)
			{
				size_t timer = CTimer::m_snTimeInMilliseconds;
				if (timer - c.strobe.timer > c.strobe.delay)
				{
					e->Get().strobe.enabled = !c.strobe.enabled;
					e->Get().strobe.timer = timer;
				}

				if (c.strobe.enabled)
				{
					ModelInfoMgr::EnableStrobeMaterial(pVeh, c.dummyIdx);
				} else {
					continue;
				}
			}

			EnableDummy((int)pVeh + 42 + id++, e, pVeh, highlight ? 1.5f : 1.0f);

			if (shadows && c.shadow.render)
			{
				texture = (c.shadow.texture == "") ? texture : c.shadow.texture;
				e->Update();
				RenderUtil::RegisterShadow(pVeh, c.shadow.position, c.shadow.color, c.rotation.angle, c.dummyType, texture, {sz.x * c.shadow.size.x, sz.y * c.shadow.size.y}, {offset.x + c.shadow.offset.x, offset.y + c.shadow.offset.y});
			}
		}
	}

	if (litMats) {
		ModelInfoMgr::EnableLightMaterial(pVeh, state);
	}
}

void Lights::RenderLights(CVehicle *pControlVeh, CVehicle *pTowedVeh, eLightType state, bool shadows, std::string texture, CVector2D sz, CVector2D offset, bool highlight)
{	
	int model = pControlVeh->m_nModelIndex;
	if (CModelInfo::IsHeliModel(model) || CModelInfo::IsPlaneModel(model)) {
		offset = {0.0f, 0.0f};
		sz = {1.0f, 1.0f};
		texture = "pointlight";
	}

	RenderLight(pControlVeh, state, shadows, texture, sz, offset, highlight);

	if (pControlVeh != pTowedVeh)
	{
		RenderLight(pTowedVeh, state, shadows, texture, sz, offset, highlight);
	}
}

void Lights::RenderHeadlights(CVehicle *pControlVeh, bool realTime) {
	CVehicle *pTowedVeh = pControlVeh;
	VehData &data = m_VehData.Get(pControlVeh);

	if (pControlVeh->m_pTrailer) {
		pTowedVeh = pControlVeh->m_pTrailer;
	}

	if (pControlVeh->m_nVehicleFlags.bLightsOn) {
		bool isFoggy = (CWeather::NewWeatherType == WEATHER_FOGGY_SF || CWeather::NewWeatherType == WEATHER_SANDSTORM_DESERT || CWeather::OldWeatherType == WEATHER_FOGGY_SF || CWeather::OldWeatherType == WEATHER_SANDSTORM_DESERT);
		std::string texName = data.m_bLongLightsOn ? "headlight_long" : "headlight_short";
		
		if (pControlVeh->m_renderLights.m_bLeftFront || pControlVeh->m_renderLights.m_bRightFront) {
			if (realTime) {
				CPointLights::AddLight(PLTYPE_SPOTLIGHT, pControlVeh->m_matrix->pos, pControlVeh->m_matrix->up, 20.0f, 1.0, 1.0, 1.0, 0, 0, 0);
			}
			if (pControlVeh->m_renderLights.m_bLeftFront)
			{
				RenderLights(pControlVeh, pTowedVeh, eLightType::HeadLightLeft, true, texName, headlightSz, headlightOffset, isFoggy || data.m_bLongLightsOn);
			}

			if (pControlVeh->m_renderLights.m_bRightFront)
			{
				RenderLights(pControlVeh, pTowedVeh, eLightType::HeadLightRight, true, texName, headlightSz, headlightOffset, isFoggy || data.m_bLongLightsOn);
			}
		}
	}
}

void Lights::EnableDummy(int id, VehicleDummy *dummy, CVehicle *pVeh, float szMul)
{
	if (gConfig.ReadBoolean("VEHICLE_FEATURES", "LightCoronas", false))
	{
		const VehicleDummyConfig& c = dummy->GetRef();
		if (c.corona.lightingType == eLightingMode::NonDirectional)
		{
			RenderUtil::RegisterCorona(pVeh, (reinterpret_cast<unsigned int>(pVeh) * 255) + 255 + id, c.position, c.corona.color, c.corona.size * szMul);
		}
		else
		{
			RenderUtil::RegisterCoronaWithAngle(pVeh, (reinterpret_cast<unsigned int>(pVeh) * 255) + 255 + id, c.position, c.corona.color, c.rotation.angle + (c.corona.lightingType == eLightingMode::Inversed ? 180.0f : 0.0f), 180.0f, c.corona.size * szMul);
		}
	}
}

void Lights::Reload(CVehicle *pVeh)
{
	m_Dummies.erase(pVeh);
	DataMgr::Reload(pVeh->m_nModelIndex);
}

bool Lights::IsDummyAvail(CVehicle *pVeh, eLightType state)
{
	return m_Dummies[pVeh][state].size() != 0;
}

bool Lights::IsDummyAvail(CVehicle* pVeh, std::initializer_list<eLightType> states)
{
	for (eLightType state : states) {
		if (IsDummyAvail(pVeh, state)) {
			return true;
		}
	}
	return false;
}