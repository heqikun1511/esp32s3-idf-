#!/usr/bin/env python3
"""Transform screens.c - multiply all position/size values by 2.25 for 1920x1080 display"""

import re

SCALE = 2.25

with open('main/ui/screens.c', 'r') as f:
    content = f.read()

def scale_match(m):
    """Scale numeric values in a function call like (obj, X, Y) or (obj, W, H)"""
    full = m.group(0)
    def repl_num(n):
        v = n.group(0)
        try:
            val = int(v)
            scaled = round(val * SCALE)
            return str(scaled)
        except:
            return v
    
    # Match the arguments part: "(obj, X, Y)" or "(obj, W, H)"
    args_match = re.match(r'(lv_obj_set_pos|lv_obj_set_size|lv_obj_set_style_width|lv_obj_set_style_height)\(([^,]+),\s*(\d+),\s*(\d+)', full)
    if args_match:
        func = args_match.group(1)
        obj = args_match.group(2)
        val1 = int(args_match.group(3))
        val2 = int(args_match.group(4))
        scaled1 = round(val1 * SCALE)
        scaled2 = round(val2 * SCALE)
        return f"{func}({obj}, {scaled1}, {scaled2}"
    
    return full

# Transform lv_obj_set_pos(obj, X, Y)
content = re.sub(
    r'lv_obj_set_pos\(([^,]+),\s*(\d+),\s*(\d+)\)',
    lambda m: f'lv_obj_set_pos({m.group(1)}, {round(int(m.group(2))*SCALE)}, {round(int(m.group(3))*SCALE)})',
    content
)

# Transform lv_obj_set_size(obj, W, H) - only pure numbers, not LV_SIZE_CONTENT or LV_PCT
def transform_size(m):
    obj = m.group(1)
    w = m.group(2)
    h = m.group(3)
    try:
        w_val = int(w)
        w_new = str(round(w_val * SCALE))
    except:
        w_new = w
    try:
        h_val = int(h)
        h_new = str(round(h_val * SCALE))
    except:
        h_new = h
    if w_new != w or h_new != h:
        return f'lv_obj_set_size({obj}, {w_new}, {h_new})'
    return m.group(0)

# Find set_size calls where both args are pure numbers
content = re.sub(
    r'lv_obj_set_size\(([^,]+),\s*(\d+),\s*(\d+)\)',
    transform_size,
    content
)

# Transform lv_arc_create and lv_meter_create sizes (already handled above, skip duplicate)


# Transform hardcoded sizes in styles
# lv_obj_set_style_width(obj, N, ...) and lv_obj_set_style_height(obj, N, ...)
content = re.sub(
    r'(lv_obj_set_style_width|lv_obj_set_style_height)\(([^,]+),\s*(\d+),',
    lambda m: f'{m.group(1)}({m.group(2)}, {round(int(m.group(3))*SCALE)},',
    content
)

with open('main/ui/screens.c', 'w') as f:
    f.write(content)

print("Transformation complete!")
