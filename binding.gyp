{
  'targets': [
    {
      'target_name': 'ws144',
      'sources': ["<!@(find src -regex \".*\.\(c\|cc\)\")"],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")", "./src", "./src/Config", "./src/Fonts", "./src/GUI", "./src/LCD", "./lgpio"],
      'dependencies': ["<!(node -p \"require('node-addon-api').targets\"):node_addon_api" ],
      'defines': ["USE_DEV_LIB"],
      'cflags!': [ '-fno-exceptions', "-g -O0 -Wall"],
      'cflags_cc!': [ '-fno-exceptions', "-g -O0 -Wall"],
      "conditions": [
          # Linux x86_64
          [
            "OS=='linux' and target_arch=='x64'",
            {
              "libraries": [
                "-lm",
                "-L<(module_root_dir)/lgpio/dist/x64/lib",
                "-llgpio",
                "-Wl,-rpath,'$$ORIGIN/../lgpio/dist/x64/lib'"
              ],
              "ldflags": [
                "-Wl,-rpath,<(module_root_dir)/lgpio/dist/x64/lib"
              ]
            }
          ],
          # Linux ARM 32-bit (Raspberry Pi Zero, Pi 1)
          [
            "OS=='linux' and target_arch=='arm'",
            {
              "libraries": [
                "-lm",
                "-L<(module_root_dir)/lgpio/dist/arm/lib",
                "-llgpio",
                "-Wl,-rpath,'$$ORIGIN/../lgpio/dist/arm/lib'"
              ],
              "ldflags": [
                "-Wl,-rpath,<(module_root_dir)/lgpio/dist/arm/lib"
              ]
            }
          ],
          # Linux ARM 64-bit (Raspberry Pi 3, 4, 5)
          [
            "OS=='linux' and target_arch=='arm64'",
            {
              "libraries": [
                "-lm",
                "-L<(module_root_dir)/lgpio/dist/arm64/lib",
                "-llgpio",
                "-Wl,-rpath,'$$ORIGIN/../lgpio/dist/arm64/lib'"
              ],
              "ldflags": [
                "-Wl,-rpath,<(module_root_dir)/lgpio/dist/arm64/lib"
              ]
            }
          ]
        ]
    }
  ]
}
