# Oneko-sdl WiiU recompile

## Install

copy the following folder dist/wiiu/apps/onekosdl/

to the root of your SD card. When complete, you SD card should look like

```
sd:/
  wiiu/
    apps/
      onekosdl/
        icon.png  
        meta.xml
        oneko_sdl_wiiu.rpx
        oneko_sdl_wiiu.wuhb
 
```

* [Aroma]  Load the new app from the WiiU home menu.
* [Haxchi] Load the rpx through the homebrew channel

## Build

1. Set up a WiiU devkitpro PPC build environment or run Docker

```
# given the following directory structure, start Docker from the oneko directory
# 
# oneko-sdl/
#     - sdlport/
#     - wiiu/

cd oneko-sdl
docker build -t oneko-sdl wiiu
docker run -it --rm -v ${PWD}:/project --name oneko-sdl --hostname devkitppc oneko-sdl /bin/bash

```

Caveat: The code will work, and exit property using the Home Button. However, to get full functionality 
(exit cleanly using "-" button), we need to use pre-release  of WiiU SDL with addtional fixes.

2. Compile

simply run "make"

```
root@devkitppc:/project# cd wiiu
root@devkitppc:/project/wiiu# make
...
built ... oneko_sdl_wiiu.wuhb
```

3. Test

Note the ipaddress of your WiiU. Eg. open Wii U Internet Browser, enable developer tools. (should be google-able)

After you've run make, load the app with "wiiload" in your devkitpro environment.

```
root@devkitppc:/project/wiiu# ls
build  Dockerfile  main_wrapper1.c  Makefile  oneko_sdl_wiiu.elf  oneko_sdl_wiiu.rpx  oneko_sdl_wiiu.wuhb  README.md

root@devkitppc:/project/wiiu# export WIILOAD=tcp:192.168.0.100

root@devkitppc:/project/wiiu# wiiload oneko_sdl_wiiu.wuhb
```

## Acknowledgements

Thanks to DanielKO for his help getting this to work on the WiiU. There is no way I would have gotten
proper exit behavior without him, and his SDL fixes will prevent future pain.


# notes on patched SDL

DanielKO provided patched SDL source to support exiting. Without these fixes, 
the app cannot exit back to the memory correctly.  In its current form,
you can press the "Home" button, and exit to the Home Menu.  However, exiting
from within the app (press '-') doesn't work correctly.
