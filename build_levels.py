"""
THE LAST SIGNAL - level builder (Unreal Editor Python, UE 5.4 API).

Creates the campaign maps and places the C++ gameplay classes: doors, pickups, documents,
puzzles, checkpoints, creatures, the level director, the ending terminal, lighting (Lumen,
volumetric fog), a navmesh bounds volume and placeholder geometry.

Run inside the editor: Tools > Execute Python Script > Content/Python/build_levels.py

Placeholder geometry is built from engine cubes. Replace it with modelled, Nanite-enabled meshes
once art is in. Actor classes and property names must stay in sync with the C++ headers.

Not executed in the environment that produced it. Enum and struct fields that the Python API
cannot set reliably are listed in Docs/README.md section 5 and must be set by hand.
"""

import unreal

GAME = "/Script/TheLastSignal"
CUBE = "/Engine/BasicShapes/Cube"
CUBE_SIZE = 100.0  # engine cube edge length; scale = desired size / 100

log = unreal.log
log_warning = unreal.log_warning


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def cls(path):
    c = unreal.load_class(None, path)
    if c is None:
        raise RuntimeError(f"Class not found: {path}. Did the C++ module compile?")
    return c


def spawn(class_path, loc=(0, 0, 0), rot=(0, 0, 0)):
    return unreal.EditorLevelLibrary.spawn_actor_from_class(
        cls(class_path), unreal.Vector(*loc), unreal.Rotator(*rot))


def set_name(actor, prop, value):
    actor.set_editor_property(prop, unreal.Name(value))


def set_text(actor, prop, value):
    actor.set_editor_property(prop, unreal.Text(value))


def set_enum(actor, prop, enum_name, value_name, fallback_int):
    """Enum properties: try the Python enum first, then fall back to the integer value."""
    enum_type = getattr(unreal, enum_name, None)
    try:
        value = getattr(enum_type, value_name) if enum_type else fallback_int
        actor.set_editor_property(prop, value)
    except Exception as exc:  # the API varies between minor versions
        log_warning(f"Could not set {prop} on {actor.get_name()} ({exc}); set it by hand.")
        actor.set_editor_property(prop, fallback_int)


def add_tag(actor, tag):
    tags = list(actor.get_editor_property("tags"))
    tags.append(unreal.Name(tag))
    actor.set_editor_property("tags", tags)


def cube(label, center, size):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(*center), unreal.Rotator(0, 0, 0))
    actor.set_actor_label(label)
    actor.static_mesh_component.set_static_mesh(unreal.EditorAssetLibrary.load_asset(CUBE))
    actor.set_actor_scale3d(unreal.Vector(size[0] / CUBE_SIZE, size[1] / CUBE_SIZE, size[2] / CUBE_SIZE))
    return actor


def room(prefix, x0, y0, x1, y1, height=320.0, gaps=None, wall=20.0):
    """
    Floor, ceiling and four walls.

    gaps: list of (side, center, width) doorways. side is 'N' (y1 wall), 'S' (y0 wall),
    'E' (x1 wall) or 'W' (x0 wall). center is the ABSOLUTE world coordinate of the opening's
    middle along that wall (y for E/W walls, x for N/S walls).
    """
    gaps = gaps or []
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    cube(f"{prefix}_Floor", (cx, cy, -wall / 2), (x1 - x0, y1 - y0, wall))
    cube(f"{prefix}_Ceiling", (cx, cy, height + wall / 2), (x1 - x0, y1 - y0, wall))

    for side in ("N", "S", "E", "W"):
        along_start, along_end = (x0, x1) if side in ("N", "S") else (y0, y1)
        openings = sorted((c - w / 2, c + w / 2) for s, c, w in gaps if s == side)
        pieces, cursor = [], along_start
        for a, b in openings:
            if a > cursor:
                pieces.append((cursor, a))
            cursor = max(cursor, b)
        if cursor < along_end:
            pieces.append((cursor, along_end))

        for i, (a, b) in enumerate(pieces):
            mid, length = (a + b) / 2, b - a
            if side == "N":
                cube(f"{prefix}_WallN{i}", (mid, y1, height / 2), (length, wall, height))
            elif side == "S":
                cube(f"{prefix}_WallS{i}", (mid, y0, height / 2), (length, wall, height))
            elif side == "E":
                cube(f"{prefix}_WallE{i}", (x1, mid, height / 2), (wall, length, height))
            else:
                cube(f"{prefix}_WallW{i}", (x0, mid, height / 2), (wall, length, height))


def light(loc, intensity=2500.0, color=(1.0, 0.85, 0.7), flicker=False, radius=900.0):
    actor = spawn("/Script/Engine.PointLight", loc)
    comp = actor.point_light_component
    comp.set_editor_property("intensity", intensity)
    comp.set_editor_property("light_color", unreal.Color(*[int(c * 255) for c in color], 255))
    comp.set_editor_property("attenuation_radius", radius)
    comp.set_editor_property("source_radius", 20.0)
    comp.set_editor_property("cast_shadows", True)
    if flicker:
        add_tag(actor, "TLS_Flicker")  # the horror director flickers anything with this tag
    return actor


def nav_bounds(min_xyz, max_xyz):
    center = [(a + b) / 2 for a, b in zip(min_xyz, max_xyz)]
    size = [b - a for a, b in zip(min_xyz, max_xyz)]
    vol = spawn("/Script/NavigationSystem.NavMeshBoundsVolume", center)
    # The brush default is ~200 units per axis. Check the result in the viewport and adjust.
    vol.set_actor_scale3d(unreal.Vector(size[0] / 200.0, size[1] / 200.0, size[2] / 200.0))
    return vol


def environment(fog_color, fog_density, sun_intensity, sun_pitch=-35.0):
    sun = spawn("/Script/Engine.DirectionalLight", (0, 0, 2000), (sun_pitch, 45, 0))
    sun.directional_light_component.set_editor_property("intensity", sun_intensity)
    sun.directional_light_component.set_editor_property("cast_shadows", True)
    spawn("/Script/Engine.SkyAtmosphere", (0, 0, 0))
    sky = spawn("/Script/Engine.SkyLight", (0, 0, 500))
    sky.light_component.set_editor_property("real_time_capture", True)

    fog = spawn("/Script/Engine.ExponentialHeightFog", (0, 0, 0))
    fog.component.set_editor_property("fog_density", fog_density)
    fog.component.set_editor_property("fog_inscattering_color", unreal.LinearColor(*fog_color, 1.0))
    fog.component.set_editor_property("volumetric_fog", True)
    fog.component.set_editor_property("volumetric_fog_scattering_distribution", 0.2)

    ppv = spawn("/Script/Engine.PostProcessVolume", (0, 0, 0))
    ppv.set_editor_property("unbound", True)
    s = ppv.settings
    s.set_editor_property("override_bloom_intensity", True)
    s.set_editor_property("bloom_intensity", 0.4)
    s.set_editor_property("override_film_grain_intensity", True)
    s.set_editor_property("film_grain_intensity", 0.12)
    s.set_editor_property("override_vignette_intensity", True)
    s.set_editor_property("vignette_intensity", 0.6)


def checkpoint(loc, checkpoint_id, chapter):
    chk = spawn(f"{GAME}.TLSCheckpoint", loc)
    set_name(chk, "CheckpointId", checkpoint_id)
    chk.set_editor_property("ChapterIndex", chapter)
    return chk


def pickup(loc, item_id, count=1, persistent=None):
    p = spawn(f"{GAME}.TLSPickup", loc)
    set_name(p, "ItemId", item_id)
    p.set_editor_property("Count", count)
    set_name(p, "PersistentId", persistent or f"pickup_{item_id}_{int(loc[0])}_{int(loc[1])}")
    return p


def battery(loc, persistent, amount=45.0):
    b = spawn(f"{GAME}.TLSPickup", loc)
    b.set_editor_property("bIsBattery", True)
    b.set_editor_property("BatteryAmount", amount)
    set_name(b, "PersistentId", persistent)
    return b


def document(loc, doc_id, title, body, flag_on_read=None):
    d = spawn(f"{GAME}.TLSDocument", loc)
    set_name(d, "DocumentId", doc_id)
    set_name(d, "PersistentId", doc_id)
    set_text(d, "Title", title)
    set_text(d, "Body", body)
    if flag_on_read:
        set_name(d, "FlagOnRead", flag_on_read)
    return d


def door(loc, yaw=0.0, key_id=None):
    d = spawn(f"{GAME}.TLSDoor", loc, (0, yaw, 0))
    if key_id:
        set_name(d, "RequiredKeyId", key_id)
    return d


def creature(loc, kind, dormant):
    c = spawn(f"{GAME}.TLSCreature", loc)
    set_enum(c, "CreatureType", "ETLSCreatureType", kind, {"Stalker": 0, "Hunter": 1, "Lurker": 2}[kind])
    c.set_editor_property("bStartDormant", dormant)
    return c


def puzzle(loc, yaw, code, solved_flag, required_flag=None):
    t = spawn(f"{GAME}.TLSPuzzleTerminal", loc, (0, yaw, 0))
    t.set_editor_property("CorrectCode", code)
    set_name(t, "SolvedFlag", solved_flag)
    if required_flag:
        set_name(t, "RequiredFlag", required_flag)
    return t


def level_exit(loc, target, required_flags=None, locked_text=None, yaw=0.0):
    e = spawn(f"{GAME}.TLSLevelExit", loc, (0, yaw, 0))
    set_name(e, "TargetLevel", target)
    if required_flags:
        e.set_editor_property("RequiredFlags", [unreal.Name(f) for f in required_flags])
    if locked_text:
        set_text(e, "LockedText", locked_text)
    return e


def level_director(tension_scale=1.0, flicker_threshold=0.3, interval=(25.0, 60.0)):
    d = spawn(f"{GAME}.TLSLevelDirector", (0, 0, 0))
    d.set_editor_property("TensionScale", tension_scale)
    d.set_editor_property("FlickerThreshold", flicker_threshold)
    d.set_editor_property("AmbientScareInterval", unreal.Vector2D(*interval))
    return d


def player_start(loc, yaw=0.0):
    return spawn("/Script/Engine.PlayerStart", loc, (0, yaw, 0))


def ensure_level(path):
    subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    if not subsystem.new_level(path):
        raise RuntimeError(f"Could not create level {path}")


def save_level():
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()


# ---------------------------------------------------------------------------
# Levels
# ---------------------------------------------------------------------------

def build_main_menu():
    ensure_level("/Game/Maps/MainMenu")
    environment(fog_color=(0.02, 0.02, 0.03), fog_density=0.05, sun_intensity=0.2)
    room("Menu", -800, -800, 800, 800, height=400)
    light((0, 0, 300), intensity=600, color=(0.5, 0.7, 1.0), radius=1200)
    player_start((0, 0, 100))
    save_level()


def build_forest_01():
    ensure_level("/Game/Maps/Forest_01")
    environment(fog_color=(0.04, 0.06, 0.05), fog_density=0.035, sun_intensity=0.4, sun_pitch=-12)

    cube("Ground", (0, 0, -10), (12000, 12000, 20))
    for i, (x, y) in enumerate([(-1400, 900), (-300, 1500), (1100, 800), (1700, -400), (-1800, -300),
                                (600, -1300), (-900, -1400), (300, 300), (-200, -800)]):
        cube(f"Trunk{i}", (x, y, 900), (90, 90, 1800))

    # Gatehouse: the path into the facility. Opening on the west side faces the player's start.
    room("Gatehouse", 2000, -500, 2800, 500, height=380, gaps=[("W", 0, 300)])

    player_start((-800, 0, 100))
    checkpoint((-600, 0, 100), "forest_start", 0)
    document((2600, 0, 100), "doc_gate_log", "Gate Log - Day 1",
             "Perimeter sensors went dark two weeks ago. Night crew reports a signal on the old band, "
             "always at the same frequency. Nobody has been able to trace it.")
    creature((1200, 300, 100), "Stalker", dormant=True)
    level_exit((2740, 0, 100), "Facility_01", locked_text=None)

    level_director(tension_scale=0.8)
    nav_bounds((-2000, -2000, -50), (3000, 2000, 1000))
    save_level()


def build_facility_01():
    ensure_level("/Game/Maps/Facility_01")
    environment(fog_color=(0.03, 0.04, 0.05), fog_density=0.05, sun_intensity=0.05)

    # Hall (0..1400 x 0..1000) -> corridor (y 400..600) -> storage (east), control room (north).
    room("Hall", 0, 0, 1400, 1000, height=340, gaps=[("E", 500, 200), ("N", 500, 220)])
    room("Corridor", 1400, 400, 3400, 600, height=300, gaps=[("E", 500, 200)])
    room("Storage", 3400, 0, 4800, 1000, height=340, gaps=[("W", 500, 200)])
    room("Control", 0, 1000, 1400, 2400, height=340, gaps=[("S", 500, 220)])

    light((700, 500, 280), flicker=True)
    light((2400, 500, 260), flicker=True)
    light((4100, 500, 300))
    light((700, 1700, 300), flicker=True, intensity=1800, color=(0.6, 0.8, 1.0))

    pickup((300, 800, 60), "key_storage", persistent="pickup_key_storage")
    door((3400, 500, 0), yaw=90.0, key_id="key_storage")

    battery((1100, 700, 60), "battery_fac_0")
    battery((2800, 500, 60), "battery_fac_1")
    battery((4300, 500, 60), "battery_fac_2")

    # Generator keypad: opens with the code from the generator note. Sets generator_on.
    puzzle((300, 1950, 120), 270.0, "2207", solved_flag="generator_on")
    # Security keypad: needs power. Sets security_unlocked, which opens the lower-level elevator.
    puzzle((700, 1950, 120), 270.0, "4071", solved_flag="security_unlocked", required_flag="generator_on")
    level_exit((4650, 500, 100), "Underground_01", required_flags=["security_unlocked"],
               locked_text="Sealed. Security must unlock the lower level.")

    document((300, 1500, 110), "doc_security_memo", "Security Memo",
             "The lab sealed the lower level after the third incident. Do not open the drain doors. "
             "The signal comes from below.")
    document((800, 1500, 110), "doc_generator", "Generator Note",
             "The main generator in the control room needs a manual restart. Flip it before anyone "
             "tries the keypad.", flag_on_read="generator_hint")

    player_start((200, 200, 100))
    checkpoint((1200, 500, 100), "facility_hall", 1)
    creature((2500, 500, 100), "Stalker", dormant=True)
    creature((4300, 800, 100), "Lurker", dormant=True)

    level_director(tension_scale=1.0, flicker_threshold=0.25, interval=(20.0, 45.0))
    nav_bounds((-200, -200, -50), (4900, 2500, 800))
    save_level()


def build_underground_01():
    ensure_level("/Game/Maps/Underground_01")
    environment(fog_color=(0.02, 0.03, 0.02), fog_density=0.07, sun_intensity=0.02)

    # Tunnel (y 0..500) -> lab (x 3000..5200) -> vault (x 5200..6200).
    room("Tunnel", 0, 0, 3000, 500, height=280, gaps=[("E", 250, 200)])
    room("Lab", 3000, -300, 5200, 1100, height=340, gaps=[("W", 250, 200), ("E", 400, 200)])
    room("Vault", 5200, 0, 6200, 800, height=320, gaps=[("W", 400, 200)])

    light((600, 250, 230), flicker=True)
    light((3600, 400, 280), flicker=True, intensity=1500)
    light((5700, 400, 280))

    player_start((200, 250, 100))
    checkpoint((2800, 250, 100), "underground_tunnel", 2)
    creature((1500, 250, 100), "Lurker", dormant=True)
    creature((4300, 400, 100), "Hunter", dormant=False)

    ending = spawn(f"{GAME}.TLSEndingTerminal", (5800, 400, 120))
    set_name(ending, "PersistentId", "ending_terminal")

    level_director(tension_scale=1.2, flicker_threshold=0.2, interval=(15.0, 35.0))
    nav_bounds((-200, -200, -50), (6400, 1200, 800))
    save_level()


MAPS = [
    ("MainMenu", build_main_menu),
    ("Forest_01", build_forest_01),
    ("Facility_01", build_facility_01),
    ("Underground_01", build_underground_01),
]


def main():
    log("=== THE LAST SIGNAL: building levels ===")
    for name, builder in MAPS:
        log(f"Building {name} ...")
        builder()
        log(f"Done {name}")
    log("=== Levels built. Finish the manual steps in Docs/README.md section 5. ===")


if __name__ == "__main__":
    main()
