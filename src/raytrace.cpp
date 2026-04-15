//
// Created by Michal Přikryl on 30.08.2025.
// Copyright (c) 2025 slynxcz. All rights reserved.
//
#include "raytrace.h"
#include <shared.h>
#include "schema/CBaseModelEntity.h"
#include "vectorextends.h"
#include "dynlibutils/memaddr.h"
#include "dynlibutils/module.h"
#include "colors.h"
#include "log.h"

namespace RayTracePlugin::RayTrace
{
    CRayTrace g_CRayTrace;

    bool CRayTrace::Initialize()
    {
        m_pCNavPhysicsInterfaceVTable = DynLibUtils::CModule(shared::g_pServer).GetVirtualTableByName("CNavPhysicsInterface").RCast<void**>();
        if (!m_pCNavPhysicsInterfaceVTable)
        {
            FP_WARN("Tried getting virtual function from a null vtable.");
            return false;
        }

        m_pCNavPhysicsInterface_TraceShape = m_pCNavPhysicsInterfaceVTable[shared::g_pGameConfig->GetOffset("CNavPhysicsInterface_TraceShape")];
        return true;
    }

    static void DrawBeam(const Vector& start, const Vector& end, const Color& color)
    {
        CBeam* beam = UTIL_CreateEntityByName<CBeam>("env_beam");
        if (!beam) return;

        beam->m_clrRender().SetColor(color.r(), color.g(), color.b(), color.a());
        beam->m_fWidth() = 1.5f;
        beam->m_nRenderMode() = kRenderNormal;
        beam->m_nRenderFX() = kRenderFxNone;

        beam->Teleport(&start, &VectorExtends::RotationZero, &VectorExtends::VectorZero);
        beam->m_vecEndPos() = end;
        beam->DispatchSpawn();
    }

    bool CRayTrace::TraceShape(const Vector* pVecStart, const QAngle* pAngAngles, CEntityInstance* pIgnoreEntity, TraceOptions* pTraceOptions, TraceResult* pTraceResult)
    {
        CTraceFilterEx filter = pIgnoreEntity ? CTraceFilterEx(static_cast<CBaseEntity*>(pIgnoreEntity)) : CTraceFilterEx();

        filter.m_nInteractsAs = 0;
        filter.m_nInteractsWith = static_cast<uint64_t>(MASK_SHOT_PHYSICS);
        filter.m_nInteractsExclude = 0;

        if (pTraceOptions)
        {
            if (pTraceOptions->InteractsWith != static_cast<uint64_t>(MASK_SHOT_PHYSICS))
                filter.m_nInteractsWith = pTraceOptions->InteractsWith;

            if (pTraceOptions->InteractsExclude != 0)
                filter.m_nInteractsExclude = pTraceOptions->InteractsExclude;
        }

        Vector forward;
        AngleVectors(pAngAngles ? *pAngAngles : VectorExtends::RotationZero, &forward);
        Vector vecEnd{
            pVecStart ? pVecStart->x : 0 + forward.x * 8192.f,
            pVecStart ? pVecStart->y : 0 + forward.y * 8192.f,
            pVecStart ? pVecStart->z : 0 + forward.z * 8192.f
        };

        Ray_t ray;
        auto res = TraceShapeEx(pVecStart, &vecEnd, &filter, &ray, pTraceResult);

        if (pTraceOptions && pTraceOptions->DrawBeam)
        {
            Color col = res ? colors::Red().ToValveColor() : colors::Green().ToValveColor();
            DrawBeam(pVecStart ? *pVecStart : VectorExtends::VectorZero, (res && pTraceResult) ? pTraceResult->EndPos : vecEnd, col);
        }

        return res;
    }

    bool CRayTrace::TraceEndShape(const Vector* pVecStart, const Vector* pVecEnd, CEntityInstance* pIgnoreEntity, TraceOptions* pTraceOptions, TraceResult* pTraceResult)
    {
        CTraceFilterEx filter = pIgnoreEntity ? CTraceFilterEx(static_cast<CBaseEntity*>(pIgnoreEntity)) : CTraceFilterEx();

        filter.m_nInteractsAs = 0;
        filter.m_nInteractsWith = static_cast<uint64_t>(MASK_SHOT_PHYSICS);
        filter.m_nInteractsExclude = 0;

        if (pTraceOptions)
        {
            if (pTraceOptions->InteractsWith != static_cast<uint64_t>(MASK_SHOT_PHYSICS))
                filter.m_nInteractsWith = pTraceOptions->InteractsWith;

            if (pTraceOptions->InteractsExclude != 0)
                filter.m_nInteractsExclude = pTraceOptions->InteractsExclude;
        }

        Ray_t ray;
        auto res = TraceShapeEx(pVecStart, pVecEnd, &filter, &ray, pTraceResult);

        if (pTraceOptions && pTraceOptions->DrawBeam)
        {
            Color col = res ? colors::Red().ToValveColor() : colors::Green().ToValveColor();
            DrawBeam(pVecEnd ? *pVecEnd : VectorExtends::VectorZero, (res && pTraceResult) ? pTraceResult->EndPos : (pVecEnd ? *pVecEnd : VectorExtends::VectorZero), col);
        }

        return res;
    }

    bool CRayTrace::TraceHullShape(const Vector* pVecStart, const Vector* pVecEnd, const Vector* pVecMins,
                                          const Vector* pVecMaxs, CEntityInstance* pIgnoreEntity, TraceOptions* pTraceOptions, TraceResult* pTraceResult)
    {
        CTraceFilterEx filter = pIgnoreEntity ? CTraceFilterEx(static_cast<CBaseEntity*>(pIgnoreEntity)) : CTraceFilterEx();

        filter.m_nInteractsAs = 0;
        filter.m_nInteractsWith = static_cast<uint64_t>(MASK_SHOT_PHYSICS);
        filter.m_nInteractsExclude = 0;

        if (pTraceOptions)
        {
            if (pTraceOptions->InteractsWith != static_cast<uint64_t>(MASK_SHOT_PHYSICS))
                filter.m_nInteractsWith = pTraceOptions->InteractsWith;

            if (pTraceOptions->InteractsExclude != 0)
                filter.m_nInteractsExclude = pTraceOptions->InteractsExclude;
        }

        Ray_t ray;
        ray.Init(pVecMins ? *pVecMins : VectorExtends::VectorZero, pVecMaxs ? *pVecMaxs : VectorExtends::VectorZero);

        auto res = TraceShapeEx(pVecStart, pVecEnd, &filter, &ray, pTraceResult);

        if (pTraceOptions && pTraceOptions->DrawBeam)
        {
            Color col = res ? colors::Red().ToValveColor() : colors::Green().ToValveColor();
            DrawBeam(pVecEnd ? *pVecEnd : VectorExtends::VectorZero, (res && pTraceResult) ? pTraceResult->EndPos : (pVecEnd ? *pVecEnd : VectorExtends::VectorZero), col);
        }

        return res;
    }

    bool CRayTrace::TraceShapeEx(const Vector* pVecStart, const Vector* pVecEnd, CTraceFilter* pTraceFilter, Ray_t* pRay, TraceResult* pTraceResult)
    {
        if (!m_pCNavPhysicsInterface_TraceShape)
        {
            FP_ERROR("CNavPhysicsInterface::TraceShape is not bound!");
            return false;
        }

        Vector vecStartCopy = pVecStart ? *pVecStart : VectorExtends::VectorZero;
        Vector vecEndCopy = pVecEnd ? *pVecEnd : VectorExtends::VectorZero;
        CGameTrace trace;

        bool bResult = m_pCNavPhysicsInterface_TraceShape.RCast<
            bool (*)(void*, Ray_t&, Vector&, Vector&, CTraceFilter*, CGameTrace*)>()(
            nullptr, *pRay, vecStartCopy, vecEndCopy, pTraceFilter, &trace);

        *pTraceResult = TraceResult(&trace);

        return bResult;
    }
}
