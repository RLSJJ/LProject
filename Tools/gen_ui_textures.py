#!/usr/bin/env python3
"""Single source of truth for LProject UI art.

Generates every UI PNG procedurally (no external assets) into Content/UI/Tex/.
Dark-fantasy Lost Ark palette: slate base, gold + crimson accents, danger orange,
counter cyan. Rendered at 2x then downscaled for clean anti-aliasing. Idempotent.
"""
import math
import os
from PIL import Image, ImageDraw, ImageFilter, ImageFont

OUT = os.path.join(os.path.dirname(__file__), "..", "Content", "UI", "Tex")
OUT = os.path.normpath(OUT)
os.makedirs(OUT, exist_ok=True)
SS = 2  # supersample factor

# ---- palette (RGBA) ----
SLATE_D = (18, 21, 28, 255)
SLATE = (26, 31, 43, 255)
SLATE_L = (40, 47, 63, 255)
GOLD = (200, 162, 74, 255)
GOLD_L = (230, 196, 106, 255)
CRIMSON = (160, 45, 45, 255)
CRIMSON_L = (200, 70, 60, 255)
ORANGE = (230, 126, 34, 255)
CYAN = (79, 224, 255, 255)
GREEN = (63, 174, 90, 255)
TEXT = (232, 228, 216, 255)
SHADOW = (0, 0, 0, 255)


def canvas(w, h):
    return Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))


def save(img, name, w, h):
    img = img.resize((w, h), Image.LANCZOS)
    img.save(os.path.join(OUT, name + ".png"))
    print("WROTE", name, f"{w}x{h}")


def font(px):
    for fn in ("seguibl.ttf", "segoeuib.ttf", "ariblk.ttf", "arialbd.ttf"):
        try:
            return ImageFont.truetype("C:/Windows/Fonts/" + fn, px * SS)
        except Exception:
            continue
    return ImageFont.load_default()


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(4))


def rrect(d, box, r, fill=None, outline=None, width=1):
    d.rounded_rectangle(box, radius=r * SS, fill=fill, outline=outline, width=width * SS)


def vgrad(w, h, top, bottom):
    img = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    px = img.load()
    for y in range(h * SS):
        t = y / max(1, h * SS - 1)
        c = lerp(top, bottom, t)
        for x in range(w * SS):
            px[x, y] = c
    return img


def radial(w, h, inner, outer, cx=0.5, cy=0.5, power=1.0):
    img = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    px = img.load()
    W, H = w * SS, h * SS
    maxd = math.hypot(max(cx, 1 - cx) * W, max(cy, 1 - cy) * H)
    for y in range(H):
        for x in range(W):
            d = math.hypot(x - cx * W, y - cy * H) / maxd
            t = min(1.0, d) ** power
            px[x, y] = lerp(inner, outer, t)
    return img


def glow_layer(w, h, draw_fn, blur):
    g = canvas(w, h)
    draw_fn(ImageDraw.Draw(g))
    return g.filter(ImageFilter.GaussianBlur(blur * SS))


# ============================ generators ============================

def gen_title_logo():
    w, h = 2048, 768
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    f = font(150)
    txt = "BEHEMOTH"
    bb = d.textbbox((0, 0), txt, font=f)
    tw, th = bb[2] - bb[0], bb[3] - bb[1]
    x = (w * SS - tw) // 2 - bb[0]
    y = (h * SS - th) // 2 - bb[1] - 60 * SS
    # glow
    g = glow_layer(w, h, lambda dd: dd.text((x, y), txt, font=f, fill=GOLD), 18)
    img.alpha_composite(g)
    d.text((x + 4 * SS, y + 4 * SS), txt, font=f, fill=SHADOW)
    d.text((x, y), txt, font=f, fill=GOLD_L)
    # subtitle
    f2 = font(54)
    sub = "S O L O   R A I D"
    bb2 = d.textbbox((0, 0), sub, font=f2)
    d.text(((w * SS - (bb2[2] - bb2[0])) // 2 - bb2[0], y + th + 70 * SS), sub, font=f2, fill=TEXT)
    save(img, "Tex_TitleLogo", w, h)


def gen_vignettes():
    save(radial(1024, 1024, (0, 0, 0, 0), (0, 0, 0, 235), power=2.2), "Tex_Vignette_Dark", 1024, 1024)
    save(radial(1024, 1024, (0, 0, 0, 0), (150, 20, 20, 200), power=2.6), "Tex_Vignette_Red", 1024, 1024)


def gen_bar_bg():
    w, h = 512, 64
    img = vgrad(w, h, (8, 9, 13, 255), (24, 27, 36, 255))
    d = ImageDraw.Draw(img)
    rrect(d, [2 * SS, 2 * SS, (w - 2) * SS, (h - 2) * SS], 10, outline=(0, 0, 0, 255), width=3)
    rrect(d, [4 * SS, 4 * SS, (w - 4) * SS, 10 * SS], 6, fill=(255, 255, 255, 18))
    # mask to rounded
    mask = Image.new("L", (w * SS, h * SS), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, w * SS, h * SS], radius=12 * SS, fill=255)
    img.putalpha(mask)
    save(img, "Tex_BarBG", w, h)


def gen_bar_fill(name, base, light):
    w, h = 256, 48
    img = vgrad(w, h, light, base)
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w * SS, int(h * 0.42) * SS], fill=(255, 255, 255, 40))
    save(img, name, w, h)


def gen_bar_frame_gold():
    w, h = 512, 64
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    rrect(d, [2 * SS, 2 * SS, (w - 2) * SS, (h - 2) * SS], 12, outline=GOLD, width=4)
    rrect(d, [5 * SS, 5 * SS, (w - 5) * SS, (h - 5) * SS], 9, outline=(90, 70, 30, 200), width=2)
    save(img, "Tex_Bar_Frame_Gold", w, h)


def gen_notch():
    w, h = 8, 48
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    d.rectangle([3 * SS, 0, 5 * SS, h * SS], fill=(0, 0, 0, 230))
    d.rectangle([5 * SS, 0, 6 * SS, h * SS], fill=(255, 255, 255, 40))
    save(img, "Tex_Notch_Tick", w, h)


def gen_radial_mask():
    # angular sweep data texture: alpha encodes angle 0..360 clockwise from top. sRGB OFF downstream.
    w = h = 256
    img = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    px = img.load()
    W = w * SS
    cx = cy = W / 2
    rout = W / 2 - 2 * SS
    rin = W * 0.30
    for y in range(W):
        for x in range(W):
            dx, dy = x - cx, y - cy
            dist = math.hypot(dx, dy)
            if dist > rout or dist < rin:
                continue
            ang = (math.atan2(dx, -dy) / (2 * math.pi)) % 1.0  # 0 at top, clockwise
            v = int(ang * 255)
            px[x, y] = (v, v, v, 255)
    save(img, "Tex_RadialMask", w, h)


def gen_ring_frame():
    w = h = 256
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    m = 6 * SS
    d.ellipse([m, m, w * SS - m, h * SS - m], outline=GOLD, width=5 * SS)
    d.ellipse([m + 7 * SS, m + 7 * SS, w * SS - m - 7 * SS, h * SS - m - 7 * SS], outline=(70, 55, 25, 180), width=2 * SS)
    save(img, "Tex_Ring_Frame", w, h)


def gen_skill_slot_frame():
    w = h = 128
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    rrect(d, [4 * SS, 4 * SS, (w - 4) * SS, (h - 4) * SS], 14, fill=(20, 24, 33, 235), outline=GOLD, width=3)
    rrect(d, [9 * SS, 9 * SS, (w - 9) * SS, (h - 9) * SS], 10, outline=(60, 48, 22, 200), width=2)
    save(img, "Tex_SkillSlot_Frame", w, h)


def _icon_base(accent):
    w = h = 96
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    rrect(d, [6 * SS, 6 * SS, (w - 6) * SS, (h - 6) * SS], 12, fill=lerp(SLATE_D, accent, 0.18))
    return img, d, w, h


def gen_skill_icon(name, accent, kind):
    img, d, w, h = _icon_base(accent)
    c = w * SS / 2
    if kind == "attack":  # sword
        d.polygon([(c, 18 * SS), (c + 7 * SS, 60 * SS), (c - 7 * SS, 60 * SS)], fill=(220, 220, 230, 255))
        d.rectangle([c - 14 * SS, 60 * SS, c + 14 * SS, 66 * SS], fill=GOLD)
        d.rectangle([c - 4 * SS, 66 * SS, c + 4 * SS, 80 * SS], fill=(120, 90, 50, 255))
    elif kind == "dash":  # motion arrows
        for i, off in enumerate((-18, 0, 18)):
            a = 120 + i * 50
            d.polygon([(c + off * SS - 10 * SS, 40 * SS), (c + off * SS + 8 * SS, 48 * SS),
                       (c + off * SS - 10 * SS, 56 * SS)], fill=(a, a + 40, 255, 255))
    elif kind == "counter":  # shield + spark
        d.polygon([(c, 24 * SS), (c + 22 * SS, 34 * SS), (c + 18 * SS, 64 * SS), (c, 74 * SS),
                   (c - 18 * SS, 64 * SS), (c - 22 * SS, 34 * SS)], fill=CYAN, outline=(255, 255, 255, 255), width=2 * SS)
    elif kind == "charge":  # forward chevrons
        for i in range(3):
            x = 26 * SS + i * 16 * SS
            d.line([(x, 34 * SS), (x + 14 * SS, 48 * SS), (x, 62 * SS)], fill=GOLD_L, width=5 * SS)
    elif kind == "cleave":  # arc slash
        d.arc([18 * SS, 14 * SS, 78 * SS, 90 * SS], 200, 340, fill=(255, 120, 60, 255), width=8 * SS)
        d.arc([24 * SS, 20 * SS, 84 * SS, 96 * SS], 200, 340, fill=(255, 200, 120, 200), width=3 * SS)
    elif kind == "bolt":  # ranged dart
        d.line([(24 * SS, 70 * SS), (72 * SS, 26 * SS)], fill=(120, 220, 255, 255), width=6 * SS)
        d.polygon([(72 * SS, 26 * SS), (60 * SS, 30 * SS), (68 * SS, 38 * SS)], fill=(200, 245, 255, 255))
    elif kind == "awakening":  # radiant burst
        for i in range(8):
            a = i * math.pi / 4
            d.line([(c, c), (c + math.cos(a) * 34 * SS, c + math.sin(a) * 34 * SS)], fill=GOLD_L, width=4 * SS)
        d.ellipse([c - 12 * SS, c - 12 * SS, c + 12 * SS, c + 12 * SS], fill=(255, 240, 200, 255))
    save(img, name, w, h)


def gen_counter_burst():
    w = h = 512
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    c = w * SS / 2
    for i in range(16):
        a = i * math.pi / 8
        d.line([(c, c), (c + math.cos(a) * 240 * SS, c + math.sin(a) * 240 * SS)], fill=(79, 224, 255, 110), width=6 * SS)
    img = img.filter(ImageFilter.GaussianBlur(6 * SS))
    d = ImageDraw.Draw(img)
    d.ellipse([c - 120 * SS, c - 120 * SS, c + 120 * SS, c + 120 * SS], outline=CYAN, width=6 * SS)
    save(img, "Tex_Counter_Burst", w, h)


def gen_keycap(name, label):
    w = h = 64
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    rrect(d, [4 * SS, 4 * SS, (w - 4) * SS, (h - 6) * SS], 10, fill=(30, 35, 47, 255), outline=GOLD, width=2)
    rrect(d, [4 * SS, (h - 12) * SS, (w - 4) * SS, (h - 4) * SS], 10, fill=(12, 14, 19, 255))
    f = font(22 if len(label) <= 2 else 15)
    bb = d.textbbox((0, 0), label, font=f)
    d.text(((w * SS - (bb[2] - bb[0])) // 2 - bb[0], (h * SS - (bb[3] - bb[1])) // 2 - bb[1] - 3 * SS),
           label, font=f, fill=TEXT)
    save(img, name, w, h)


def gen_phase_pip(name, on):
    w = h = 32
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    col = GOLD_L if on else (60, 60, 66, 255)
    d.ellipse([6 * SS, 6 * SS, (w - 6) * SS, (h - 6) * SS], fill=col, outline=(0, 0, 0, 200), width=2 * SS)
    if on:
        d.ellipse([11 * SS, 10 * SS, 17 * SS, 16 * SS], fill=(255, 255, 240, 200))
    save(img, name, w, h)


def gen_nameplate():
    w, h = 256, 96
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    pts = [(20 * SS, 10 * SS), ((w - 20) * SS, 10 * SS), ((w - 6) * SS, h * SS / 2),
           ((w - 20) * SS, (h - 10) * SS), (20 * SS, (h - 10) * SS), (6 * SS, h * SS / 2)]
    d.polygon(pts, fill=(16, 19, 27, 220), outline=GOLD, width=3 * SS)
    save(img, "Tex_NamePlate_Frame", w, h)


def gen_underline():
    w, h = 512, 16
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    cx = w * SS / 2
    d.polygon([(cx - 200 * SS, 8 * SS), (cx, 2 * SS), (cx + 200 * SS, 8 * SS), (cx, 14 * SS)], fill=GOLD)
    d.ellipse([cx - 8 * SS, 2 * SS, cx + 8 * SS, 14 * SS], fill=GOLD_L)
    save(img, "Tex_NameUnderline_Gold", w, h)


def gen_letterbox():
    w, h = 1024, 256
    img = vgrad(w, h, (0, 0, 0, 255), (0, 0, 0, 0))
    save(img, "Tex_Letterbox_Gradient", w, h)


def gen_status_frame(name, accent):
    w = h = 64
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    rrect(d, [4 * SS, 4 * SS, (w - 4) * SS, (h - 4) * SS], 10, fill=(18, 21, 29, 235), outline=accent, width=3)
    save(img, name, w, h)


def gen_status_glyph(name, accent, kind):
    w = h = 48
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    c = w * SS / 2
    if kind == "atk":
        d.polygon([(c, 8 * SS), (c + 6 * SS, 34 * SS), (c - 6 * SS, 34 * SS)], fill=accent)
        d.polygon([(c, 6 * SS), (c + 5 * SS, 16 * SS), (c - 5 * SS, 16 * SS)], fill=(255, 255, 255, 200))
    elif kind == "def":
        d.polygon([(c, 38 * SS), (c + 14 * SS, 26 * SS), (c + 11 * SS, 10 * SS), (c - 11 * SS, 10 * SS),
                   (c - 14 * SS, 26 * SS)], fill=accent)
    elif kind == "bleed":
        d.polygon([(c, 8 * SS), (c + 11 * SS, 30 * SS), (c, 40 * SS), (c - 11 * SS, 30 * SS)], fill=accent)
    save(img, name, w, h)


def gen_portrait_frame():
    w = h = 128
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    m = 5 * SS
    d.ellipse([m, m, w * SS - m, h * SS - m], fill=(16, 19, 27, 235), outline=GOLD, width=4 * SS)
    save(img, "Tex_Portrait_Frame", w, h)


def gen_enrage_frame():
    w = h = 96
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    m = 5 * SS
    d.ellipse([m, m, w * SS - m, h * SS - m], fill=(20, 14, 14, 230), outline=ORANGE, width=4 * SS)
    save(img, "Tex_EnrageClock_Frame", w, h)


def gen_button(name, base, edge, top):
    w, h = 256, 72
    img = vgrad(w, h, top, base)
    d = ImageDraw.Draw(img)
    mask = Image.new("L", (w * SS, h * SS), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, w * SS, h * SS], radius=12 * SS, fill=255)
    img.putalpha(mask)
    rrect(d, [2 * SS, 2 * SS, (w - 2) * SS, (h - 2) * SS], 11, outline=edge, width=3)
    save(img, name, w, h)


def gen_panel():
    w = h = 512
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    rrect(d, [6 * SS, 6 * SS, (w - 6) * SS, (h - 6) * SS], 22, fill=(14, 16, 22, 235), outline=GOLD, width=3)
    rrect(d, [12 * SS, 12 * SS, (w - 12) * SS, (h - 12) * SS], 18, outline=(60, 48, 22, 160), width=2)
    save(img, "Tex_Panel_Frame", w, h)


def gen_result_banner(name, word, accent, accent_l):
    w, h = 1280, 320
    img = canvas(w, h)
    g = radial(w, h, lerp(accent, (0, 0, 0, 0), 0.2), (0, 0, 0, 0), power=1.6)
    img.alpha_composite(g)
    d = ImageDraw.Draw(img)
    f = font(150)
    bb = d.textbbox((0, 0), word, font=f)
    x = (w * SS - (bb[2] - bb[0])) // 2 - bb[0]
    y = (h * SS - (bb[3] - bb[1])) // 2 - bb[1]
    gl = glow_layer(w, h, lambda dd: dd.text((x, y), word, font=f, fill=accent), 16)
    img.alpha_composite(gl)
    d.text((x + 4 * SS, y + 4 * SS), word, font=f, fill=SHADOW)
    d.text((x, y), word, font=f, fill=accent_l)
    save(img, name, w, h)


def gen_groggy_glow():
    w, h = 512, 256
    img = radial(w, h, (255, 200, 60, 180), (255, 140, 0, 0), power=1.5)
    save(img, "Tex_Groggy_Glow", w, h)


def gen_boss_silhouette():
    w = h = 1024
    img = canvas(w, h)
    d = ImageDraw.Draw(img)
    cx = w * SS / 2
    # crude beast silhouette: body + head + horns + legs
    d.ellipse([cx - 260 * SS, 420 * SS, cx + 260 * SS, 760 * SS], fill=(0, 0, 0, 200))
    d.ellipse([cx - 120 * SS, 250 * SS, cx + 120 * SS, 470 * SS], fill=(0, 0, 0, 210))
    d.polygon([(cx - 90 * SS, 290 * SS), (cx - 150 * SS, 170 * SS), (cx - 60 * SS, 270 * SS)], fill=(0, 0, 0, 210))
    d.polygon([(cx + 90 * SS, 290 * SS), (cx + 150 * SS, 170 * SS), (cx + 60 * SS, 270 * SS)], fill=(0, 0, 0, 210))
    for lx in (-180, -70, 70, 180):
        d.rectangle([cx + lx * SS - 26 * SS, 700 * SS, cx + lx * SS + 26 * SS, 900 * SS], fill=(0, 0, 0, 210))
    d.ellipse([cx - 55 * SS, 330 * SS, cx - 25 * SS, 360 * SS], fill=(200, 40, 30, 255))
    d.ellipse([cx + 25 * SS, 330 * SS, cx + 55 * SS, 360 * SS], fill=(200, 40, 30, 255))
    img = img.filter(ImageFilter.GaussianBlur(2 * SS))
    save(img, "Tex_BossSilhouette", w, h)


def gen_damage_impact():
    w, h = 256, 128
    img = radial(w, h, (255, 230, 180, 160), (255, 200, 120, 0), power=1.4)
    save(img, "Tex_DamageText_Impact", w, h)


def main():
    gen_title_logo()
    gen_vignettes()
    gen_bar_bg()
    gen_bar_fill("Tex_Bar_Fill_Crimson", CRIMSON, CRIMSON_L)
    gen_bar_fill("Tex_Bar_Fill_Amber", (180, 130, 20, 255), (235, 195, 60, 255))
    gen_bar_fill("Tex_Bar_Fill_Green", (40, 130, 60, 255), GREEN)
    gen_bar_fill("Tex_Bar_Fill_Gold", (150, 110, 40, 255), GOLD_L)
    gen_bar_frame_gold()
    gen_notch()
    gen_radial_mask()
    gen_ring_frame()
    gen_skill_slot_frame()
    gen_skill_icon("Tex_Skill_Attack", GOLD, "attack")
    gen_skill_icon("Tex_Skill_Dash", CYAN, "dash")
    gen_skill_icon("Tex_Skill_Counter", CYAN, "counter")
    gen_skill_icon("Tex_Skill_Charge", GOLD, "charge")
    gen_skill_icon("Tex_Skill_Cleave", ORANGE, "cleave")
    gen_skill_icon("Tex_Skill_Bolt", CYAN, "bolt")
    gen_skill_icon("Tex_Skill_Awakening", GOLD_L, "awakening")
    gen_counter_burst()
    for n, l in (("Q", "Q"), ("F", "F"), ("Space", "SPC"), ("LMB", "LMB"), ("RMB", "RMB"), ("R", "R"),
                 ("W", "W"), ("E", "E")):
        gen_keycap("Tex_Key_" + n, l)
    gen_phase_pip("Tex_PhasePip_On", True)
    gen_phase_pip("Tex_PhasePip_Off", False)
    gen_nameplate()
    gen_underline()
    gen_letterbox()
    gen_status_frame("Tex_StatusFrame_Buff", GOLD)
    gen_status_frame("Tex_StatusFrame_Debuff", CRIMSON_L)
    gen_status_glyph("Tex_Status_AttackUp", GOLD_L, "atk")
    gen_status_glyph("Tex_Status_DefenseDown", CRIMSON_L, "def")
    gen_status_glyph("Tex_Status_Bleed", (200, 50, 50, 255), "bleed")
    gen_portrait_frame()
    gen_enrage_frame()
    gen_button("Tex_Button_Normal", (28, 33, 45, 255), GOLD, (44, 51, 68, 255))
    gen_button("Tex_Button_Hover", (46, 54, 72, 255), GOLD_L, (66, 76, 100, 255))
    gen_button("Tex_Button_Pressed", (18, 21, 30, 255), GOLD, (28, 33, 45, 255))
    gen_panel()
    gen_result_banner("Tex_Result_Banner_Victory", "VICTORY", GOLD, GOLD_L)
    gen_result_banner("Tex_Result_Banner_Defeat", "DEFEAT", CRIMSON, CRIMSON_L)
    gen_groggy_glow()
    gen_boss_silhouette()
    gen_damage_impact()
    print("DONE: all UI textures generated to", OUT)


if __name__ == "__main__":
    main()
