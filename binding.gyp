{
  "targets": [
    {
      "target_name": "pyksdl2",
      "sources": ["src/pyksdl2.cpp", "src/constants.cpp"],
      'dependencies': ['deps/libsdl2/libsdl2.gyp:libsdl2'],
    }
  ]
}
# vim:set expandtab tabstop=2 shiftwidth=2:
