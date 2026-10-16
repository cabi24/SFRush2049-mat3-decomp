/* ngcDmalist.c - PS2 DMA list shim. Matches. (ReserveDmaBuffer, InitDMAList, SendDMAListToDisplay,
   CurrentDMAReady and FakeSendDMA were stripped by the linker.) */
#include "rush/rush_protos.h"

/* 0x80097888 */
int GetWorkingBufIdx(void)
{
    return 0;
}
