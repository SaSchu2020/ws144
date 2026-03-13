{
  "targets": [
    {
      "target_name": "ws144",
      "sources": [
        "<!@(find src -regex \".*\\.\\(c\\|cc\\)\")",
        "lgpio/lgCtx.c",
        "lgpio/lgDbg.c",
        "lgpio/lgErr.c",
        "lgpio/lgGpio.c",
        "lgpio/lgHdl.c",
        "lgpio/lgI2C.c",
        "lgpio/lgNotify.c",
        "lgpio/lgPthAlerts.c",
        "lgpio/lgPthTx.c",
        "lgpio/lgSerial.c",
        "lgpio/lgSPI.c",
        "lgpio/lgThread.c",
        "lgpio/lgUtil.c"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "./src",
        "./src/Config",
        "./src/Fonts",
        "./src/GUI",
        "./src/LCD",
        "./src/spng",
        "./lgpio"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').targets\"):node_addon_api"
      ],
      "defines": ["USE_DEV_LIB"],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "cflags": ["-pthread", "-O3", "-Wall", "-fpic"],
      "cflags_c": ["-std=gnu99"],
      "libraries": ["-lm", "-lrt", "-lpthread", "-lz"],
      "ldflags": ["-pthread"]
    }
  ]
}
