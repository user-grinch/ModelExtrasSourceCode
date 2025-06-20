#include "pch.h"
#include "parenttype.h"
#include "lighttype.h"

eParentType eParentTypeFromString(const std::string& str) {
    if (str == "wing-left-front") return eParentType::WingLeftFront;
    else if (str == "wing-right-front") return eParentType::WingRightFront;
    else if (str == "wing-left-rear") return eParentType::WingLeftRear;
    else if (str == "wing-right-rear") return eParentType::WingRightRear;
    else if (str == "wind-screen") return eParentType::WindScreen;
    else if (str == "bumper-front") return eParentType::BumperFront;
    else if (str == "bumper-rear") return eParentType::BumperRear;
    else if (str == "boonet") return eParentType::Boonet;
    else if (str == "boot") return eParentType::Boot;
    else if (str == "door-left-front") return eParentType::DoorLeftFront;
    else if (str == "door-right-front") return eParentType::DoorRightFront;
    else if (str == "door-left-rear") return eParentType::DoorLeftRear;
    else if (str == "door-right-rear") return eParentType::DoorRightRear;
    else if (str == "light-left-front") return eParentType::LightLeftFront;
    else if (str == "light-right-front") return eParentType::LightRightFront;
    else if (str == "light-right-rear") return eParentType::LightRightRear;
    else if (str == "light-left-rear") return eParentType::LightLeftRear;
    else if (str == "wheel-left-front") return eParentType::WheelLeftFront;
    else if (str == "wheel-right-front") return eParentType::WheelRightFront;
    else if (str == "wheel-right-rear") return eParentType::WheelRightRear;
    else if (str == "wheel-left-rear") return eParentType::WheelLeftRear;
    return eParentType::Unknown;
}

// Ugly piece of code but someone's gonna do it right?
// Probably should've gone with a OOP model lol
bool IsParentTypeDamaged(CVehicle* pVeh, eParentType type, eLightType lightType) {
    if (type == eParentType::Unknown) {
        if (lightType == eLightType::HeadLightLeft || lightType == eLightType::FogLightLeft 
            || lightType == eLightType::IndicatorLightLeftFront) {
            type = eParentType::LightLeftFront;
        }
        else if (lightType == eLightType::HeadLightRight || lightType == eLightType::FogLightRight 
            || lightType == eLightType::IndicatorLightRightFront) {
            type = eParentType::LightRightFront;
        }
        else if (lightType == eLightType::BrakeLightLeft || lightType == eLightType::ReverseLightLeft 
            || lightType == eLightType::STTLightLeft || lightType == eLightType::NABrakeLightLeft 
            || lightType == eLightType::IndicatorLightLeftRear || lightType == eLightType::TailLightLeft) {
            type = eParentType::Boot;
        }
        else if (lightType == eLightType::BrakeLightRight || lightType == eLightType::ReverseLightRight
            || lightType == eLightType::STTLightRight || lightType == eLightType::NABrakeLightRight
            || lightType == eLightType::IndicatorLightRightRear || lightType == eLightType::TailLightRight) {
            type = eParentType::Boot;
        } else {
            return false; 
        }
    }

    if (pVeh->m_nVehicleSubClass != VEHICLE_AUTOMOBILE) {
        return false; // no damage states for them
    }

    CAutomobile* pAutomobile = reinterpret_cast<CAutomobile*>(pVeh);

    if (type >= eParentType::PANEL_START && type <= eParentType::PANEL_END) {
        uint32_t panelIndex = static_cast<uint32_t>(type) - static_cast<uint32_t>(eParentType::PANEL_START) - 1;
        return pAutomobile->m_damageManager.GetPanelStatus(panelIndex);
    }

    if (type >= eParentType::DOOR_START && type <= eParentType::DOOR_END) {
        uint32_t doorIndex = static_cast<uint32_t>(type) - static_cast<uint32_t>(eParentType::DOOR_START) - 1;
        return pAutomobile->m_damageManager.GetDoorStatus(static_cast<eDoors>(doorIndex));
    }

    if (type >= eParentType::LIGHT_START && type <= eParentType::LIGHT_END) {
        uint32_t lightIndex = static_cast<uint32_t>(type) - static_cast<uint32_t>(eParentType::LIGHT_START) - 1;
        return pAutomobile->m_damageManager.GetLightStatus(static_cast<eLights>(lightIndex));
    }

    if (type >= eParentType::WHEEL_START && type <= eParentType::WHEEL_END) {
        uint32_t wheelIndex = static_cast<uint32_t>(type) - static_cast<uint32_t>(eParentType::WHEEL_START) - 1;
        return pAutomobile->m_damageManager.GetWheelStatus(static_cast<eWheels>(wheelIndex));
    }

    return false;
}