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
})();