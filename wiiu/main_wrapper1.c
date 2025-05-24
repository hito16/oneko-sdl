/* Reusable WiiU homebrew app to wrap "main()"

In an ideal situation, our CLI or SDL program can compile and 
run on our Native OS.

With any luck, the "porting" is just calling the program's main 
with this wrapper.

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

#include <sysapp/title.h>       // SYSCheckTitleExists()

int real_main(int argc, char** argv);

#undef main  // build with -Dmain=real_main

int main(int argc, char** argv) {
    SYSCheckTitleExists(0); // workaround for SDL bug
    return real_main(argc, argv);
}
