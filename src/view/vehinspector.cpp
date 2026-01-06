#include "pch.h"
#include "imgui/imgui.h"
#include "CVehicle.h"
#include "CWorld.h"

// --- MACROS FOR SANITY ---
// Handles 1-bit flags (unsigned char bFlag : 1)
#define BF_BOOL(name, var) { bool b = var; if (ImGui::Checkbox(name, &b)) var = b; }

// Handles Multi-bit flags (unsigned char bFlag : 2 or : 3)
#define BF_MBIT(name, var, max_val) { int i = var; if (ImGui::SliderInt(name, &i, 0, max_val)) var = i; }

// Handles byte fields displayed as integers
#define VAL_BYTE(name, var) { int i = var; if (ImGui::InputInt(name, &i)) var = (unsigned char)i; }

// Handles short fields
#define VAL_SHORT(name, var) { int i = var; if (ImGui::InputInt(name, &i)) var = (short)i; }

void DrawMonsterInspector()
{
    CVehicle* pVeh = FindPlayerVehicle(0, false);
    
    if (!pVeh) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "NO ACTIVE VEHICLE FOUND.");
        return;
    }

    ImGui::Text("Vehicle Pointer: 0x%p | Model: %d", pVeh, pVeh->m_nModelIndex);
    ImGui::Separator();

    if (ImGui::BeginTabBar("MonsterTabs"))
    {
        // =========================================================
        // TAB 1: INHERITANCE (Placeable, Entity, Physical)
        // =========================================================
        if (ImGui::BeginTabItem("1. Hierarchy"))
        {
            if (ImGui::CollapsingHeader("CPlaceable [0x00]"))
            {
                if (pVeh->m_matrix) {
                    ImGui::DragFloat3("Matrix Position", reinterpret_cast<float *>(&pVeh->m_matrix->pos));
                    ImGui::Text("Matrix Ptr: 0x%p", pVeh->m_matrix);
                } else {
                    ImGui::DragFloat3("Placement Position", reinterpret_cast<float *>(&pVeh->m_placement.m_vPosn));
                }
                ImGui::DragFloat("Heading", &pVeh->m_placement.m_fHeading);
            }

            if (ImGui::CollapsingHeader("CEntity [0x18]"))
            {
                ImGui::Text("RW Object: 0x%p", pVeh->m_pRwObject);
                // FIX: Use temporary variables for bit-fields
                int tempType = pVeh->m_nType;
                if (ImGui::InputInt("Type (eEntityType)", &tempType)) {
                    // Optional: Clamp value to fit 3 bits (0-7)
                    if (tempType < 0) tempType = 0;
                    if (tempType > 7) tempType = 7;
                    pVeh->m_nType = (unsigned char)tempType;
                }

                int tempStatus = pVeh->m_nStatus;
                if (ImGui::InputInt("Status (eEntityStatus)", &tempStatus)) {
                    // Optional: Clamp value to fit 5 bits (0-31)
                    if (tempStatus < 0) tempStatus = 0;
                    if (tempStatus > 31) tempStatus = 31;
                    pVeh->m_nStatus = (unsigned char)tempStatus;
                }
                
                if (ImGui::TreeNode("Entity Flags")) {
                    BF_BOOL("bUsesCollision", pVeh->bUsesCollision);
                    BF_BOOL("bCollisionProcessed", pVeh->bCollisionProcessed);
                    BF_BOOL("bIsStatic", pVeh->bIsStatic);
                    BF_BOOL("bHasContacted", pVeh->bHasContacted);
                    BF_BOOL("bIsStuck", pVeh->bIsStuck);
                    BF_BOOL("bIsInSafePosition", pVeh->bIsInSafePosition);
                    BF_BOOL("bWasPostponed", pVeh->bWasPostponed);
                    BF_BOOL("bIsVisible", pVeh->bIsVisible);
                    BF_BOOL("bIsBIGBuilding", pVeh->bIsBIGBuilding);
                    BF_BOOL("bRenderDamaged", pVeh->bRenderDamaged);
                    BF_BOOL("bStreamingDontDelete", pVeh->bStreamingDontDelete);
                    BF_BOOL("bRemoveFromWorld", pVeh->bRemoveFromWorld);
                    BF_BOOL("bHasHitWall", pVeh->bHasHitWall);
                    BF_BOOL("bImBeingRendered", pVeh->bImBeingRendered);
                    BF_BOOL("bDrawLast", pVeh->bDrawLast);
                    BF_BOOL("bDistanceFade", pVeh->bDistanceFade);
                    BF_BOOL("bDontCastShadowsOn", pVeh->bDontCastShadowsOn);
                    BF_BOOL("bOffscreen", pVeh->bOffscreen);
                    BF_BOOL("bIsStaticWaitingForCollision", pVeh->bIsStaticWaitingForCollision);
                    BF_BOOL("bDontStream", pVeh->bDontStream);
                    BF_BOOL("bUnderwater", pVeh->bUnderwater);
                    BF_BOOL("bHasPreRenderEffects", pVeh->bHasPreRenderEffects);
                    BF_BOOL("bIsTempBuilding", pVeh->bIsTempBuilding);
                    BF_BOOL("bDontUpdateHierarchy", pVeh->bDontUpdateHierarchy);
                    BF_BOOL("bHasRoadsignText", pVeh->bHasRoadsignText);
                    BF_BOOL("bDisplayedSuperLowLOD", pVeh->bDisplayedSuperLowLOD);
                    BF_BOOL("bIsProcObject", pVeh->bIsProcObject);
                    BF_BOOL("bBackfaceCulled", pVeh->bBackfaceCulled);
                    BF_BOOL("bLightObject", pVeh->bLightObject);
                    BF_BOOL("bUnimportantStream", pVeh->bUnimportantStream);
                    BF_BOOL("bTunnel", pVeh->bTunnel);
                    BF_BOOL("bTunnelTransition", pVeh->bTunnelTransition);
                    ImGui::TreePop();
                }
                
                ImGui::InputScalar("Model Index", ImGuiDataType_U16, &pVeh->m_nModelIndex);
                ImGui::InputScalar("IPL Index", ImGuiDataType_S8, &pVeh->m_nIplIndex);
                ImGui::InputScalar("Area Code", ImGuiDataType_U8, &pVeh->m_nAreaCode);
                ImGui::InputScalar("Lod Index", ImGuiDataType_S32, &pVeh->m_nLodIndex);
                ImGui::InputScalar("Num Lod Children", ImGuiDataType_U8, &pVeh->m_nNumLodChildren);
                ImGui::InputScalar("Num Lod Rendered", ImGuiDataType_U8, &pVeh->m_nNumLodChildrenRendered);
            }

            if (ImGui::CollapsingHeader("CPhysical [0x38]"))
            {
                ImGui::DragFloat3("Move Speed", (float*)&pVeh->m_vecMoveSpeed, 0.01f);
                ImGui::DragFloat3("Turn Speed", (float*)&pVeh->m_vecTurnSpeed, 0.01f);
                ImGui::DragFloat3("Friction Move", (float*)&pVeh->m_vecFrictionMoveSpeed, 0.01f);
                ImGui::DragFloat3("Friction Turn", (float*)&pVeh->m_vecFrictionTurnSpeed, 0.01f);
                ImGui::DragFloat3("Force", (float*)&pVeh->m_vecForce, 1.0f);
                ImGui::DragFloat3("Torque", (float*)&pVeh->m_vecTorque, 1.0f);
                ImGui::SliderFloat("Mass", &pVeh->m_fMass, 0.1f, 50000.0f);
                ImGui::SliderFloat("Turn Mass", &pVeh->m_fTurnMass, 0.1f, 50000.0f);
                ImGui::SliderFloat("Air Resistance", &pVeh->m_fAirResistance, 0.0f, 10.0f);
                ImGui::SliderFloat("Elasticity", &pVeh->m_fElasticity, 0.0f, 1.0f);
                ImGui::DragFloat3("Center of Mass", (float*)&pVeh->m_vecCentreOfMass, 0.01f);

                if (ImGui::TreeNode("Physical Bitfields")) {
                    BF_BOOL("b01", pVeh->b01);
                    BF_BOOL("bApplyGravity", pVeh->bApplyGravity);
                    BF_BOOL("bDisableCollisionForce", pVeh->bDisableCollisionForce);
                    BF_BOOL("bCollidable", pVeh->bCollidable);
                    BF_BOOL("bDisableTurnForce", pVeh->bDisableTurnForce);
                    BF_BOOL("bDisableMoveForce", pVeh->bDisableMoveForce);
                    BF_BOOL("bInfiniteMass", pVeh->bInfiniteMass);
                    BF_BOOL("bDisableZ", pVeh->bDisableZ);
                    BF_BOOL("bSubmergedInWater", pVeh->bSubmergedInWater);
                    BF_BOOL("bOnSolidSurface", pVeh->bOnSolidSurface);
                    BF_BOOL("bBroken", pVeh->bBroken);
                    BF_BOOL("bBulletProof", pVeh->bBulletProof);
                    BF_BOOL("bFireProof", pVeh->bFireProof);
                    BF_BOOL("bCollisionProof", pVeh->bCollisionProof);
                    BF_BOOL("bMeleeProof", pVeh->bMeleeProof);
                    BF_BOOL("bInvulnerable", pVeh->bInvulnerable);
                    BF_BOOL("bExplosionProof", pVeh->bExplosionProof);
                    BF_BOOL("bAttachedToEntity", pVeh->bAttachedToEntity);
                    BF_BOOL("bTouchingWater", pVeh->bTouchingWater);
                    BF_BOOL("bCanBeCollidedWith", pVeh->bCanBeCollidedWith);
                    BF_BOOL("bDestroyed", pVeh->bDestroyed);
                    ImGui::TreePop();
                }
                
                ImGui::Text("Attached Entity: 0x%p", pVeh->m_pAttachedTo);
                ImGui::DragFloat3("Attach Offset", (float*)&pVeh->m_vecAttachOffset);
                ImGui::Text("Damage Entity: 0x%p", pVeh->m_pDamageEntity);
                ImGui::SliderFloat("Damage Intensity", &pVeh->m_fDamageIntensity, 0.0f, 1000.0f);
            }
            ImGui::EndTabItem();
        }

        // =========================================================
        // TAB 2: CVehicle FLAGS (The Bitfield Wall)
        // =========================================================
        if (ImGui::BeginTabItem("2. Vehicle Flags"))
        {
            ImGui::BeginChild("FlagsScroll");
            // Grouping them loosely by functionality
            ImGui::TextDisabled("Status & Type");
            BF_BOOL("bIsLawEnforcer", pVeh->bIsLawEnforcer);
            BF_BOOL("bIsAmbulanceOnDuty", pVeh->bIsAmbulanceOnDuty);
            BF_BOOL("bIsFireTruckOnDuty", pVeh->bIsFireTruckOnDuty);
            BF_BOOL("bIsVan", pVeh->bIsVan);
            BF_BOOL("bIsBus", pVeh->bIsBus);
            BF_BOOL("bIsBig", pVeh->bIsBig);
            BF_BOOL("bLowVehicle", pVeh->bLowVehicle);
            BF_BOOL("bIsRCVehicle", pVeh->bIsRCVehicle);
            
            ImGui::Separator();
            ImGui::TextDisabled("State & Logic");
            BF_BOOL("bIsLocked", pVeh->bIsLocked);
            BF_BOOL("bEngineOn", pVeh->bEngineOn);
            BF_BOOL("bIsHandbrakeOn", pVeh->bIsHandbrakeOn);
            BF_BOOL("bLightsOn", pVeh->bLightsOn);
            BF_BOOL("bFreebies", pVeh->bFreebies);
            BF_BOOL("bComedyControls", pVeh->bComedyControls);
            BF_BOOL("bWarnedPeds", pVeh->bWarnedPeds);
            BF_BOOL("bCraneMessageDone", pVeh->bCraneMessageDone);
            BF_BOOL("bTakeLessDamage", pVeh->bTakeLessDamage);
            BF_BOOL("bIsDamaged", pVeh->bIsDamaged);
            BF_BOOL("bHasBeenOwnedByPlayer", pVeh->bHasBeenOwnedByPlayer);
            BF_BOOL("bFadeOut", pVeh->bFadeOut);
            BF_BOOL("bIsBeingCarJacked", pVeh->bIsBeingCarJacked);
            BF_BOOL("bCreateRoadBlockPeds", pVeh->bCreateRoadBlockPeds);
            BF_BOOL("bCanBeDamaged", pVeh->bCanBeDamaged);
            BF_BOOL("bOccupantsHaveBeenGenerated", pVeh->bOccupantsHaveBeenGenerated);
            BF_BOOL("bGunSwitchedOff", pVeh->bGunSwitchedOff);
            BF_BOOL("bVehicleColProcessed", pVeh->bVehicleColProcessed);
            BF_BOOL("bIsCarParkVehicle", pVeh->bIsCarParkVehicle);
            BF_BOOL("bHasAlreadyBeenRecorded", pVeh->bHasAlreadyBeenRecorded);
            BF_BOOL("bPartOfConvoy", pVeh->bPartOfConvoy);
            BF_BOOL("bHeliMinimumTilt", pVeh->bHeliMinimumTilt);
            BF_BOOL("bAudioChangingGear", pVeh->bAudioChangingGear);
            BF_BOOL("bIsDrowning", pVeh->bIsDrowning);
            BF_BOOL("bTyresDontBurst", pVeh->bTyresDontBurst);
            BF_BOOL("bCreatedAsPoliceVehicle", pVeh->bCreatedAsPoliceVehicle);
            BF_BOOL("bRestingOnPhysical", pVeh->bRestingOnPhysical);
            BF_BOOL("bParking", pVeh->bParking);
            BF_BOOL("bCanPark", pVeh->bCanPark);
            BF_BOOL("bFireGun", pVeh->bFireGun);
            BF_BOOL("bDriverLastFrame", pVeh->bDriverLastFrame);
            BF_BOOL("bNeverUseSmallerRemovalRange", pVeh->bNeverUseSmallerRemovalRange);
            BF_BOOL("bAlwaysSkidMarks", pVeh->bAlwaysSkidMarks);
            BF_BOOL("bEngineBroken", pVeh->bEngineBroken);
            BF_BOOL("bVehicleCanBeTargetted", pVeh->bVehicleCanBeTargetted);
            BF_BOOL("bPartOfAttackWave", pVeh->bPartOfAttackWave);
            BF_BOOL("bWinchCanPickMeUp", pVeh->bWinchCanPickMeUp);
            BF_BOOL("bImpounded", pVeh->bImpounded);
            BF_BOOL("bVehicleCanBeTargettedByHS", pVeh->bVehicleCanBeTargettedByHS);
            BF_BOOL("bSirenOrAlarm", pVeh->bSirenOrAlarm);
            BF_BOOL("bHasGangLeaningOn", pVeh->bHasGangLeaningOn);
            BF_BOOL("bGangMembersForRoadBlock", pVeh->bGangMembersForRoadBlock);
            BF_BOOL("bDoesProvideCover", pVeh->bDoesProvideCover);
            BF_BOOL("bMadDriver", pVeh->bMadDriver);
            BF_BOOL("bUpgradedStereo", pVeh->bUpgradedStereo);
            BF_BOOL("bConsideredByPlayer", pVeh->bConsideredByPlayer);
            BF_BOOL("bPetrolTankIsWeakPoint", pVeh->bPetrolTankIsWeakPoint);
            BF_BOOL("bDisableParticles", pVeh->bDisableParticles);
            BF_BOOL("bHasBeenResprayed", pVeh->bHasBeenResprayed);
            BF_BOOL("bUseCarCheats", pVeh->bUseCarCheats);
            BF_BOOL("bDontSetColourWhenRemapping", pVeh->bDontSetColourWhenRemapping);
            BF_BOOL("bUsedForReplay", pVeh->bUsedForReplay);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // =========================================================
        // TAB 3: CVehicle DATA (Main Variables)
        // =========================================================
        if (ImGui::BeginTabItem("3. Vehicle Data"))
        {
            ImGui::Text("Audio Entity: 0x%p", &pVeh->m_vehicleAudio);
            
            if (ImGui::TreeNode("Handling Data")) {
                ImGui::Text("Handling Pointer: 0x%p", pVeh->m_pHandlingData);
                ImGui::Text("Flying Handling: 0x%p", pVeh->m_pFlyingHandlingData);
                
                // Union Handling Flags
                if (ImGui::TreeNode("Handling Flags (Union)")) {
                    BF_BOOL("b1gBoost", pVeh->m_nHandlingFlags.b1gBoost);
                    BF_BOOL("b2gBoost", pVeh->m_nHandlingFlags.b2gBoost);
                    BF_BOOL("bNpcAntiRoll", pVeh->m_nHandlingFlags.bNpcAntiRoll);
                    BF_BOOL("bNpcNeutralHandl", pVeh->m_nHandlingFlags.bNpcNeutralHandl);
                    BF_BOOL("bNoHandbrake", pVeh->m_nHandlingFlags.bNoHandbrake);
                    BF_BOOL("bSteerRearwheels", pVeh->m_nHandlingFlags.bSteerRearwheels);
                    BF_BOOL("bHbRearwheelSteer", pVeh->m_nHandlingFlags.bHbRearwheelSteer);
                    BF_BOOL("bAltSteerOpt", pVeh->m_nHandlingFlags.bAltSteerOpt);
                    BF_BOOL("bWheelFNarrow2", pVeh->m_nHandlingFlags.bWheelFNarrow2);
                    BF_BOOL("bWheelFNarrow", pVeh->m_nHandlingFlags.bWheelFNarrow);
                    BF_BOOL("bWheelFWide", pVeh->m_nHandlingFlags.bWheelFWide);
                    BF_BOOL("bWheelFWide2", pVeh->m_nHandlingFlags.bWheelFWide2);
                    BF_BOOL("bHydraulicGeom", pVeh->m_nHandlingFlags.bHydraulicGeom);
                    BF_BOOL("bHydraulicInst", pVeh->m_nHandlingFlags.bHydraulicInst);
                    BF_BOOL("bNosInst", pVeh->m_nHandlingFlags.bNosInst);
                    BF_BOOL("bOffroadAbility", pVeh->m_nHandlingFlags.bOffroadAbility);
                    BF_BOOL("bHalogenLights", pVeh->m_nHandlingFlags.bHalogenLights);
                    BF_BOOL("bLowRider", pVeh->m_nHandlingFlags.bLowRider);
                    BF_BOOL("bStreetRacer", pVeh->m_nHandlingFlags.bStreetRacer);
                    BF_BOOL("bSwingingChassis", pVeh->m_nHandlingFlags.bSwingingChassis);
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("AutoPilot")) {
                ImGui::DragFloat3("Destination", (float*)&pVeh->m_autoPilot.m_vecDestinationCoors);
                ImGui::InputInt("Mode", (int*)&pVeh->m_autoPilot.m_nCarMission);
                ImGui::DragFloat("Cruise Speed", reinterpret_cast<float *>(&pVeh->m_autoPilot.m_nCruiseSpeed));
                ImGui::TreePop();
            }
            
            ImGui::Separator();
            ImGui::Text("Creation Time: %u", pVeh->m_nCreationTime);
            
            int colors[4] = { pVeh->m_nPrimaryColor, pVeh->m_nSecondaryColor, pVeh->m_nTertiaryColor, pVeh->m_nQuaternaryColor };
            if (ImGui::InputInt4("Colors (1-4)", colors)) {
                pVeh->m_nPrimaryColor = colors[0]; pVeh->m_nSecondaryColor = colors[1];
                pVeh->m_nTertiaryColor = colors[2]; pVeh->m_nQuaternaryColor = colors[3];
            }
            
            // Arrays
            ImGui::Text("Extras: [%d, %d]", pVeh->m_anExtras[0], pVeh->m_anExtras[1]);
            if (ImGui::TreeNode("Upgrades Array (m_anUpgrades)")) {
                for (int i=0; i<15; i++) {
                     int val = pVeh->m_anUpgrades[i];
                     if (ImGui::InputInt(std::format("Slot {}", i).c_str(), &val)) pVeh->m_anUpgrades[i] = (short)val;
                }
                ImGui::TreePop();
            }
            
            ImGui::DragFloat("Wheel Scale", &pVeh->m_fWheelScale, 0.01f);
            VAL_SHORT("Alarm State", pVeh->m_nAlarmState);
            VAL_SHORT("Random Route Seed", pVeh->m_nForcedRandomRouteSeed);
            
            ImGui::Separator();
            ImGui::Text("Driver Ptr: 0x%p", pVeh->m_pDriver);
            if (ImGui::TreeNode("Passengers")) {
                for (int i=0; i<8; i++) {
                    ImGui::Text("Passenger %d: 0x%p", i, pVeh->m_apPassengers[i]);
                }
                VAL_BYTE("Num Passengers", pVeh->m_nNumPassengers);
                VAL_BYTE("Max Passengers", pVeh->m_nMaxPassengers);
                ImGui::TreePop();
            }

            VAL_BYTE("Nitro Boosts", pVeh->m_nNitroBoosts);
            ImGui::Text("Entity We Are On: 0x%p", pVeh->m_pEntityWeAreOn);
            ImGui::Text("Fire Ptr: 0x%p", pVeh->m_pFire);
            
            ImGui::SliderFloat("Steer Angle", &pVeh->m_fSteerAngle, -1.0f, 1.0f);
            ImGui::SliderFloat("Gas Pedal", &pVeh->m_fGasPedal, 0.0f, 1.0f);
            ImGui::SliderFloat("Break Pedal", &pVeh->m_fBreakPedal, 0.0f, 1.0f);
            
            VAL_BYTE("Created By", pVeh->m_nCreatedBy);
            VAL_SHORT("Extended Removal Range", pVeh->m_nExtendedRemovalRange);
            
            ImGui::EndTabItem();
        }

        // =========================================================
        // TAB 4: DEEP INTERNALS (Packed bits, Timers, Weapons)
        // =========================================================
        if (ImGui::BeginTabItem("4. Deep Internals"))
        {
            ImGui::TextDisabled("Packed Variables (Bitfields)");
            BF_MBIT("Bomb On Board (0-5)", pVeh->m_nBombOnBoard, 5);
            BF_MBIT("Override Lights (0-2)", pVeh->m_nOverrideLights, 2);
            BF_MBIT("Winch Type (0-2)", pVeh->m_nWinchType, 2);
            BF_MBIT("Guns Cycle (0-2)", pVeh->m_nGunsCycleIndex, 2);
            BF_MBIT("Ordnance Cycle (0-2)", pVeh->m_nOrdnanceCycleIndex, 2);
            
            ImGui::Separator();
            VAL_BYTE("Used For Cover", pVeh->m_nUsedForCover);
            VAL_BYTE("Ammo In Clip", pVeh->m_nAmmoInClip);
            VAL_BYTE("Pac Mans Collected", pVeh->m_nPacMansCollected);
            VAL_BYTE("Peds Pos RoadBlock", pVeh->m_nPedsPositionForRoadBlock);
            VAL_BYTE("Num Cops RoadBlock", pVeh->m_nNumCopsForRoadBlock);
            
            ImGui::SliderFloat("Dirt Level", &pVeh->m_fDirtLevel, 0.0f, 15.0f);
            VAL_BYTE("Current Gear", pVeh->m_nCurrentGear);
            ImGui::DragFloat("Gear Change Count", &pVeh->m_fGearChangeCount);
            ImGui::DragFloat("Wheel Spin Audio", &pVeh->m_fWheelSpinForAudio);
            ImGui::DragFloat("Health", &pVeh->m_fHealth);
            
            ImGui::Text("Tractor Ptr: 0x%p", pVeh->m_pTractor);
            ImGui::Text("Trailer Ptr: 0x%p", pVeh->m_pTrailer);
            ImGui::Text("Bomb Installer: 0x%p", pVeh->m_pWhoInstalledBombOnMe);
            
            ImGui::Separator();
            ImGui::TextDisabled("Timers");
            ImGui::InputInt("Time Till Need This Car", (int*)&pVeh->m_nTimeTillWeNeedThisCar);
            ImGui::InputInt("Gun Firing Time", (int*)&pVeh->m_nGunFiringTime);
            ImGui::InputInt("Time When Blowed Up", (int*)&pVeh->m_nTimeWhenBlowedUp);
            VAL_SHORT("Cops In Car Timer", pVeh->m_nCopsInCarTimer);
            VAL_SHORT("Bomb Timer", pVeh->m_wBombTimer);
            
            ImGui::Text("Who Detonated Me: 0x%p", pVeh->m_pWhoDetonatedMe);
            ImGui::DragFloat("Vehicle Front Z", &pVeh->m_fVehicleFrontGroundZ);
            ImGui::DragFloat("Vehicle Rear Z", &pVeh->m_fVehicleRearGroundZ);
            
            ImGui::Separator();
            ImGui::TextDisabled("Weapons & Weirds");
            // The "Field" vars - displaying as hex
            ImGui::Text("field_4EC: 0x%X", pVeh->field_4EC);
            ImGui::Text("field_4ED (Array): [0x%X, 0x%X ...]", pVeh->field_4ED[0], pVeh->field_4ED[1]);
            
            ImGui::InputInt("Door Lock State", (int*)&pVeh->m_eDoorLock);
            ImGui::InputInt("Projectile Fire Time", (int*)&pVeh->m_nProjectileWeaponFiringTime);
            ImGui::InputInt("Add. Proj Fire Time", (int*)&pVeh->m_nAdditionalProjectileWeaponFiringTime);
            ImGui::InputInt("Minigun Fire Time", (int*)&pVeh->m_nTimeForMinigunFiring);
            VAL_BYTE("Last Weapon Damage Type", pVeh->m_nLastWeaponDamageType);
            ImGui::Text("Last Damage Entity: 0x%p", pVeh->m_pLastDamageEntity);
            
            ImGui::Text("field_510: 0x%X", pVeh->field_510);
            VAL_BYTE("Vehicle Weapon In Use", pVeh->m_nVehicleWeaponInUse);
            ImGui::InputInt("Horn Counter", (int*)&pVeh->m_nHornCounter);
            ImGui::Text("field_518: 0x%X", pVeh->field_518);
            ImGui::Text("field_519: 0x%X", pVeh->field_519);
            ImGui::Text("field_51A: 0x%X", pVeh->field_51A);
            VAL_BYTE("Hassle Pos ID", pVeh->m_nHasslePosId);
            
            if (ImGui::TreeNode("Render Lights (Union)")) {
                BF_BOOL("Right Front", pVeh->m_renderLights.m_bRightFront);
                BF_BOOL("Left Front", pVeh->m_renderLights.m_bLeftFront);
                BF_BOOL("Right Rear", pVeh->m_renderLights.m_bRightRear);
                BF_BOOL("Left Rear", pVeh->m_renderLights.m_bLeftRear);
                ImGui::TreePop();
            }
            
            ImGui::Text("Custom Plate Tex: 0x%p", pVeh->m_pCustomCarPlate);
            ImGui::InputInt("Vehicle Class", (int*)&pVeh->m_nVehicleClass);
            ImGui::InputInt("Vehicle SubClass", (int*)&pVeh->m_nVehicleSubClass);
            VAL_SHORT("Prev Remap TXD", pVeh->m_nPreviousRemapTxd);
            VAL_SHORT("Remap TXD", pVeh->m_nRemapTxd);
            ImGui::Text("Remap Texture: 0x%p", pVeh->m_pRemapTexture);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}