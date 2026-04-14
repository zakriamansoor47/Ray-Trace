//
// Created by Michal Přikryl on 10.10.2025.
// Copyright (c) 2025 slynxcz. All rights reserved.
//

using System.Buffers;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;
using CounterStrikeSharp.API;
using CounterStrikeSharp.API.Core;
using CounterStrikeSharp.API.Core.Attributes.Registration;
using CounterStrikeSharp.API.Core.Capabilities;
using CounterStrikeSharp.API.Modules.Commands;
using CounterStrikeSharp.API.Modules.Memory;
using CounterStrikeSharp.API.Modules.Utils;
using RayTraceAPI;
using Vector = CounterStrikeSharp.API.Modules.Utils.Vector;

namespace RayTraceImpl;

// ReSharper disable once InconsistentNaming
// ReSharper disable once UnusedType.Global
public class RayTraceImpl : BasePlugin
{
    public override string ModuleName => "RayTraceImpl";
    public override string ModuleVersion => "v1.0.0";
    public override string ModuleAuthor => "Slynx";

    internal static PluginCapability<CRayTraceInterface> RayTraceInterface { get; } =
        new("raytrace:craytraceinterface");

    public override void Load(bool hotReload)
    {
        Capabilities.RegisterPluginCapability(RayTraceInterface, () => new CRayTrace());
        RegisterListener<Listeners.OnMetamodAllPluginsLoaded>(OnMetamodAllPluginsLoaded);
    }

    public override void Unload(bool hotReload)
    {
        RemoveListener<Listeners.OnMetamodAllPluginsLoaded>(OnMetamodAllPluginsLoaded);
    }

    private void OnMetamodAllPluginsLoaded()
    {
        if (!NativeBridge.Initialize())
        {
            Prints.ServerLog("[RayTraceImpl] Native bridge initialization failed.", ConsoleColor.Red);
            return;
        }

        Prints.ServerLog("[RayTraceImpl] Managed side initialized.", ConsoleColor.Green);
    }
}

public class CRayTrace : CRayTraceInterface
{
    public TraceResult TraceShape(Vector start, QAngle angles, CEntityInstance ignore, TraceOptions options)
    {
        unsafe
        {
            if (!NativeBridge.Loaded)
                return default;

            VectorNative startNative = new() { X = start.X, Y = start.Y, Z = start.Z };
            QAngleNative angNative = new() { X = angles.X, Y = angles.Y, Z = angles.Z };
            TraceOptions optCopy = options;

            return NativeBridge.TraceShape!(
                NativeBridge.Handle,
                (nint)(&startNative),
                (nint)(&angNative),
                ignore.Handle,
                (nint)(&optCopy)
            );
        }
    }

    public TraceResult TraceEndShape(Vector start, Vector end, CEntityInstance ignore, TraceOptions options)
    {
        unsafe
        {
            if (!NativeBridge.Loaded)
                return default;

            VectorNative s = new() { X = start.X, Y = start.Y, Z = start.Z };
            VectorNative e = new() { X = end.X, Y = end.Y, Z = end.Z };
            TraceOptions opt = options;

            return NativeBridge.TraceEndShape!(
                NativeBridge.Handle,
                (nint)(&s),
                (nint)(&e),
                ignore.Handle,
                (nint)(&opt)
            );
        }
    }

    public TraceResult TraceHullShape(Vector start, Vector end, Vector mins, Vector maxs, CEntityInstance ignore, TraceOptions options)
    {
        unsafe
        {
            if (!NativeBridge.Loaded)
                return default;

            VectorNative s = new() { X = start.X, Y = start.Y, Z = start.Z };
            VectorNative e = new() { X = end.X, Y = end.Y, Z = end.Z };
            VectorNative mi = new() { X = mins.X, Y = mins.Y, Z = mins.Z };
            VectorNative ma = new() { X = maxs.X, Y = maxs.Y, Z = maxs.Z };
            TraceOptions opt = options;

            return NativeBridge.TraceHullShape!(
                NativeBridge.Handle,
                (nint)(&s),
                (nint)(&e),
                (nint)(&mi),
                (nint)(&ma),
                ignore.Handle,
                (nint)(&opt)
            );
        }
    }

    public TraceResult TraceShapeEx(Vector start, Vector end, nint filter, nint ray)
    {
        unsafe
        {
            if (!NativeBridge.Loaded)
                return default;

            VectorNative s = new() { X = start.X, Y = start.Y, Z = start.Z };
            VectorNative e = new() { X = end.X, Y = end.Y, Z = end.Z };

            return NativeBridge.TraceShapeEx!(
                NativeBridge.Handle,
                (nint)(&s),
                (nint)(&e),
                filter,
                ray
            );
        }
    }
}

public static class NativeBridge
{
    public static nint Handle;
    public static bool Loaded;

    public static unsafe Func<nint, nint, nint, nint, nint, TraceResult>? TraceShape;
    public static unsafe Func<nint, nint, nint, nint, nint, TraceResult>? TraceEndShape;
    public static unsafe Func<nint, nint, nint, nint, nint, nint, nint, TraceResult>? TraceHullShape;
    public static unsafe Func<nint, nint, nint, nint, nint, TraceResult>? TraceShapeEx;

    public static bool Initialize()
    {
        Handle = (nint)Utilities.MetaFactory("CRayTraceInterface001")!;

        if (Handle == 0)
            return false;

        Bind();
        Loaded = true;
        return true;
    }

    private static void Bind()
    {
        int shape, end, hull, ex;

        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            shape = 1;
            end = 2;
            hull = 3;
            ex = 4;
        }
        else
        {
            shape = 2;
            end = 3;
            hull = 4;
            ex = 5;
        }

        TraceShape = VirtualFunction.Create<nint, nint, nint, nint, nint, TraceResult>(Handle, shape);
        TraceEndShape = VirtualFunction.Create<nint, nint, nint, nint, nint, TraceResult>(Handle, end);
        TraceHullShape = VirtualFunction.Create<nint, nint, nint, nint, nint, nint, nint, TraceResult>(Handle, hull);
        TraceShapeEx = VirtualFunction.Create<nint, nint, nint, nint, nint, TraceResult>(Handle, ex);
    }
}

public static class Prints
{
    public static void ServerLog(string msg, ConsoleColor color = ConsoleColor.White)
    {
        Console.ForegroundColor = color;
        Console.WriteLine(msg);
        Console.ResetColor();
    }
}