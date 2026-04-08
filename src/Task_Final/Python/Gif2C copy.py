import sys
from PIL import Image, ImageDraw, ImageFont

def find_font(size=24):
    """尝试查找可用的TrueType字体，若失败则使用默认字体（可能很小）"""
    font_paths = [
        # Windows
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        # Linux
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        # macOS
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
    ]
    for path in font_paths:
        try:
            return ImageFont.truetype(path, size)
        except IOError:
            continue
    # 如果都不存在，使用默认字体（无法调节大小，通常很小）
    print("警告：未找到合适的TrueType字体，将使用默认小字体，文字可能不清晰。")
    return ImageFont.load_default()

def create_frame(draw, text, position, font, bg=(0,0,0), fg=(255,255,255)):
    """在画布上绘制文字，返回新的Image对象（不修改原画布）"""
    img = Image.new("RGB", (128, 64), bg)
    draw_obj = ImageDraw.Draw(img)
    draw_obj.text(position, text, fill=fg, font=font)
    return img

def main():
    width, height = 128, 64
    bg_color = (0, 0, 0)      # 黑色背景
    text_color = (255, 255, 255)  # 白色文字
    font_size = 28             # 字体大小，适合128x64显示单个字符
    
    font = find_font(font_size)
    # 获取字符的尺寸（近似，用于居中和定位）
    try:
        # 尝试获取文本边界框
        bbox = font.getbbox("E")  # 返回 (left, top, right, bottom)
        char_width = bbox[2] - bbox[0]
        char_height = bbox[3] - bbox[1]
    except AttributeError:
        # 旧版Pillow使用 getsize
        char_width, char_height = font.getsize("E")
    
    # 垂直居中位置
    y_center = (height) // 2 - char_height
    
    frames = []
    durations = []  # 每帧持续时间（毫秒）
    
    # 1. E 从中间向左移动
    start_x = (width - char_width) // 2   # 初始居中位置
    end_x = 10                            # 最终左侧位置
    step = 3                              # 每次移动像素
    current_x = start_x
    while current_x > end_x:
        img = Image.new("RGB", (width, height), bg_color)
        draw = ImageDraw.Draw(img)
        draw.text((current_x, y_center), "E", fill=text_color, font=font)
        frames.append(img)
        durations.append(100)  # 每帧100ms
        current_x -= step
        if current_x < end_x:
            current_x = end_x
    
    # 确保最后一帧停在 end_x
    img = Image.new("RGB", (width, height), bg_color)
    draw = ImageDraw.Draw(img)
    draw.text((end_x, y_center), "E", fill=text_color, font=font)
    frames.append(img)
    durations.append(100)
    
    # 2. 依次添加 S, P, 3, 2
    extra_chars = ["S", "P", "3", "2"]
    # 字符之间的水平间距
    spacing = 5
    # 当前已经显示的字符位置（x坐标列表）
    positions = [end_x]
    current_x_offset = end_x + char_width + spacing
    
    for ch in extra_chars:
        # 新字符出现的位置
        new_x = current_x_offset
        # 创建一帧：显示已有的所有字符 + 新字符
        img = Image.new("RGB", (width, height), bg_color)
        draw = ImageDraw.Draw(img)
        # 绘制已有的所有字符
        for i, c in enumerate(["E"] + extra_chars[:extra_chars.index(ch)]):
            x = positions[i] if i < len(positions) else None
            if x is not None:
                draw.text((x, y_center), c, fill=text_color, font=font)
        # 绘制新字符
        draw.text((new_x, y_center), ch, fill=text_color, font=font)
        frames.append(img)
        durations.append(200)  # 每个新字符出现停留稍长
        positions.append(new_x)
        current_x_offset += char_width + spacing
    
    # 3. 最终完整画面停留几帧
    final_img = Image.new("RGB", (width, height), bg_color)
    draw = ImageDraw.Draw(final_img)
    draw.text((end_x, y_center), "E", fill=text_color, font=font)
    x_offset = end_x + char_width + spacing
    for ch in extra_chars:
        draw.text((x_offset, y_center), ch, fill=text_color, font=font)
        x_offset += char_width + spacing
    for _ in range(5):  # 停留5帧，约0.5秒
        frames.append(final_img.copy())
        durations.append(100)
    
    # 保存为GIF
    output_file = "esp32_animation.gif"
    frames[0].save(
        output_file,
        save_all=True,
        append_images=frames[1:],
        duration=durations,
        loop=0,
        optimize=False
    )
    print(f"GIF已生成：{output_file}，共{len(frames)}帧，尺寸128x64。")

if __name__ == "__main__":
    main()