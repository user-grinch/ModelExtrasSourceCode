#include "pch.h"
#include "defines.h"
#include "features/mgr.h"

#include <extensions/ScriptCommands.h>
#include <extensions/scripting/ScriptCommandNames.h>

#define GET_SCRIPT_STRUCT_NAMED 0xAAA

extern void ShowDonationWindow();

std::vector<std::string> donators = {
    "Wei Woo",
    "©Wishy",
    "Alexander Alexander",
    "Imad Fikri",
    "ID_not4ound",
    "Macc",
    "lemaze93",
    "XG417",
    "Ruethy",
    "Flaqko _GTA",
    "MG45",
    "Boris Ilincic",
    "Damix",
    "spdfnpe",
    "Pol3 Million",
    "Bubby Jackson",
    "Keith Ferrell",
    "Clayton Morrison",
    "SimBoRRis",
    "Agha"
};

extern void InjectImGuiHooks();

void InitLogFile()
{
    static bool flag = true;
    if (!flag)
    {
        return;
    }
    auto sink_cout = std::make_shared<AixLog::SinkCout>(AixLog::Severity::debug);
    auto sink_file = std::make_shared<AixLog::SinkFile>(AixLog::Severity::debug, std::string(MOD_NAME) + ".log");
    AixLog::Log::init({sink_cout, sink_file});
    LOG(INFO) << "Starting " << MOD_TITLE << " (" << __DATE__ << ")\nAuthor: Grinch_\nDiscord: " << DISCORD_INVITE << "\nPatreon: " << PATREON_LINK << "\nMore Info: " << GITHUB_LINK;

    // date time
    SYSTEMTIME st;
    GetSystemTime(&st);
    LOG(INFO) << "Date: " << st.wYear << "-" << st.wMonth << "-" << st.wDay << " Time: " << st.wHour << ":" << st.wMinute;
    LOG(INFO) << "\nDonators:";
    for (const auto &name : donators)
    {
        LOG(INFO) << "- " << name;
    }

    flag = false;
}

BOOL WINAPI DllMain(HINSTANCE hDllHandle, DWORD nReason, LPVOID Reserved)
{
    if (nReason == DLL_PROCESS_ATTACH)
    {
#if !PATRON_BUILD
        if (gConfig.ReadBoolean("CONFIG", "ShowDonationPopup", true))
        {
            ShowDonationWindow();
            gConfig.WriteBoolean("CONFIG", "ShowDonationPopup", false);
        }
#endif
        if (gConfig.ReadBoolean("CONFIG", "DeveloperMode", false))
        {
            InjectImGuiHooks();
            LOG(INFO) << "DeveloperMode enabled, injecting ImGui hooks...";
        }

        gVerboseLogging = gConfig.ReadBoolean("CONFIG", "VerboseLogging", false);

        Events::initScriptsEvent.after += []()
        {
            if (!gVerboseLogging)
            {
                LOG(INFO) << "Enable 'VerboseLogging' in ModelExtras.ini to display model-related errors.";
            }
        };

        Events::initRwEvent.after += []()
        {
            bool fxFuncs = GetModuleHandle("FxsFuncs.asi");
            bool ola = GetModuleHandle("III.VC.SA.LimitAdjuster.asi");
            bool fla = GetModuleHandle("$fastman92limitAdjuster.asi");
            bool cleo = GetModuleHandle("CLEO.asi");

            if ((fxFuncs && !(ola || fla)))
            {
                std::string str = "Install any of the below,\n\n";

                str += "- Open Limit Adjuster (Recommanded)\n";
                str += "- Fastman92 Limit Adjuster (Increase IDE limits)\n";
                MessageBox(RsGlobal.ps->window, str.c_str(), "LimitAdjuster required!", MB_OK);
            }

            if (!cleo)
            {
                MessageBox(RsGlobal.ps->window, "CLEO Library 4.4 or above is required!", "ModelExtras", MB_OK);
                LOG(ERROR) << "CLEO Library 4.4 or above is required!";
            }
        };

        Events::initGameEvent += []()
        {
            bool CLEOInstalled = GetModuleHandle("CLEO.asi");
            bool ImVehFtInstalled = GetModuleHandle("ImVehFt.asi");
            bool ImVehFtFixInstalled = GetModuleHandle("ImVehFtFix.asi");
            bool AVSInstalled = GetModuleHandle("AdvancedVehicleSirens.asi");
            bool PedFuncs = GetModuleHandle("PedFuncs.asi");
            bool BackFireZAZInstalled = false;
            bool BackFireJDRInstalled = false;

            if (CLEOInstalled)
            {
                int script = NULL;
                plugin::Command<GET_SCRIPT_STRUCT_NAMED>("IFLAME", &script);
                BackFireZAZInstalled = script != NULL;
                plugin::Command<GET_SCRIPT_STRUCT_NAMED>("Backfir", &script);
                BackFireJDRInstalled = script != NULL;
            }

            InitLogFile();

            /*
                Had to put this in place since some people put the folder in root
                directory and the asi in modloader. Why??
            */
            if (!std::filesystem::is_directory(PLUGIN_PATH((char *)MOD_NAME)))
            {
                std::string msg = std::format("{} folder not found. You need to put both '{}.asi' & '{}' folder in the same directory", MOD_NAME, MOD_NAME, MOD_NAME);
                LOG(ERROR) << msg.c_str();
                MessageBox(RsGlobal.ps->window, msg.c_str(), MOD_NAME, MB_ICONERROR);
                return TRUE;
            }

            if (gConfig.ReadBoolean("CONFIG", "ShowIncompatibleWarning", true) && (BackFireJDRInstalled || BackFireZAZInstalled || ImVehFtInstalled || ImVehFtFixInstalled || AVSInstalled))
            {
                std::string str = "ModelExtras contain the functions of these plugins,\n\n";

                if (ImVehFtInstalled)
                    str += "- ImVehFt.asi\n";
                if (ImVehFtFixInstalled)
                    str += "- ImVehFtFix.asi\n";
                if (AVSInstalled)
                    str += "- AdvancedVehicleSirens.asi\n";
                if (PedFuncs)
                    str += "- PedFuncs.asi\n";
                if (BackFireZAZInstalled)
                    str += "- Back-fire.cs by ZAZ\n";
                if (BackFireJDRInstalled)
                    str += "- Backfire - ALS.cs by Junior-Djjr\n";

                str += "\nRemove them to continue playing the game.";
                MessageBox(RsGlobal.ps->window, str.c_str(), "Incompatible plugins found!", MB_OK);
                LOG(ERROR) << str;
                exit(EXIT_FAILURE);
            }
            return TRUE;
        };
        FeatureMgr::Initialize();
    }
    return TRUE;
}