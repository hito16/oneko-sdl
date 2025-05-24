# Oneko-sdl WiiU recompile

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

Thanks to DanielKO for his help getting this work on the WiiU. Especially, thankful for his changes
to the WiiU cod e
