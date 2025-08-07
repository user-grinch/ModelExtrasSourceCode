/*
 * API provided for ModelExtras v2.0 Release
 *
 *
 * Usage:
 * - Initialize a connection with TAPI_InitConnect(). This step is required to establish a link between your mod and the trainer.
 * - Add various widgets such as buttons, sliders, and text using the provided functions (e.g., TAPI_Button, TAPI_SliderInt).
 * - If you need to clear the interface or reload widgets dynamically, use TAPI_ClearWidgets(). This is helpful when your mod needs to refresh or reinitialize the UI.
 * - Close the connection with TAPI_CloseConnect() after you are done. This ensures that resources are properly released and the connection is terminated.
 */

#pragma once
#define ME_API_VERSION 10000

typedef void (*T_FUNC)(void *value);

#ifdef MODELEXTRAS_DEV
#define ME_WRAPPER __declspec(dllexport)
#else
#define ME_WRAPPER __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // Functions to get API and ModelExtras version
    ME_WRAPPER int ME_GetAPIVersion();
    ME_WRAPPER int ME_GetVersion();
    
#ifdef __cplusplus
}
#endif