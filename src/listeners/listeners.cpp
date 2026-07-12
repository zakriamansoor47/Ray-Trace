//
// Created by Michal Přikryl on 10.07.2025.
// Copyright (c) 2025 slynxcz. All rights reserved.
//
#include "listeners.h"
#include <raytrace.h>
#include <shared.h>
#include <dynlibutils/module.h>

class GameSessionConfiguration_t
{
};

namespace RayTracePlugin::Listeners {
    SourceHooks sourceHooks;

    SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t&, ISource2WorldSession*, const char*);
    SH_DECL_HOOK2(IGameEventManager2, LoadEventsFromFile, SH_NOATTRIB, 0, int, const char*, bool);

    int g_iLoadEventsFromFileId = -1;

    void InitListeners() {
        SH_ADD_HOOK(INetworkServerService, StartupServer, shared::g_pNetworkServerService, SH_MEMBER(&sourceHooks, &SourceHooks::Hook_StartupServer), true);
        auto pCGameEventManagerVTable = DynLibUtils::CModule(shared::g_pServer).GetVirtualTableByName("CGameEventManager").RCast<IGameEventManager2*>();
        g_iLoadEventsFromFileId = SH_ADD_DVPHOOK(IGameEventManager2, LoadEventsFromFile, pCGameEventManagerVTable, SH_MEMBER(&sourceHooks, &SourceHooks::Hook_LoadEventsFromFile), false);
    }

    void DestructListeners() {
        SH_REMOVE_HOOK(INetworkServerService, StartupServer, shared::g_pNetworkServerService, SH_MEMBER(&sourceHooks, &SourceHooks::Hook_StartupServer), true);
        SH_REMOVE_HOOK_ID(g_iLoadEventsFromFileId);
    }

    void SourceHooks::Hook_StartupServer(const GameSessionConfiguration_t& config,
                                            ISource2WorldSession*, const char*)
    {
        if (!shared::g_bDetoursLoaded)
        {
            shared::g_pEntitySystem = GameEntitySystem();
            RayTrace::g_CRayTrace.Initialize();
            shared::g_bDetoursLoaded = true;
        }
    }

    int SourceHooks::Hook_LoadEventsFromFile(const char* filename, bool bSearchAll)
    {
        ExecuteOnce(shared::g_pGameEventManager = META_IFACEPTR(IGameEventManager2));
        RETURN_META_VALUE(MRES_IGNORED, 0);
    }
}