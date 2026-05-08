using MS2.GCPrototype;

var gc = new GCManager();

// ── Value bit layout sanity checks ──────────────────────────────────────────
Console.WriteLine("=== Bit layout checks ===");
var nullVal = Value.Null;
var intVal  = Value.MakeInt(42);
var dblVal  = Value.MakeDouble(3.14);
var tinyStr = Value.MakeTinyString("hi");
Console.WriteLine($"null  IsNull={nullVal.IsNull}  bits=0x{nullVal.Bits:X16}");
Console.WriteLine($"42    IsInt={intVal.IsInt}    bits=0x{intVal.Bits:X16}");
Console.WriteLine($"3.14  IsDouble={dblVal.IsDouble}  bits=0x{dblVal.Bits:X16}");
Console.WriteLine($"\"hi\"  IsTinyStr={tinyStr.IsTinyStr}  val=\"{tinyStr}\"  bits=0x{tinyStr.Bits:X16}");

// ── Allocate some GC objects ─────────────────────────────────────────────────
Console.WriteLine("\n=== Allocating GC objects ===");

Value s1 = gc.NewString("hello");
Value s2 = gc.NewString("world");
Console.WriteLine($"s1: IsString={s1.IsString}  gcSet={s1.GCSetIndex}  idx={s1.ItemIndex}  bits=0x{s1.Bits:X16}");
Console.WriteLine($"s2: IsString={s2.IsString}  gcSet={s2.GCSetIndex}  idx={s2.ItemIndex}");

Value list = gc.NewList();
gc.GetList(list).Push(s1);
gc.GetList(list).Push(Value.MakeInt(99));
Console.WriteLine($"list: IsList={list.IsList}  count={gc.GetList(list).Count}");

Value map = gc.NewMap();
gc.GetMap(map).Set(Value.MakeTinyString("key"), s2);
Console.WriteLine($"map: IsMap={map.IsMap}  count={gc.GetMap(map).Count}");

Value err = gc.NewError(gc.NewString("something went wrong"), Value.Null, Value.Null, Value.Null);
Console.WriteLine($"err: IsError={err.IsError}");

// Allocate some unreachable objects (no root, no retain)
Value dead1 = gc.NewString("I will be collected");
Value dead2 = gc.NewList();
_ = dead1; _ = dead2;   // suppress warnings; these C# vars don't make them GC roots

Console.WriteLine("\nBefore GC:");
gc.PrintStats();

// ── Root the live objects and collect ────────────────────────────────────────
gc.AddRoot(list);   // list → s1 → survives; list item int(99) survives
gc.AddRoot(map);    // map → s2 → survives
gc.AddRoot(err);    // error message string survives

gc.CollectGarbage();

Console.WriteLine("\nAfter GC (dead1, dead2, s1/s2 survive via list/map roots):");
gc.PrintStats();

// ── Verify live objects still work ──────────────────────────────────────────
Console.WriteLine("\n=== Verify live data ===");
Console.WriteLine($"list[0] = {gc.GetList(list).Get(0)}  (should be <gc:0[0]>)");
Console.WriteLine($"list[1] = {gc.GetList(list).Get(1)}  (should be 42... wait, 99)");
ref GCMap m = ref gc.GetMap(map);
if (m.TryGet(Value.MakeTinyString("key"), out Value mapVal))
    Console.WriteLine($"map[\"key\"] gcSet={mapVal.GCSetIndex} idx={mapVal.ItemIndex}  data=\"{gc.GetString(mapVal).Data}\"");

Console.WriteLine($"error msg data: \"{gc.GetString(gc.GetError(err).Message).Data}\"");

// ── Retain / Release ─────────────────────────────────────────────────────────
Console.WriteLine("\n=== Retain/Release ===");
Value retained = gc.NewString("retained string");
gc.RetainValue(retained);   // keep alive without being in root set

gc.CollectGarbage();        // root set unchanged, but retained survives
Console.WriteLine($"After GC with retain: Strings live={gc.Strings.LiveCount}");
Console.WriteLine($"retained data: \"{gc.GetString(retained).Data}\"");

gc.ReleaseValue(retained);
gc.CollectGarbage();        // now it should be swept
Console.WriteLine($"After release + GC: Strings live={gc.Strings.LiveCount}");

// ── Handle with dispose callback ─────────────────────────────────────────────
Console.WriteLine("\n=== Handle dispose ===");
bool disposed = false;
Value handle = gc.NewHandle(new object(), _ => { disposed = true; Console.WriteLine("  Handle disposed!"); });
gc.CollectGarbage();   // handle has no root → swept immediately
Console.WriteLine($"disposed={disposed}  (should be True)");

Console.WriteLine("\nDone.");
