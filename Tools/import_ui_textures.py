import os
import unreal

SRC = "D:/Claude/LProject/LProject/Content/UI/Tex"
DEST = "/Game/UI/Tex"
DATA_TEXTURES = {"Tex_RadialMask"}  # sRGB off (angular data, not color)

pngs = [f for f in os.listdir(SRC) if f.lower().endswith(".png")]
tasks = []
for f in pngs:
    t = unreal.AssetImportTask()
    t.set_editor_property("filename", os.path.join(SRC, f))
    t.set_editor_property("destination_path", DEST)
    t.set_editor_property("automated", True)
    t.set_editor_property("save", True)
    t.set_editor_property("replace_existing", True)
    tasks.append(t)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

eal = unreal.EditorAssetLibrary
count = 0
for f in pngs:
    name = os.path.splitext(f)[0]
    path = DEST + "/" + name
    tex = eal.load_asset(path)
    if not tex:
        print("MISSING", path)
        continue
    try:
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    except Exception as e:
        print("lod_group err", name, e)
    try:
        tex.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    except Exception as e:
        print("mip err", name, e)
    try:
        tex.set_editor_property("srgb", name not in DATA_TEXTURES)
    except Exception as e:
        print("srgb err", name, e)
    try:
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_USER_INTERFACE2D)
    except Exception as e:
        print("compression err", name, e)
    eal.save_asset(path)
    count += 1

print("IMPORTED_UI_TEXTURES", count, "of", len(pngs))
