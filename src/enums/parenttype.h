#pragma once
#include <string>

enum class eParentType {
    Unknown,

    PANEL_START,
    WingLeftFront,
    WingRightFront,
    WingLeftRear,
    WingRightRear,
    WindScreen,
    BumperFront,
    BumperRear,
    PANEL_END,

    DOOR_START,
    Boonet,
    Boot,
    DoorLeftFront,
    DoorRightFront,
    DoorLeftRear,
    DoorRightRear,
    DOOR_END,

    LIGHT_START,
    LightLeftFront,
    LightRightFront,
    LightRightRear,
    LightLeftRear,
    LIGHT_END,

    WHEEL_START,
    WheelLeftFront,
    WheelLeftRear,
    WheelRightFront,
    WheelRightRear,
    WHEEL_END,
};

enum eLightType;

eParentType eParentTypeFromString(const std::string& str);
bool IsParentTypeDamaged(CVehicle* pVeh, eParentType type, eLightType lightType);