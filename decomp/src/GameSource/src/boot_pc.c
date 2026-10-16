/* boot_pc.c - program entry. Matches. */
#include "rush/rush_protos.h"

signed char gNoAudio;

/* 0x80005df8 */
void main(int argc, char **argv)
{
    appParseCmdArgs(argc, argv);
    if (GSettings.bNoSound)
        gNoAudio = 1;
    InitSystem();
    gStarted = 1;
    while (BeginLoop()) {
    }
    MainLoop();
}
