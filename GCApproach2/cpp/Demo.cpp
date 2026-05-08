#include "GCManager.h"
#include <cstdio>
#include <cstring>

using namespace MS2::GCPrototype;

int main() {

    // ── Value bit layout sanity checks ──────────────────────────────────────
    printf("=== Bit layout checks ===\n");
    Value nullVal = Value::Null;
    Value intVal  = Value::MakeInt(42);
    Value dblVal  = Value::MakeDouble(3.14);
    Value tinyStr = Value::MakeTinyString("hi");
    printf("null  IsNull=%s  bits=0x%016llX\n", nullVal.IsNull() ? "True" : "False", (unsigned long long)nullVal.Bits());
    printf("42    IsInt=%s   bits=0x%016llX\n", intVal.IsInt()   ? "True" : "False", (unsigned long long)intVal.Bits());
    printf("3.14  IsDouble=%s  bits=0x%016llX\n", dblVal.IsDouble() ? "True" : "False", (unsigned long long)dblVal.Bits());
    printf("\"hi\"  IsTinyStr=%s  val=\"%s\"  bits=0x%016llX\n",
        tinyStr.IsTinyStr() ? "True" : "False", tinyStr.TinyToString().c_str(), (unsigned long long)tinyStr.Bits());

    // ── Allocate some GC objects ─────────────────────────────────────────────
    printf("\n=== Allocating GC objects ===\n");

    GCManager gc;
    Value s1   = gc.NewString("hello");
    Value s2   = gc.NewString("world");
    printf("s1: IsString=%s  gcSet=%d  idx=%d  bits=0x%016llX\n",
        s1.IsString() ? "True" : "False", s1.GCSetIndex(), s1.ItemIndex(), (unsigned long long)s1.Bits());
    printf("s2: IsString=%s  gcSet=%d  idx=%d\n",
        s2.IsString() ? "True" : "False", s2.GCSetIndex(), s2.ItemIndex());

    Value list = gc.NewList();
    gc.GetList(list).Push(s1);
    gc.GetList(list).Push(Value::MakeInt(99));
    printf("list: IsList=%s  count=%d\n", list.IsList() ? "True" : "False", gc.GetList(list).Count());

    Value map = gc.NewMap();
    gc.GetMap(map).Set(Value::MakeTinyString("key"), s2);
    printf("map: IsMap=%s  count=%d\n", map.IsMap() ? "True" : "False", gc.GetMap(map).Count());

    Value err = gc.NewError(gc.NewString("something went wrong"), Value::Null, Value::Null, Value::Null);
    printf("err: IsError=%s\n", err.IsError() ? "True" : "False");

    // Allocate some unreachable objects (no root, no retain)
    Value dead1 = gc.NewString("I will be collected");
    Value dead2 = gc.NewList();
    (void)dead1; (void)dead2;   // suppress warnings; C++ vars don't make them GC roots

    printf("\nBefore GC:\n");
    gc.PrintStats();

    // ── Root the live objects and collect ────────────────────────────────────
    gc.AddRoot(list);   // list → s1 survives; list item int(99) survives
    gc.AddRoot(map);    // map → s2 survives
    gc.AddRoot(err);    // error message string survives

    gc.CollectGarbage();

    printf("\nAfter GC (dead1, dead2 collected; s1/s2 survive via list/map roots):\n");
    gc.PrintStats();

    // ── Verify live objects still work ──────────────────────────────────────
    printf("\n=== Verify live data ===\n");
    printf("list[0] = %s  (should be <gc:0[0]>)\n", gc.GetList(list).Get(0).ToString().c_str());
    printf("list[1] = %s  (should be 99)\n",         gc.GetList(list).Get(1).ToString().c_str());

    Value mapVal;
    if (gc.GetMap(map).TryGet(Value::MakeTinyString("key"), mapVal))
        printf("map[\"key\"] gcSet=%d idx=%d  data=\"%s\"\n",
            mapVal.GCSetIndex(), mapVal.ItemIndex(), gc.GetString(mapVal).Data.c_str());

    printf("error msg data: \"%s\"\n", gc.GetString(gc.GetError(err).Message).Data.c_str());

    // ── Retain / Release ─────────────────────────────────────────────────────
    printf("\n=== Retain/Release ===\n");
    Value retained = gc.NewString("retained string");
    gc.RetainValue(retained);   // keep alive without being in root set

    gc.CollectGarbage();        // root set unchanged, but retained survives
    printf("After GC with retain: Strings live=%d\n", gc.Strings.LiveCount());
    printf("retained data: \"%s\"\n", gc.GetString(retained).Data.c_str());

    gc.ReleaseValue(retained);
    gc.CollectGarbage();        // now it should be swept
    printf("After release + GC: Strings live=%d\n", gc.Strings.LiveCount());

    // ── Handle with dispose callback ─────────────────────────────────────────
    printf("\n=== Handle dispose ===\n");
    bool disposed = false;
    Value handle = gc.NewHandle(
        &disposed,
        [](void* p) { *static_cast<bool*>(p) = true; printf("  Handle disposed!\n"); }
    );
    gc.CollectGarbage();   // handle has no root → swept immediately
    printf("disposed=%s  (should be True)\n", disposed ? "True" : "False");

    printf("\nDone.\n");
    return 0;
}
