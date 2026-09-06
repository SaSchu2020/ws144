# Design: Clear Rectangular Region

## Overview

Add a `clearRectangle` function to clear a rectangular region of the 128x128 LCD display, complementing the existing full-screen `clear` function.

## New JavaScript API

```javascript
Display.clearRectangle(x1, y1, x2, y2)
Display.clearRectangle(x1, y1, x2, y2, color)
```

### Parameters

- `x1`, `y1`: First corner coordinates (any corner — top-left or bottom-right works)
- `x2`, `y2`: Second corner coordinates (opposite corner to x1,y1)
- `color` (optional): 16-bit RGB565 color, defaults to `0x0000` (black)

### Behavior

1. **Coordinate swapping**: If `x1 > x2` or `y1 > y2`, coordinates are swapped automatically so the rectangle is always valid
2. **Boundary clamping**: If the rectangle extends beyond display bounds (0-127), the region is silently clamped to valid display coordinates
3. **Console warning**: If clamping occurs, a warning is printed to console

## Implementation

### Files to modify

1. **`src/display.h`** — Add `clearRectangle` declaration
2. **`src/display.cc`** — Add `clearRectangle` implementation that delegates to `Paint_ClearWindow`
3. **`src/addon.cc`** — Export `clearRectangle` to JavaScript

### No changes needed

- `src/GUI/GUI_Paint.cc` — `Paint_ClearWindow` already exists and handles all the clearing logic

### Implementation details

The `Paint_ClearWindow` function signature:
```cpp
void Paint_ClearWindow(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
```

The `clearRectangle` implementation will:
1. Normalize coordinates (swap if inverted)
2. Clamp to display bounds (0-127 for both axes)
3. Call `Paint_ClearWindow` with the normalized coordinates
4. Call `LCD_1in44_Display(displayBuffer)` to flush to display

### Error handling

- Out-of-order coordinates: swap automatically (no error)
- Out of bounds: clamp silently with console warning
- Invalid color: use default black

## Testing

Test cases:
1. `clearRectangle(10, 10, 50, 50)` — normal rect
2. `clearRectangle(50, 50, 10, 10)` — inverted coordinates (should auto-swap)
3. `clearRectangle(0, 0, 127, 127)` — full screen (same as `clear()`)
4. `clearRectangle(-10, 0, 20, 50)` — partially out of bounds (should clamp)
5. `clearRectangle(100, 100, 150, 150)` — fully out of bounds (should clamp to visible area)
6. `clearRectangle(10, 10, 50, 50, 0xF800)` — with red color

## Related existing functions

- `clear(color?)` — clears entire screen (display.cc:125)
- `Paint_ClearWindow` — underlying GUI function (GUI_Paint.cc:279)