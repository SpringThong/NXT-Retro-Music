# v1.9.7.7 UI assets

Run `python3 tools/refine-ui-icons.py` to regenerate 316 white icons from the pinned lucide-static 0.468.0 archive and original avatar SVGs. The generator checks the archive SHA256, rasterizes at 384 pixels with librsvg/cairo, and writes antialiased 192-pixel PNGs using Pillow Lanczos. It does not change coloured provider logos or 3D emoji. Runtime creates and caches only the actual on-screen glyph size; no high-resolution full-screen render target is added.

The runtime applies alpha-correct area sampling at cache creation, with bilinear enlargement. Rounded masks use fractional pixel coverage, opaque interiors keep SDL fills, and animated diagonal lines visit only a narrow band. PNG covers are always limited by the source image detail and the physical display resolution.
