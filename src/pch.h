#pragma once
#define FMT_UNICODE 0
#include <CTimer.h>
#include <NodeName.h>
#include <CModelInfo.h>

#include <RenderWare.h>
#include <plugin.h>
#include <game_sa/common.h>

#include "nlohmann/json.hpp"
#include "db/ini.hpp"
#include "AixLog/AixLog.hpp"
#include <format>
#include "utils/util.h"
#include "vkeys.h"

using namespace plugin;

enum class eModelEntityType
{
  Ped,
  Object,
  Vehicle,
  Weapon,
  Jetpack,
};

extern CIniReader gConfig;

#define LOG_NO_LEVEL(x) LOG(INFO) << x;

extern bool gVerboseLogging;

#define LOG_VERBOSE(fmt, ...)             \
  do                                      \
  {                                       \
    if (gVerboseLogging)                  \
    {                                     \
      LOG(DEBUG) << std::format(fmt, ##__VA_ARGS__); \
    }                                     \
  } while (0)

extern unsigned int FramePluginOffset;
#define PLUGIN_ID_STR 'MEX'
#define PLUGIN_ID_NUM 0x42945628
#define FRAME_EXTENSION(frame) ((RwFrameExtension *)((unsigned int)frame + FramePluginOffset))

struct RwFrameExtension
{
  CVehicle *pOwner;
  RwMatrix *pOrigMatrix;

  static RwFrame *Initialize(RwFrame *pFrame)
  {
    FRAME_EXTENSION(pFrame)->pOwner = nullptr;
    FRAME_EXTENSION(pFrame)->pOrigMatrix = nullptr;
    return pFrame;
  }

  static RwFrame *Shutdown(RwFrame *pFrame)
  {
    if (FRAME_EXTENSION(pFrame)->pOrigMatrix)
    {
      delete FRAME_EXTENSION(pFrame)->pOrigMatrix;
    }
    return pFrame;
  }

  static RwFrame *Clone(RwFrame *pCopy, RwFrame *pFrame)
  {
    return pCopy;
  }
};

extern bool gbVehIKInstalled;

static inline CBaseModelInfo **CModelInfo__ms_modelInfoPtrs = reinterpret_cast<CBaseModelInfo **>(patch::GetPointer(0x403DA7));