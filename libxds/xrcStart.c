/* xrcStart.c - target-specific startup code */
/* Copyright (c) 2000 Excelsior. */

#if defined _MSC_VER
/* Microsoft Visual C++ */

/*
  MSVC runtime library calls WinMain() function instead of main()
  if the program is built for the WINDOWS subsystem, whereas XDS
  always generates main() from the body of the main module.
  Fortunately, MSVC's stdlib.h contains variables that may be
  passed to main().
*/

#include <stdlib.h>
#include <windows.h>

extern int main(int argc, char *argv[]);

int WINAPI WinMain(
    HINSTANCE hInstance,	/* handle to current instance  */
    HINSTANCE hPrevInstance,	/* handle to previous instance */
    LPSTR lpCmdLine,	        /* pointer to command line     */
    int nCmdShow 	        /* show state of window        */
   )

{
	return main(__argc, __argv);
}
#else
/* default */

/*
  Some C compilers do not like to compile empty files.
  The function below only aims at fooling such compilers.
*/

#ifndef X2C_KRC
void xrcStart_INIT(void)
#else
void xrcStart_INIT()
#endif
{
}


#endif /* #if defined _MSC_VER */
