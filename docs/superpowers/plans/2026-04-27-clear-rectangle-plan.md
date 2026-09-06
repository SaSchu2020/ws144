# clearRectangle Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `clearRectangle(x1, y1, x2, y2, color?)` function to clear a rectangular region of the 128x128 LCD display.

**Architecture:** The JavaScript binding delegates to the existing `Paint_ClearWindow` C++ function in GUI_Paint.cc. New implementation handles coordinate normalization (swapping if inverted) and boundary clamping before delegating.

**Tech Stack:** Node.js native addon (Napi), C++

---

## Task 1: Add clearRectangle declaration to display.h

**Files:**
- Modify: `src/display.h:11`

- [ ] **Step 1: Add clearRectangle declaration**

Add after line 11 (after `clear`):
```cpp
Napi::Boolean clearRectangle(const Napi::CallbackInfo& info);
```

---

## Task 2: Implement clearRectangle in display.cc

**Files:**
- Modify: `src/display.cc:139` (append before end of file)

- [ ] **Step 1: Add clearRectangle implementation**

Append before the closing of the file:
```cpp
Napi::Boolean clearRectangle(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    UWORD x1 = (uint32_t)info[0].As<Napi::Number>();
    UWORD y1 = (uint32_t)info[1].As<Napi::Number>();
    UWORD x2 = (uint32_t)info[2].As<Napi::Number>();
    UWORD y2 = (uint32_t)info[3].As<Napi::Number>();

    UWORD color = 0x0000;
    if (info.Length() > 4) {
        color = (uint32_t)info[4].As<Napi::Number>() << 8;
    }

    if (x1 > x2) {
        UWORD tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        UWORD tmp = y1;
        y1 = y2;
        y2 = tmp;
    }

    if (x1 >= LCD_WIDTH) {
        printf("clearRectangle: x1 (%d) >= LCD_WIDTH (%d), clamping to 0\n", x1, LCD_WIDTH);
        x1 = 0;
    }
    if (y1 >= LCD_HEIGHT) {
        printf("clearRectangle: y1 (%d) >= LCD_HEIGHT (%d), clamping to 0\n", y1, LCD_HEIGHT);
        y1 = 0;
    }
    if (x2 >= LCD_WIDTH) {
        printf("clearRectangle: x2 (%d) >= LCD_WIDTH (%d), clamping to %d\n", x2, LCD_WIDTH, LCD_WIDTH - 1);
        x2 = LCD_WIDTH - 1;
    }
    if (y2 >= LCD_HEIGHT) {
        printf("clearRectangle: y2 (%d) >= LCD_HEIGHT (%d), clamping to %d\n", y2, LCD_HEIGHT, LCD_HEIGHT - 1);
        y2 = LCD_HEIGHT - 1;
    }

    if (x2 < x1 || y2 < y1) {
        DEBUG("clearRectangle: invalid rectangle after clamping");
        return Napi::Boolean::New(env, false);
    }

    Paint_ClearWindow(x1, y1, x2 + 1, y2 + 1, color);

    LCD_1in44_Display(displayBuffer);

    return Napi::Boolean::New(env, true);
}
```

**Note:** The `Paint_ClearWindow` function takes exclusive end coordinates (Xend, Yend), so we add 1 to x2 and y2.

---

## Task 3: Export clearRectangle in addon.cc

**Files:**
- Modify: `src/addon.cc`

- [ ] **Step 1: Add clearRectangle export**

Find the existing `clear` export around line 29 and add `clearRectangle` nearby:
```cpp
display.Set(Napi::String::New(env, "clear"), Napi::Function::New(env, clear));
display.Set(Napi::String::New(env, "clearRectangle"), Napi::Function::New(env, clearRectangle));
```

- [ ] **Step 2: Add function declaration at top of addon.cc**

Add after the other function declarations:
```cpp
Napi::Boolean clearRectangle(const Napi::CallbackInfo& info);
```

---

## Task 4: Build and test

**Files:**
- Test: `test/test_binding.js`

- [ ] **Step 1: Build the addon**

Run: `npm run build`

Expected: Build succeeds with no errors

- [ ] **Step 2: Add test for clearRectangle**

Add to `test/test_binding.js`:
```javascript
function testClearRectangle() {
  console.log("\n=== Testing clearRectangle ===");

  const result = waveshare.clearRectangle(10, 10, 50, 50);
  console.log("clearRectangle(10, 10, 50, 50):", result);

  const result2 = waveshare.clearRectangle(50, 50, 10, 10, 0xF800);
  console.log("clearRectangle(50, 50, 10, 10, 0xF800) [inverted coords]:", result2);

  const result3 = waveshare.clearRectangle(0, 0, 127, 127);
  console.log("clearRectangle(0, 0, 127, 127) [full screen]:", result3);

  return result && result2 && result3;
}
```

And call `testClearRectangle()` in the test function.

- [ ] **Step 3: Run tests**

Run: `node test/test_binding.js`

Expected: All tests pass (GPIO errors expected on non-Pi hardware)

---

## Task 5: Commit

- [ ] **Step 1: Commit changes**

```bash
git add src/display.h src/display.cc src/addon.cc test/test_binding.js
git commit -m "feat: add clearRectangle function"
```