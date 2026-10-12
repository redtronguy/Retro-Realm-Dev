"""Generate a simple cover image for The Tidebound Count."""
import math, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter

W, H = 1600, 2400
out = sys.argv[1]
img = Image.new("RGB", (W, H))
d = ImageDraw.Draw(img)

# Night sky to deep sea gradient.
for y in range(H):
    t = y / H
    if t < 0.55:
        k = t / 0.55
        c = (int(10 + 20 * k), int(14 + 30 * k), int(40 + 50 * k))
    else:
        k = (t - 0.55) / 0.45
        c = (int(8 + 4 * (1 - k)), int(40 + 20 * (1 - k)), int(70 + 40 * (1 - k)))
    d.line([(0, y), (W, y)], fill=c)

# Stars.
import random
random.seed(7)
for _ in range(260):
    x, y = random.randrange(W), random.randrange(int(H * 0.5))
    r = random.choice([1, 1, 1, 2, 2, 3])
    b = random.randrange(150, 255)
    d.ellipse([x - r, y - r, x + r, y + r], fill=(b, b, min(255, b + 20)))

# The Teeth: a black rock with a tower and a red light.
rock = [(520, 1320), (640, 1010), (720, 1080), (800, 900), (880, 1060), (980, 980), (1100, 1320)]
d.polygon(rock, fill=(6, 8, 14))
d.rectangle([770, 760, 830, 920], fill=(6, 8, 14))
d.polygon([(760, 760), (800, 700), (840, 760)], fill=(6, 8, 14))
glow = Image.new("RGB", (W, H), (0, 0, 0))
gd = ImageDraw.Draw(glow)
gd.ellipse([770, 700, 830, 760], fill=(255, 60, 40))
glow = glow.filter(ImageFilter.GaussianBlur(25))
img = Image.composite(Image.blend(img, glow, 0.6), img, glow.convert("L").point(lambda v: 255 if v > 8 else 0))
d = ImageDraw.Draw(img)
d.ellipse([790, 720, 810, 740], fill=(255, 120, 90))

# Sea surface waves.
for i in range(40):
    y = 1320 + i * 28
    amp = 6 + i * 0.6
    pts = [(x, y + amp * math.sin(x / (60 + i * 3) + i)) for x in range(0, W + 20, 20)]
    shade = 70 + i * 3
    d.line(pts, fill=(30, shade, min(255, shade + 70)), width=3)

# Seven gold chains going down into the deep, fading.
for k in range(7):
    x0 = 700 + k * 33
    for y in range(1320, H, 6):
        a = 1 - (y - 1320) / (H - 1320)
        col = (int(255 * a + 20 * (1 - a)), int(200 * a + 50 * (1 - a)), int(80 * a + 90 * (1 - a)))
        d.line([(x0 + (y - 1320) * (k - 3) * 0.08, y), (x0 + (y - 1320) * (k - 3) * 0.08, y + 4)], fill=col, width=4)

# A curling-wave sigil: concentric arcs.
cx, cy = 800, 2060
for r, wdt in [(150, 10), (110, 8), (70, 6)]:
    d.arc([cx - r, cy - r, cx + r, cy + r], 200, 520, fill=(115, 239, 247), width=wdt)

serif = "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf"
serif_r = "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf"
def center(text, y, size, font, fill):
    f = ImageFont.truetype(font, size)
    w = d.textlength(text, font=f)
    d.text(((W - w) / 2 + 4, y + 4), text, font=f, fill=(0, 0, 0))
    d.text(((W - w) / 2, y), text, font=f, fill=fill)

center("THE", 180, 90, serif, (255, 205, 117))
center("TIDEBOUND", 290, 190, serif, (255, 205, 117))
center("COUNT", 500, 190, serif, (255, 205, 117))
sub = sys.argv[2] if len(sys.argv) > 2 else "A Novel"
center(sub, 1450, 70 if sub == "A Novel" else 90, serif if sub != "A Novel" else serif_r, (220, 230, 245) if sub == "A Novel" else (115, 239, 247))
center("Wait, and hope.", 2260, 60, serif_r, (180, 210, 235))
img.save(out, quality=92)
print("cover:", out)
