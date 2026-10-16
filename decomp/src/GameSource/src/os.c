/* os.c - TV/cable type globals. Matches. (osSetIntMask was stripped by the linker.)
   Uninitialised globals are laid out in reverse declaration order. */

enum {
    SYE_CBL_VGA = 0,
    SYE_CBL_PAL = 1,
    SYE_CBL_NTSC = 2
} osCblType;

signed char osTvType;
