# Drawing Buffer (Deferred Flush) — Design Spec

**Date:** 2026-07-21
**Component:** Display module (`src/display.cc`)

## Problem

When creating animations or movement, the current pattern is:

1. Draw image `I` at point `P1` → flush to LCD
2. Clear the screen to black → flush to LCD
3. Draw image `I` at point `P2` → flush to LCD
4. Repeat

Every drawing function draws into the in-memory `displayBuffer` and then
*immediately* calls `LCD_1in44_Display(displayBuffer)`, pushing the entire
frame to the physical LCD over SPI. A single animation frame therefore
triggers multiple full-frame SPI flushes. Because the display's refresh rate
is limited, these intermediate flushes are visible as latency and flicker.

## Goal

Let callers batch all drawing for one frame and flush to hardware exactly
once, eliminating the intermediate flushes that cause flicker. Provide two new
functions:

- `Display.beginDraw(color?: number)` — start a buffered session, prefilling
  the buffer with `color` (default black).
- `Display.endDraw()` — flush the buffer to the display once and end the
  session.

After `endDraw()`, drawing calls behave exactly as they do today (draw + flush
per call).

## Key Insight

The existing `displayBuffer` already acts as an off-screen buffer: the LCD
keeps its own copy of the frame after each flush, so nothing reads from
`displayBuffer` between flushes. The flicker is caused solely by flushing after
*every* operation — not by the drawing itself.

Therefore no second buffer is needed. We defer the flush: while a buffered
session is active, drawing operations write to `displayBuffer` but skip the
hardware flush. `endDraw()` performs the single flush.

## Approach

### State

A single file-scoped flag in `src/display.cc`:

```cpp
static bool g_buffering = false;
```

### Flush helper

Replaces the direct `LCD_1in44_Display(displayBuffer)` call in every drawing
function:

```cpp
static void PresentIfLive() {
    if (!g_buffering) {
        LCD_1in44_Display(displayBuffer);
    }
}
```

When not buffering (default/live mode) this flushes immediately, preserving
current behavior. When buffering, it does nothing.

### `beginDraw(color?)`

Always returns `true`.

1. Read optional `color`. Default `0x0000`. Apply the same `<< 8` shift used by
   the existing `clear()` function so color semantics are consistent.
2. `Paint_Clear(color)` to prefill the buffer.
3. Set `g_buffering = true`.
4. Do **not** flush.

If `beginDraw` is called while already buffering, it re-prefills the buffer
with the newly passed color and remains in buffering mode. This gives
predictable "start from a fresh color-filled frame" behavior.

### `endDraw()`

Returns `true` if a session was active, `false` otherwise.

1. If `!g_buffering`: no-op, return `false`.
2. Otherwise: set `g_buffering = false`, call
   `LCD_1in44_Display(displayBuffer)` (single flush), return `true`.

## Data Flow

**Live mode (default, unchanged):**

```
draw call → displayBuffer → LCD_1in44_Display (flush)
```

**Buffered mode:**

```
beginDraw(color)  → Paint_Clear(color)          [no flush]
draw call         → displayBuffer               [no flush]
draw call         → displayBuffer               [no flush]
...
endDraw()         → LCD_1in44_Display (flush)    [single flush]
```

## Scope of Buffering

All existing drawing functions respect the buffering flag by using
`PresentIfLive()` instead of a direct flush:

- `writeText`
- `drawBitmap`
- `drawPng`
- `drawBitmapAt`
- `drawPngAt`
- `setPixel`
- `drawImageBuffer`
- `clear`
- `clearRectangle`

Their drawing behavior is otherwise unchanged; only the flush is conditional.
This means `clear()` and `clearRectangle()` also defer their flush during a
buffered session, consistent with "any drawing call goes to the buffer first."

## Error Handling (Tolerant No-Ops)

The functions never throw; animation loops stay robust under misuse.

| Situation                          | Behavior                                    |
| ---------------------------------- | ------------------------------------------- |
| `beginDraw` while not buffering    | Prefill, enter buffering, return `true`     |
| `beginDraw` while already buffering| Re-prefill with new color, stay buffering, return `true` |
| `endDraw` while buffering          | Flush once, exit buffering, return `true`   |
| `endDraw` while not buffering      | No-op, no flush, return `false`             |

## Files Changed

1. `src/display.cc` — add `g_buffering` flag and `PresentIfLive()` helper; add
   `beginDraw` and `endDraw`; replace the 9 `LCD_1in44_Display(displayBuffer)`
   calls in the drawing functions with `PresentIfLive()`.
2. `src/display.h` — declare `beginDraw` and `endDraw`.
3. `src/addon.cc` — export `beginDraw` and `endDraw` on the `Display` object.
4. `binding.d.ts` — add to the `Display` interface:
   ```ts
   beginDraw(color?: number): boolean;
   endDraw(): boolean;
   ```
5. `test/test_binding.js` — add tests (see below).

## Testing

Following the existing test style in `test/test_binding.js`:

- **Buffered cycle:** call `beginDraw()`, perform several draw operations
  (e.g. `drawPngAt` / `clear` / `setPixel`), then `endDraw()`; assert
  `beginDraw` returns `true` and `endDraw` returns `true`.
- **Misuse:** call `endDraw()` without a preceding `beginDraw()`; assert it
  returns `false`.
- **Double begin:** call `beginDraw()` twice, then `endDraw()`; assert both
  `beginDraw` calls return `true` and `endDraw` returns `true`.

The visual flicker reduction itself is verified manually on-device, since the
automated test suite runs without display hardware attached.

## Out of Scope (YAGNI)

- No separate/second in-memory buffer allocation.
- No nesting/depth counting for buffered sessions (a single boolean flag is
  sufficient; nested `beginDraw` re-prefills rather than stacking).
- No changes to color representation beyond matching the existing `clear()`
  semantics.
