/* Reusable WiiU homebrew app to wrap "main()"

SDL is cross platform library.  Ideally the SDL app contains no 
platform specific api, so  "porting" an SDL app is just calling 
that program's main() with this wrapper.

  Step 1. Compile your program in linux, mac, etc. and see it working
      as a normal cli or server.
  Step 2. Get it compiled for Devkitpro PPC w/ WUT.  It won't run yet,
      but you figured out your build dependencies.
  Step 3: In your other program, make these changes to main()
     #ifdef __WIIU__
         int myapp_main(int argc, char* argv[]) {
     #else
         int main(int argc, char* argv[]) {
     #endif
  Step 4: Build myapp_main as a library or just link it in here.


*/

#ifndef HAS_DKO_SDL_QUIT_FIXES
#include <sysapp/title.h>       // SYSCheckTitleExists()
#include <sysapp/launch.h>
#include <whb/proc.h>
#endif

#include <SDL.h>	// check if we have latest version

int real_main(int argc, char** argv);

// build with -Dmain=real_main 
// Rename the SDL app's "main()" symbol during compile time so we don't need to 
// modify the original SDL code
#undef main  

int main(int argc, char** argv) {

// This code relies on a custom, unrelased version of SDL
// - ProcUI cleanup on `SDL_Quit()`
// - Foreground/background events.
//
// Remove #else section once DKO_CUSTOM_SDL fixes
// are included in the next official SDL release
//
#ifdef HAS_DKO_SDL_QUIT_FIXES
    return real_main(argc, argv);  // isn't the future so simple and clean?
#else // planned obsolence
    SYSCheckTitleExists(0);      // workaround for SDL bug

    int res = real_main(argc,argv);

    // User pressed '-', initiating an exit. 
    // We need to cleanly return to WiiU main menu.
    // ie. workaround for SDL_QUIT and SDL_Quit()
    WHBProcInit();              // pre-req for WHBProcIsRunning()
    SYSLaunchMenu();            // tell WiiU app we're quitting 
    while (WHBProcIsRunning()); // check WiiU process state until ok to exit.

    return res;
#endif
}
