#!/usr/bin/env python3
"""
Convert raw ARGB8888 C arrays from EEZ Studio output to JPEG binary data,
ready to be embedded in firmware and decoded to PSRAM at boot.

Usage:
    python3 convert_images_to_jpeg.py

Output:
    main/ui/jpeg/<name>_jpg.c      - JPEG binary data as C array
    main/ui/jpeg/<name>_jpg.h      - Header with extern declaration
    main/ui/jpeg/jpeg_images.h     - Master header for all JPEG images
"""

import os
import re
import struct
import sys
from io import BytesIO

try:
    from PIL import Image
except ImportError:
    print("Please install Pillow: pip install Pillow")
    sys.exit(1)

UI_DIR = "main/ui"
OUT_DIR = os.path.join(UI_DIR, "jpeg")

# Image info: (filename, width, height, stride, color_format)
# We'll auto-detect from the C files
IMAGE_INFO = {}  # filled by scan_c_files()

JPEG_QUALITY = 85  # Adjust for size/quality tradeoff


def scan_c_files():
    """Scan ui_image_*.c files to extract dimensions and data."""
    images = []
    for fname in os.listdir(UI_DIR):
        if not fname.startswith("ui_image_") or not fname.endswith(".c"):
            continue
        fpath = os.path.join(UI_DIR, fname)
        with open(fpath, "r") as f:
            content = f.read()

        # Extract image name from filename: ui_image_<name>.c
        match = re.match(r"ui_image_(.+)\.c", fname)
        if not match:
            continue
        name = match.group(1)

        # Extract dimensions from lv_image_dsc_t initializer
        w_match = re.search(r"\.w\s*=\s*(\d+)", content)
        h_match = re.search(r"\.h\s*=\s*(\d+)", content)
        stride_match = re.search(r"\.stride\s*=\s*(\d+)", content)
        cf_match = re.search(r"\.cf\s*=\s*LV_COLOR_FORMAT_(\w+)", content)

        if not (w_match and h_match and stride_match):
            print(f"  Warning: Could not parse dimensions from {fname}, skipping")
            continue

        w = int(w_match.group(1))
        h = int(h_match.group(1))
        stride = int(stride_match.group(1))
        cf = cf_match.group(1) if cf_match else "ARGB8888"

        # Extract the pixel data array
        # Find the array definition
        arr_match = re.search(
            r"uint8_t\s+(\w+)_map\[\]\s*=\s*\{(.*?)\};",
            content,
            re.DOTALL,
        )
        if not arr_match:
            print(f"  Warning: Could not find pixel data array in {fname}, skipping")
            continue

        array_name = arr_match.group(1)
        data_str = arr_match.group(2)

        # Parse hex bytes
        hex_values = re.findall(r"0x([0-9a-fA-F]{2})", data_str)
        pixel_data = bytes(int(v, 16) for v in hex_values)

        expected_size = stride * h
        if len(pixel_data) < expected_size:
            print(
                f"  Warning: {fname}: expected {expected_size} bytes, got {len(pixel_data)}"
            )
            # Try to continue with what we have
            if len(pixel_data) < stride * (h - 1):
                print(f"  Error: Too little data, skipping")
                continue
            actual_h = len(pixel_data) // stride
            print(f"  Note: Using {actual_h} rows instead of {h}")
            h = actual_h

        images.append(
            {
                "name": name,
                "w": w,
                "h": h,
                "stride": stride,
                "cf": cf,
                "data": pixel_data[: stride * h],
            }
        )

    return images


def argb8888_to_rgb888(argb_data, w, h, stride):
    """Convert ARGB8888 bytes to flat RGB888 bytes."""
    rgb = bytearray(w * h * 3)
    for y in range(h):
        row_start = y * stride
        for x in range(w):
            idx = row_start + x * 4
            if idx + 3 >= len(argb_data):
                break
            a = argb_data[idx]
            r = argb_data[idx + 1]
            g = argb_data[idx + 2]
            b = argb_data[idx + 3]
            # If pixel is semi-transparent, blend with white background
            if a < 128:
                # Premultiply alpha with white background
                alpha = a / 255.0
                r = int(r * alpha + 255 * (1 - alpha))
                g = int(g * alpha + 255 * (1 - alpha))
                b = int(b * alpha + 255 * (1 - alpha))
            rgb[(y * w + x) * 3] = r
            rgb[(y * w + x) * 3 + 1] = g
            rgb[(y * w + x) * 3 + 2] = b
    return bytes(rgb)


def convert_image(img):
    """Convert a raw ARGB8888 image to JPEG C array."""
    name = img["name"]
    w = img["w"]
    h = img["h"]
    stride = img["stride"]
    cf = img["cf"]
    pixel_data = img["data"]

    print(f"\nConverting '{name}': {w}x{h}, stride={stride}, format={cf}")

    # Convert ARGB8888 to RGB888
    rgb_data = argb8888_to_rgb888(pixel_data, w, h, stride)

    # Create PIL Image
    pil_img = Image.frombytes("RGB", (w, h), rgb_data)

    # Encode as JPEG
    buf = BytesIO()
    pil_img.save(buf, format="JPEG", quality=JPEG_QUALITY, optimize=True)
    jpeg_bytes = buf.getvalue()

    compression_ratio = len(pixel_data) / len(jpeg_bytes)
    print(
        f"  Original: {len(pixel_data)} bytes → JPEG: {len(jpeg_bytes)} bytes"
    )
    print(f"  Compression ratio: {compression_ratio:.1f}x")

    return name, jpeg_bytes, w, h


def write_c_files(images_jpeg):
    """Write JPEG data as C arrays."""
    os.makedirs(OUT_DIR, exist_ok=True)

    master_include_lines = []
    master_entries = []
    total_original = 0
    total_compressed = 0

    for name, jpeg_bytes, w, h in images_jpeg:
        total_original += 0  # we'll compute later
        total_compressed += len(jpeg_bytes)

        var_name = f"jpg_{name}"
        guard = f"JPEG_{name.upper()}_H"

        # Create header
        hdr = f"""// Auto-generated JPEG data for {name}
#ifndef {guard}
#define {guard}

#ifdef __cplusplus
extern "C" {{
#endif

#include <stdint.h>
#include <stddef.h>

extern const uint8_t {var_name}_data[];
extern const size_t {var_name}_size;
extern const int {var_name}_w;
extern const int {var_name}_h;

#ifdef __cplusplus
}}
#endif

#endif /* {guard} */
"""
        hdr_path = os.path.join(OUT_DIR, f"{name}_jpg.h")
        with open(hdr_path, "w") as f:
            f.write(hdr)

        # Create C file with JPEG binary data
        c_lines = [
            f"// Auto-generated JPEG data for {name}",
            f"// Original: {w}x{h} ARGB8888",
            f"// JPEG size: {len(jpeg_bytes)} bytes",
            f'#include "{name}_jpg.h"',
            "",
            f"const int {var_name}_w = {w};",
            f"const int {var_name}_h = {h};",
            f"const size_t {var_name}_size = {len(jpeg_bytes)};",
            "",
            f"const uint8_t {var_name}_data[] = {{",
        ]

        # Write JPEG bytes as C array, 16 bytes per line
        line = "    "
        for i, b in enumerate(jpeg_bytes):
            line += f"0x{b:02x},"
            if (i + 1) % 16 == 0:
                c_lines.append(line)
                line = "    "
        if line.strip() != "    ":
            c_lines.append(line)
        c_lines.append("};")

        c_path = os.path.join(OUT_DIR, f"{name}_jpg.c")
        with open(c_path, "w") as f:
            f.write("\n".join(c_lines) + "\n")

        master_include_lines.append(f'#include "{name}_jpg.h"')
        master_entries.append(
            f'    {{ "{name}", jpg_{name}_data, jpg_{name}_size, jpg_{name}_w, jpg_{name}_h }}'
        )

        print(f"  Written: {c_path}")

    # Write master header
    master_hdr = f"""// Auto-generated master header for all JPEG images
#ifndef JPEG_IMAGES_H
#define JPEG_IMAGES_H

#ifdef __cplusplus
extern "C" {{
#endif

#include <stdint.h>
#include <stddef.h>

{chr(10).join(master_include_lines)}

typedef struct {{
    const char *name;
    const uint8_t *data;
    size_t size;
    int w;
    int h;
}} jpeg_image_t;

extern const jpeg_image_t jpeg_images[];
extern const int jpeg_image_count;

#ifdef __cplusplus
}}
#endif

#endif /* JPEG_IMAGES_H */
"""
    with open(os.path.join(OUT_DIR, "jpeg_images.h"), "w") as f:
        f.write(master_hdr)

    # Write master C file
    master_c = f"""// Auto-generated JPEG image table
#include "jpeg_images.h"

const jpeg_image_t jpeg_images[] = {{
{chr(10).join(master_entries)}
}};

const int jpeg_image_count = sizeof(jpeg_images) / sizeof(jpeg_images[0]);
"""
    with open(os.path.join(OUT_DIR, "jpeg_images.c"), "w") as f:
        f.write(master_c)

    print(f"\n=== Summary ===")
    print(f"Written to: {OUT_DIR}/")
    print(f"Total JPEG size: {total_compressed} bytes ({total_compressed/1024:.1f} KB)")
    print(
        f"Estimated compression from original: ~{total_compressed/1024/1024:.1f} MB in flash vs ~34.5 MB raw"
    )


def main():
    print("Scanning UI image files...")
    images = scan_c_files()
    if not images:
        print("No image files found!")
        sys.exit(1)

    print(f"Found {len(images)} images:")
    for img in images:
        stride = img["stride"]
        h = img["h"]
        data_size = stride * h
        print(f"  {img['name']}: {img['w']}x{img['h']}, stride={stride}, ~{data_size/1024:.0f} KB")

    print(f"\nConverting to JPEG (quality={JPEG_QUALITY})...")
    results = []
    for img in images:
        result = convert_image(img)
        results.append(result)

    write_c_files(results)


if __name__ == "__main__":
    main()
