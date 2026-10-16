# -*- coding: UTF-8 -*-
# Copyright (C) 2026 Quant1X <wangfengxy@sina.cn>
# SPDX-License-Identifier: AGPL-3.0-only
"""由根目录 logo.png 生成标准 app 图标与网站图标.

用法::

    python scripts/gen_icons.py

输入: 仓库根目录 logo.png (任意尺寸, 建议透明底 RGBA)
输出: assets/icons/ 下的 ios / android / macos / windows / web 五组
      以及 1024x1024 方形母版 assets/icons/icon-1024.png

仅依赖 Pillow: python -m pip install Pillow
"""

from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
LOGO = ROOT / "logo.png"
OUT = ROOT / "assets" / "icons"
MASTER_SIZE = 1024
CONTENT_RATIO = 0.92  # 图形内容在方形画布中的占比, 四周留边
APPLE_BG = (255, 255, 255, 255)  # iOS 不支持透明图标, 垫白底


def build_master(logo: Image.Image) -> Image.Image:
    """把任意尺寸的 logo 等比缩放并居中放到透明方形画布上."""
    side = max(logo.size)
    inner = round(MASTER_SIZE * CONTENT_RATIO)
    scale = inner / side
    scaled = logo.resize(
        (round(logo.width * scale), round(logo.height * scale)), Image.LANCZOS
    )
    master = Image.new("RGBA", (MASTER_SIZE, MASTER_SIZE), (0, 0, 0, 0))
    offset = ((MASTER_SIZE - scaled.width) // 2, (MASTER_SIZE - scaled.height) // 2)
    master.paste(scaled, offset, scaled)
    return master


def save_png(master: Image.Image, path: Path, size: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    master.resize((size, size), Image.LANCZOS).save(path, format="PNG")


def save_apple(master: Image.Image, path: Path, size: int) -> None:
    """iOS 图标: 先垫不透明底色再缩放, 避免系统把透明渲染成黑色."""
    opaque = Image.new("RGBA", master.size, APPLE_BG)
    opaque.alpha_composite(master)
    path.parent.mkdir(parents=True, exist_ok=True)
    opaque.convert("RGB").resize((size, size), Image.LANCZOS).save(path, format="PNG")


def main() -> None:
    logo = Image.open(LOGO).convert("RGBA")
    master = build_master(logo)
    OUT.mkdir(parents=True, exist_ok=True)
    master.save(OUT / "icon-1024.png", format="PNG")

    # ---- iOS (Xcode AppIcon 命名) ----
    ios = OUT / "ios"
    for name, size in [
        ("Icon-App-20x20@1x.png", 20),
        ("Icon-App-20x20@2x.png", 40),
        ("Icon-App-20x20@3x.png", 60),
        ("Icon-App-29x29@1x.png", 29),
        ("Icon-App-29x29@2x.png", 58),
        ("Icon-App-29x29@3x.png", 87),
        ("Icon-App-40x40@2x.png", 80),
        ("Icon-App-40x40@3x.png", 120),
        ("Icon-App-60x60@2x.png", 120),
        ("Icon-App-60x60@3x.png", 180),
        ("Icon-App-76x76@1x.png", 76),
        ("Icon-App-76x76@2x.png", 152),
        ("Icon-App-83.5x83.5@2x.png", 167),
        ("Icon-App-1024x1024.png", 1024),
    ]:
        save_apple(master, ios / name, size)

    # ---- Android (mipmap 密度命名) ----
    android = OUT / "android"
    for dpi, size in [
        ("mipmap-mdpi", 48),
        ("mipmap-hdpi", 72),
        ("mipmap-xhdpi", 96),
        ("mipmap-xxhdpi", 144),
        ("mipmap-xxxhdpi", 192),
    ]:
        save_png(master, android / dpi / "ic_launcher.png", size)
    save_png(master, android / "ic_launcher-playstore.png", 512)

    # ---- macOS (AppIcon.appiconset 命名) ----
    macos = OUT / "macos"
    for name, size in [
        ("icon_16x16.png", 16),
        ("icon_16x16@2x.png", 32),
        ("icon_32x32.png", 32),
        ("icon_32x32@2x.png", 64),
        ("icon_128x128.png", 128),
        ("icon_128x128@2x.png", 256),
        ("icon_256x256.png", 256),
        ("icon_256x256@2x.png", 512),
        ("icon_512x512.png", 512),
        ("icon_512x512@2x.png", 1024),
    ]:
        save_png(master, macos / name, size)

    # ---- Windows (多尺寸 ICO + 备用 PNG) ----
    windows = OUT / "windows"
    windows.mkdir(parents=True, exist_ok=True)
    master.save(
        windows / "app.ico",
        format="ICO",
        sizes=[(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)],
    )
    save_png(master, windows / "app-256.png", 256)

    # ---- Web (favicon 套件 + PWA 清单) ----
    web = OUT / "web"
    save_png(master, web / "favicon-16x16.png", 16)
    save_png(master, web / "favicon-32x32.png", 32)
    save_png(master, web / "favicon-96x96.png", 96)
    save_png(master, web / "android-chrome-192x192.png", 192)
    save_png(master, web / "android-chrome-512x512.png", 512)
    save_png(master, web / "mstile-150x150.png", 150)
    save_apple(master, web / "apple-touch-icon.png", 180)
    web.mkdir(parents=True, exist_ok=True)
    master.save(
        web / "favicon.ico",
        format="ICO",
        sizes=[(16, 16), (32, 32), (48, 48)],
    )
    (web / "site.webmanifest").write_text(
        """{
    "name": "Quant1X",
    "short_name": "Quant1X",
    "icons": [
        {
            "src": "/android-chrome-192x192.png",
            "sizes": "192x192",
            "type": "image/png"
        },
        {
            "src": "/android-chrome-512x512.png",
            "sizes": "512x512",
            "type": "image/png"
        }
    ],
    "theme_color": "#ffffff",
    "background_color": "#ffffff",
    "display": "standalone"
}
""",
        encoding="utf-8",
    )


if __name__ == "__main__":
    main()
