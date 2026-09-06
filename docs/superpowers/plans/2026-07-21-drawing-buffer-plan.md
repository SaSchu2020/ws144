# Drawing Buffer (Deferred Flush) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `Display.beginDraw(color?)` and `Display.endDraw()` so an entire animation frame flushes to the LCD exactly once, eliminating multi-flush flicker.

**Architecture:** Reuse the existing in-memory `displayBuffer`. A file-scoped `g_buffering` flag in `display.cc` gates a new `PresentIfLive()` helper that replaces every direct `LCD_1in44_Display(displayBuffer)` call in the drawing functions. `beginDraw` prefills the buffer and enables buffering (no flush); `endDraw` performs the single flush and disables buffering.

**Tech Stack:** Node.js native addon (Napi), C++

**Reference spec:** `docs/superpowers/specs/2026-07-21-drawing-buffer-design.md`

**Testing note:** This project has no unit-test framework. Tests are integration-style in `test/test_binding.js`, run via `node test/test_binding.js`. GPIO/SPI errors are expected on non-Pi (x64) hardware; the goal of the automated test is to confirm the functions are exported and return the correct booleans. Visual flicker reduction is verified manually on-device.

---

## Task 1: Add g_buffering flag and PresentIfLive() helper to display.cc

**Files:**
- Modify: `src/display.cc:11` (after the `#include "display.h"` line, before `writeText`)

- [ ] **Step 1: Add the buffering flag and flush helper**

Insert after line 11 (`#include "display.h"`) and before the `writeText` function (line 13):

```cpp

// Deferred-flush state for beginDraw()/endDraw().
// When buffering is active, drawing operations write to displayBuffer but
// skip the per-operation hardware flush. endDraw() performs the single flush.
static bool g_buffering = false;

// Flush the buffer to the physical LCD only when not in a buffered session.
static void PresentIfLive() {
    if (!g_buffering) {
        LCD_1in44_Display(displayBuffer);
    }
}
```

- [ ] **Step 2: Verify it compiles**

Run: `npm run build`
Expected: Build succeeds with no errors (helper is unused so far — a warning is acceptable, an error is not).

- [ ] **Step 3: Commit**

```bash
git add src/display.cc
git commit -m "feat: add g_buffering flag and PresentIfLive helper"
```

---

## Task 2: Route all drawing functions through PresentIfLive()

**Files:**
- Modify: `src/display.cc` (9 occurrences)

Replace each direct flush inside the drawing functions with the gated helper. Do NOT touch the flush inside the new functions from Task 3, and do NOT touch `device.cc`.

- [ ] **Step 1: Replace the flush in every drawing function**

In `src/display.cc`, there are 9 lines that read exactly:

```cpp
    LCD_1in44_Display(displayBuffer);
```

They appear at the end of: `writeText`, `drawBitmap`, `drawPng`, `drawBitmapAt`, `drawPngAt`, `setPixel`, `drawImageBuffer`, `clear`, and `clearRectangle`.

Replace each of these 9 occurrences with:

```cpp
    PresentIfLive();
```

Note: preserve the existing 4-space indentation on each replaced line.

- [ ] **Step 2: Verify no direct flush calls remain in drawing functions**

Run: `rg -n "LCD_1in44_Display" src/display.cc`
Expected: Exactly one match — the call inside the `PresentIfLive()` helper (added in Task 1). No drawing function should still call it directly.

- [ ] **Step 3: Verify it compiles**

Run: `npm run build`
Expected: Build succeeds with no errors.

- [ ] **Step 4: Commit**

```bash
git add src/display.cc
git commit -m "refactor: route drawing functions through PresentIfLive"
```

---

## Task 3: Implement beginDraw and endDraw in display.cc

**Files:**
- Modify: `src/display.cc` (append at end of file)

- [ ] **Step 1: Add beginDraw and endDraw implementations**

Append at the end of `src/display.cc`:

```cpp
Napi::Boolean beginDraw(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD color = 0x0000;

    if (info.Length() > 0) {
        color = (uint32_t)info[0].As<Napi::Number>() << 8;
    }

    Paint_Clear(color);

    g_buffering = true;

    return Napi::Boolean::New(env, true);
}

Napi::Boolean endDraw(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (!g_buffering) {
        return Napi::Boolean::New(env, false);
    }

    g_buffering = false;

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

Notes:
- The `<< 8` shift and `0x0000` default match the existing `clear()` semantics.
- Calling `beginDraw` while already buffering simply runs `Paint_Clear(color)` again and leaves `g_buffering` true — this is the intended "re-prefill with new color, stay buffering" behavior; no special-casing is needed.
- `endDraw` returns `false` and does not flush when no session is active.

- [ ] **Step 2: Verify it compiles**

Run: `npm run build`
Expected: Build succeeds with no errors.

- [ ] **Step 3: Commit**

```bash
git add src/display.cc
git commit -m "feat: implement beginDraw and endDraw"
```

---

## Task 4: Declare beginDraw and endDraw in display.h

**Files:**
- Modify: `src/display.h:14` (after the `drawPngAt` declaration)

- [ ] **Step 1: Add declarations**

In `src/display.h`, add after line 14 (`Napi::Boolean drawPngAt(const Napi::CallbackInfo& info);`) and before `#endif`:

```cpp
Napi::Boolean beginDraw(const Napi::CallbackInfo& info);
Napi::Boolean endDraw(const Napi::CallbackInfo& info);
```

- [ ] **Step 2: Commit**

```bash
git add src/display.h
git commit -m "feat: declare beginDraw and endDraw in display.h"
```

---

## Task 5: Export beginDraw and endDraw in addon.cc

**Files:**
- Modify: `src/addon.cc:30` (after the `clearRectangle` export)

- [ ] **Step 1: Add exports on the Display object**

In `src/addon.cc`, find the line (around line 30):

```cpp
    display.Set(Napi::String::New(env, "clearRectangle"), Napi::Function::New(env, clearRectangle));
```

Add immediately after it:

```cpp
    display.Set(Napi::String::New(env, "beginDraw"), Napi::Function::New(env, beginDraw));
    display.Set(Napi::String::New(env, "endDraw"), Napi::Function::New(env, endDraw));
```

The declarations are already visible via `#include "display.h"` (line 8), so no separate declaration is needed in `addon.cc`.

- [ ] **Step 2: Verify it compiles**

Run: `npm run build`
Expected: Build succeeds with no errors.

- [ ] **Step 3: Commit**

```bash
git add src/addon.cc
git commit -m "feat: export beginDraw and endDraw"
```

---

## Task 6: Add TypeScript type declarations

**Files:**
- Modify: `binding.d.ts:28` (inside the `Display` interface, after `clearRectangle`)

- [ ] **Step 1: Add beginDraw and endDraw to the Display interface**

In `binding.d.ts`, the `Display` interface's `clearRectangle` declaration ends at line 28 with `): boolean;`. Add immediately after it (still inside the interface, before the closing `}` on line 29):

```ts
  beginDraw(color?: number): boolean;
  endDraw(): boolean;
```

- [ ] **Step 2: Commit**

```bash
git add binding.d.ts
git commit -m "feat: add beginDraw and endDraw type declarations"
```

---

## Task 7: Add integration test for buffered drawing

**Files:**
- Modify: `test/test_binding.js`

- [ ] **Step 1: Add the test function**

In `test/test_binding.js`, add this function after `testClearRectangle` (after line 109 / end of file):

```javascript
function testDrawingBuffer() {
  console.log("\n=== Testing drawing buffer (beginDraw/endDraw) ===");

  // endDraw with no active session returns false
  const orphanEnd = Display.endDraw();
  console.log("endDraw() with no active session:", orphanEnd, "(expected false)");

  // Normal buffered cycle
  const begin1 = Display.beginDraw();
  console.log("beginDraw():", begin1, "(expected true)");
  Display.drawBitmap("./src/demo.bmp");
  Display.setPixel(64, 64, 0xff00);
  Display.writeText("Buffered", 10, 10, 0x0000, 0xffff);
  const end1 = Display.endDraw();
  console.log("endDraw():", end1, "(expected true)");

  // Buffered cycle with a fill color, then a double beginDraw (re-prefill)
  const begin2 = Display.beginDraw(0x2000);
  console.log("beginDraw(0x2000):", begin2, "(expected true)");
  const begin3 = Display.beginDraw(0x0010);
  console.log("beginDraw(0x0010) while buffering:", begin3, "(expected true)");
  Display.writeText("ReFilled", 10, 30, 0xffff, 0x0000);
  const end2 = Display.endDraw();
  console.log("endDraw():", end2, "(expected true)");

  const ok = orphanEnd === false && begin1 && end1 && begin2 && begin3 && end2;
  console.log("testDrawingBuffer result:", ok);
  return ok;
}
```

- [ ] **Step 2: Call the test in the main sequence**

In `test/test_binding.js`, find the line inside the async IIFE that calls the existing test (line 93):

```javascript
    testClearRectangle();
```

Replace it with:

```javascript
    testClearRectangle();
    testDrawingBuffer();
```

- [ ] **Step 3: Build and run the test**

Run: `npm run build && node test/test_binding.js`
Expected: The build succeeds. The test logs show `endDraw() with no active session: false`, all `beginDraw`/`endDraw` calls return `true`, and `testDrawingBuffer result: true`. GPIO/SPI errors are expected and acceptable on non-Pi hardware.

- [ ] **Step 4: Commit**

```bash
git add test/test_binding.js
git commit -m "test: add integration test for beginDraw/endDraw"
```

---

## Task 8: Final verification

- [ ] **Step 1: Confirm the full build is clean**

Run: `npm run build`
Expected: Build succeeds with no errors.

- [ ] **Step 2: Confirm the flush call sites are correct**

Run: `rg -n "LCD_1in44_Display" src/display.cc`
Expected: Exactly two matches — one inside the `PresentIfLive()` helper and one inside `endDraw`. No individual drawing function should call it directly.

- [ ] **Step 3: Confirm exports and types are in place**

Run: `rg -n "beginDraw|endDraw" src/addon.cc binding.d.ts`
Expected: `beginDraw` and `endDraw` appear in both `src/addon.cc` (exports) and `binding.d.ts` (type declarations).
