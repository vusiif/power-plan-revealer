import math
from PIL import Image, ImageDraw, ImageFilter

def create_power_plan_icon():
    # Supersampled 1024x1024
    SIZE = 1024
    img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))

    # 1. Base Squircle (Windows 11 Fluent shape)
    margin = 52
    box = [margin, margin, SIZE - margin, SIZE - margin]
    radius = 210

    # Gradient for base
    grad = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    grad_draw = ImageDraw.Draw(grad)
    for y in range(margin, SIZE - margin):
        t = (y - margin) / float(SIZE - 2 * margin)
        # Deep Obsidian Slate Gradient: #131d31 to #090e18
        r = int(19 + (9 - 19) * t)
        g = int(29 + (14 - 29) * t)
        b = int(49 + (24 - 49) * t)
        grad_draw.line([(margin, y), (SIZE - margin, y)], fill=(r, g, b, 255))

    # Mask for squircle
    sq_mask = Image.new("L", (SIZE, SIZE), 0)
    sq_mask_draw = ImageDraw.Draw(sq_mask)
    sq_mask_draw.rounded_rectangle(box, radius=radius, fill=255)
    img.paste(grad, (0, 0), sq_mask)

    # Outer border / subtle neon glow
    border_draw = ImageDraw.Draw(img)
    border_draw.rounded_rectangle(box, radius=radius, outline=(56, 189, 248, 70), width=6)
    border_draw.rounded_rectangle([margin + 6, margin + 6, SIZE - margin - 6, SIZE - margin - 6], radius=radius - 6, outline=(255, 255, 255, 20), width=2)

    # 2. Silky-Smooth Power Arc
    cx, cy = SIZE // 2, SIZE // 2 + 15
    R_outer = 320
    R_inner = 265
    arc_width = R_outer - R_inner
    mid_r = (R_outer + R_inner) / 2.0

    start_deg = 138
    end_deg = 402

    # Arc mask using smooth pieslices
    arc_mask = Image.new("L", (SIZE, SIZE), 0)
    arc_m_draw = ImageDraw.Draw(arc_mask)
    arc_m_draw.pieslice([cx - R_outer, cy - R_outer, cx + R_outer, cy + R_outer], start=start_deg, end=end_deg, fill=255)
    arc_m_draw.pieslice([cx - R_inner, cy - R_inner, cx + R_inner, cy + R_inner], start=start_deg - 2, end=end_deg + 2, fill=0)

    # Rounded end caps
    cap_radius = arc_width / 2.0
    for deg in [start_deg, end_deg]:
        rad = math.radians(deg)
        cap_x = cx + mid_r * math.cos(rad)
        cap_y = cy + mid_r * math.sin(rad)
        arc_m_draw.ellipse(
            [cap_x - cap_radius, cap_y - cap_radius, cap_x + cap_radius, cap_y + cap_radius],
            fill=255
        )

    # Arc glow layer
    arc_glow = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    glow_fill = Image.new("RGBA", (SIZE, SIZE), (37, 99, 235, 180))
    arc_glow.paste(glow_fill, (0, 0), arc_mask)
    arc_glow = arc_glow.filter(ImageFilter.GaussianBlur(22))
    img.alpha_composite(arc_glow)

    # Arc gradient fill (Cyan to Electric Blue)
    arc_grad = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    arc_g_draw = ImageDraw.Draw(arc_grad)
    for y in range(cy - R_outer - 10, cy + R_outer + 10):
        t = max(0.0, min(1.0, (y - (cy - R_outer)) / (2.0 * R_outer)))
        # From Cyan (56, 189, 248) to Royal Electric Blue (37, 99, 235)
        r = int(56 + (37 - 56) * t)
        g = int(189 + (99 - 189) * t)
        b = int(248 + (235 - 248) * t)
        arc_g_draw.line([(0, y), (SIZE, y)], fill=(r, g, b, 255))

    img.paste(arc_grad, (0, 0), arc_mask)

    # 3. Dynamic Lightning Energy Bolt (Centerpiece)
    # Balanced, sleek lightning bolt
    bolt_pts = [
        (cx + 45, cy - 315),   # Top point
        (cx - 105, cy - 25),   # Inner bend left
        (cx + 15, cy - 25),    # Waist right
        (cx - 55, cy + 305),   # Bottom sharp point
        (cx + 105, cy + 15),   # Inner bend right
        (cx - 15, cy + 15),    # Waist left
    ]

    # Bolt glow
    bolt_glow = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    bglow_draw = ImageDraw.Draw(bolt_glow)
    bglow_draw.polygon(bolt_pts, fill=(56, 189, 248, 190))
    bolt_glow = bolt_glow.filter(ImageFilter.GaussianBlur(26))
    img.alpha_composite(bolt_glow)

    # Bolt fill with brilliant electric gradient
    bolt_mask = Image.new("L", (SIZE, SIZE), 0)
    bmask_draw = ImageDraw.Draw(bolt_mask)
    bmask_draw.polygon(bolt_pts, fill=255)

    bolt_grad = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    bgrad_draw = ImageDraw.Draw(bolt_grad)
    for y in range(cy - 330, cy + 320):
        t = max(0.0, min(1.0, (y - (cy - 325)) / 640.0))
        # Top: Pure Energy White/Cyan (240, 253, 255) -> Mid: Cyan (56, 189, 248) -> Bottom: Blue (37, 99, 235)
        if t < 0.35:
            st = t / 0.35
            r = int(245 + (56 - 245) * st)
            g = int(253 + (189 - 253) * st)
            b = int(255 + (248 - 255) * st)
        else:
            st = (t - 0.35) / 0.65
            r = int(56 + (37 - 56) * st)
            g = int(189 + (99 - 189) * st)
            b = int(248 + (235 - 248) * st)
        bgrad_draw.line([(0, y), (SIZE, y)], fill=(r, g, b, 255))

    img.paste(bolt_grad, (0, 0), bolt_mask)

    # Subtle inner white core highlight
    core_pts = [
        (cx + 35, cy - 280),
        (cx - 70, cy - 30),
        (cx + 10, cy - 30),
        (cx - 35, cy + 240),
        (cx + 70, cy + 20),
        (cx - 10, cy + 20),
    ]
    core_mask = Image.new("L", (SIZE, SIZE), 0)
    cmask_draw = ImageDraw.Draw(core_mask)
    cmask_draw.polygon(core_pts, fill=160)
    core_mask = core_mask.filter(ImageFilter.GaussianBlur(6))
    core_fill = Image.new("RGBA", (SIZE, SIZE), (255, 255, 255, 240))
    img.paste(core_fill, (0, 0), core_mask)

    return img

if __name__ == "__main__":
    icon_image = create_power_plan_icon()

    png_path = "src/gui/resources/app.png"
    icon_image.resize((512, 512), Image.Resampling.LANCZOS).save(png_path, "PNG")
    print(f"Saved PNG to {png_path}")

    ico_path = "src/gui/resources/app.ico"
    icon_sizes = [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    icon_image.save(ico_path, format="ICO", sizes=icon_sizes)
    print(f"Saved multi-res ICO to {ico_path}")
