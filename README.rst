=============
node-pyksdl2
=============

Low-level bindings for SDL2, aiming to emulate the C API as much as possible. 
Note that this binding is still in a very early stage of development.

Development of node-pyksdl2 is currently tied to bb4.

Installation
=============
Dependencies:

* Python (2.6+/3)
* node-gyp

Dependencies (Linux):

* nasm
* libx11-dev
* libxext-dev
* libxt-dev
* libxv-dev
* x11proto-core-dev
* libts-dev
* libpulse-dev (for Pulseaudio support in SDL2)
* libgl1-mesa-dev
* libasound2-dev
* libcaca-dev
* libusbhid-dev
* libglu1-mesa-dev

Dependencies (Windows):

* DirectX SDK

Since there is no release yet, node-pyksdl2 has not been uploaded to npm.
node-pyksdl2 currently only builds on Windows and Linux. libSDL2 is not
required to be installed, as SDL2 sources are included in the repository.

To build, run in the cloned repository::

    node-gyp rebuild

License
========
node-pyksdl2 is licensed under the MIT License. 
libSDL2 is licensed under the zlib license.
