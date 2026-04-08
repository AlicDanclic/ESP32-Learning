#!/usr/bin/env python3
"""
GIF to C Header - 高性能命令行版
依赖: pip install pillow numpy
用法: python gif2c.py input.gif [--prefix MY_GIF] [--format mono1] [--threshold 128]
"""

import argparse
import numpy as np
from PIL import Image, ImageSequence
import os
import sys

def process_gif(input_path, prefix, fmt, threshold, bg_color):
    print(f"加载GIF: {input_path}")
    img = Image.open(input_path)
    width, height = img.size
    frames = []
    delays = []
    for frame in ImageSequence.Iterator(img):
        frames.append(frame.convert("RGBA"))
        delays.append(frame.info.get('duration', 100))
    frame_count = len(frames)
    print(f"尺寸: {width}x{height}, 帧数: {frame_count}")

    # 背景色 (RGB)
    if bg_color == 'black':
        bg_rgb = (0,0,0)
    elif bg_color == 'white':
        bg_rgb = (255,255,255)
    else:
        bg_rgb = (0,0,0)

    all_data = []
    for idx, frame in enumerate(frames):
        # 合成背景 (numpy加速)
        arr = np.array(frame, dtype=np.uint8)  # (H,W,4)
        bg_arr = np.full((height, width, 3), bg_rgb, dtype=np.uint8)
        alpha = arr[:,:,3:4] / 255.0
        blended = (arr[:,:,:3] * alpha + bg_arr * (1 - alpha)).astype(np.uint8)

        if fmt == 'mono1':
            # 灰度化 + 阈值打包
            gray = np.dot(blended[...,:3], [0.299, 0.587, 0.114])
            bits = (gray > threshold).astype(np.uint8)
            # 打包为每8像素一字节 (MSB左)
            byte_width = (width + 7) // 8
            packed = np.packbits(bits, axis=1, bitorder='big')
            all_data.append(packed.tobytes())
        else:  # rgb565
            r5 = (blended[:,:,0] >> 3) & 0x1F
            g6 = (blended[:,:,1] >> 2) & 0x3F
            b5 = (blended[:,:,2] >> 3) & 0x1F
            rgb565 = (r5 << 11) | (g6 << 5) | b5
            # 转为大端字节数组
            bytes_arr = np.empty((height, width, 2), dtype=np.uint8)
            bytes_arr[:,:,0] = (rgb565 >> 8) & 0xFF
            bytes_arr[:,:,1] = rgb565 & 0xFF
            all_data.append(bytes_arr.tobytes())

        if (idx+1) % 20 == 0:
            print(f"已处理 {idx+1}/{frame_count} 帧")

    # 生成C头文件
    prefix = prefix.upper()
    offsets = []
    cur = 0
    for data in all_data:
        offsets.append(cur)
        cur += len(data)
    total_size = cur

    header = f"""/**
 * @file {prefix.lower()}_gif.h
 * @brief GIF animation data
 * Size: {width}x{height}, Frames: {frame_count}
 */
#ifndef {prefix}_GIF_H
#define {prefix}_GIF_H
#include <stdint.h>
#define {prefix}_WIDTH       {width}
#define {prefix}_HEIGHT      {height}
#define {prefix}_FRAME_COUNT {frame_count}
const uint16_t {prefix}_DELAY_MS[] = {{ {', '.join(str(d) for d in delays)} }};
const uint32_t {prefix}_FRAME_OFFSET[] = {{ {', '.join(str(o) for o in offsets)} }};
const uint8_t {prefix}_FRAMES_DATA[{total_size}] = {{"""
    # 写入字节数据
    line = []
    for data in all_data:
        for b in data:
            line.append(f"0x{b:02X}")
            if len(line) >= 16:
                header += "\n  " + ", ".join(line) + ","
                line = []
    if line:
        header += "\n  " + ", ".join(line)
    header += f"\n}};\n"
    if fmt == 'mono1':
        header += f"#define {prefix}_FRAME_BYTES (({prefix}_WIDTH * {prefix}_HEIGHT + 7) / 8)\n"
    else:
        header += f"#define {prefix}_FRAME_BYTES ({prefix}_WIDTH * {prefix}_HEIGHT * 2)\n"
    header += f"#endif /* {prefix}_GIF_H */\n"

    out_path = f"{prefix.lower()}_gif.h"
    with open(out_path, 'w') as f:
        f.write(header)
    print(f"✅ 生成成功: {out_path} (总数据 {total_size} 字节)")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="GIF to C Header Converter (High Speed)")
    parser.add_argument("input", help="输入GIF文件")
    parser.add_argument("--prefix", default="MY_GIF", help="宏前缀 (默认 MY_GIF)")
    parser.add_argument("--format", choices=["mono1","rgb565"], default="mono1", help="输出格式")
    parser.add_argument("--threshold", type=int, default=128, help="单色阈值 0-255")
    parser.add_argument("--bg", choices=["black","white"], default="black", help="背景色")
    args = parser.parse_args()
    process_gif(args.input, args.prefix, args.format, args.threshold, args.bg)