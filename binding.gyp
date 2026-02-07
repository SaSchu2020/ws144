{
  'targets': [
    {
      'target_name': 'ws144',
      'sources': ["<!@(find src -regex \".*\.\(c\|cc\)\")"],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")", "./src", "./src/Config", "./src/Fonts", "./src/GUI", "./src/LCD"],
      'dependencies': ["<!(node -p \"require('node-addon-api').targets\"):node_addon_api" ],
      'defines': ["USE_DEV_LIB"],
      'cflags!': [ '-fno-exceptions', "-g -O0 -Wall"],
      'cflags_cc!': [ '-fno-exceptions', "-g -O0 -Wall"],
      "libraries": ["-llgpio -lm"],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}
