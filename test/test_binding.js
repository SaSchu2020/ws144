const {Device, Display, Keys} = require("../binding.js");

let keyPresses = [];
let interval;

let activeItem = 0;

const KeyMap = new Map([
    [0, "Up"],
    [1, "Down"],
    [2, "Left"],
    [3, "Right"],
    [4, "Key"],
    [5, "1"],
    [6, "2"],
    [7, "3"]
]);

async function quit() {
    Device.changeDisplayState(false);

    clearInterval(interval);

    Device.release();
}

(async () => {
    Device.initialize();

    interval = setInterval(() => {
        keyPresses = Keys.getState();

        keyPresses.forEach((isPressed, index) => {
           if (isPressed) {
               console.log("Pressed: " + KeyMap.get(index));

               Display.drawBitmap("./src/demo.bmp");

               if (activeItem === 0) {
                   Display.writeText("Do nothing", 10, 50, 0x0000, 0xffff);
                   Display.writeText("Quit", 10, 70, 0xffff, 0x0000);
               }
               else {
                   Display.writeText("Do nothing", 10, 50, 0xffff, 0x0000);
                   Display.writeText("Quit", 10, 70, 0x0000, 0xffff);
               }

               if (index === 5 && activeItem === 1) {
                   quit();

                   process.exit(0);
               }

               if (index === 0 || index === 1) {
                   activeItem ^= 1;
               }
           }
        });
    }, 50)

    Display.drawBitmap("./src/demo.bmp");

    await new Promise((r) => setTimeout(r, 1000));

    Display.clear(0x2000);

    await new Promise((r) => setTimeout(r, 1000));

    Display.writeText("Hello World!", 10, 10, 0x000000, 0xffffff);

    await new Promise((r) => setTimeout(r, 1000));

    Display.setPixel(64, 64, 0xff00);
    Display.setPixel(65, 64, 0xff00);
    Display.setPixel(64, 65, 0x00ff);
    Display.setPixel(65, 65, 0x00ff);

    await new Promise((r) => setTimeout(r, 1000));

    Display.drawImageBuffer([
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,0,0,0,0,0]
    ]);

    testClearRectangle();
    testDrawingBuffer();
})();

function testClearRectangle() {
  console.log("\n=== Testing clearRectangle ===");

  const result = Display.clearRectangle(10, 10, 50, 50);
  console.log("clearRectangle(10, 10, 50, 50):", result);

  const result2 = Display.clearRectangle(50, 50, 10, 10, 0xF800);
  console.log("clearRectangle(50, 50, 10, 10, 0xF800) [inverted coords]:", result2);

  const result3 = Display.clearRectangle(0, 0, 127, 127);
  console.log("clearRectangle(0, 0, 127, 127) [full screen]:", result3);

  return result && result2 && result3;
}

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

  // Buffered cycle skipping full clear, using clearRectangle for partial clear
  const begin4 = Display.beginDraw(false);
  console.log("beginDraw(false):", begin4, "(expected true)");
  Display.clearRectangle(20, 20, 60, 60, 0x07E0);
  Display.writeText("Partial", 30, 70, 0x0000, 0xffff);
  const end3 = Display.endDraw();
  console.log("endDraw() after partial clear:", end3, "(expected true)");

  const ok = orphanEnd === false && begin1 && end1 && begin2 && begin3 && end2 && begin4 && end3;
  console.log("testDrawingBuffer result:", ok);
  return ok;
}