#include "pch.h"
#include "defines.h"
#include "ModelExtrasAPI.h"

extern "C"
{
    int ME_GetAPIVersion()
    {
        return ME_API_VERSION;
    }

    int ME_GetVersion()
    {
        return MOD_VERSION_NUMBER;
    }
}