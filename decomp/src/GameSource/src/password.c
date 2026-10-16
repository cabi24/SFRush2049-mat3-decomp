/* password.c - save passwords: bit-packed data plus a checksum, written in a 32-character alphabet.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\password.c; assembly: build/2049LE/asm/GameSource/src/password.s */
#include "rush/rush_protos.h"

static unsigned char gInversePasswordAlphabet[256] = { 255 };
unsigned char *gPasswordAlphabet = (unsigned char *)"BCDFGHJKLMNPQRTVWXY123456789@#%?";
static unsigned short gNumBits;
static unsigned short gChecksumBits;
static unsigned short gDataBits;
static unsigned short gNumBytes;
static unsigned char gData[32];

static unsigned int CalcChecksum(void);

/* 0x80030bdc, size 0x20c */
int NewPassword(unsigned int numDataBits, unsigned int numChecksumBits)
{
    int i; /* r29 */

    static int first = 1;

    if (first) {
        first = 0;
        for (i = 0; i < 32; i++) {
            gInversePasswordAlphabet[gPasswordAlphabet[i]] = i;
        }
    }
    gNumBytes = (numDataBits + numChecksumBits + 7) >> 3;
    if (gNumBytes > 32) {
        return 0;
    }
    if (numChecksumBits > 32) {
        return 0;
    }
    for (i = 0; i < gNumBytes; i++) {
        gData[i] = 0;
    }
    gDataBits = numDataBits;
    gChecksumBits = numChecksumBits;
    gNumBits = 0;
    return 1;
}

/* 0x80030de8, size 0x11c */
int StorePasswordElement(unsigned int data, unsigned int numBits)
{
    int b; /* r0 */

    if (numBits > 32) {
        return 0;
    }
    if (gNumBits + numBits > gDataBits) {
        return 0;
    }
    for (b = 0; b < numBits; b++, gNumBits++) {
        gData[gNumBits >> 3] |= (data & 1) << (gNumBits & 7);
        data >>= 1;
    }
    return 1;
}

/* 0x80030f04, size 0x90 */
int GetPasswordElement(unsigned int *data, unsigned int numBits)
{
    unsigned short b; /* r9 */

    if (numBits > 32) {
        return 0;
    }
    if (gNumBits + numBits > gDataBits) {
        return 0;
    }
    *data = 0;
    for (b = 0; b < numBits; b++) {
        *data |= ((gData[gNumBits >> 3] >> (gNumBits & 7)) & 1) << b;
        gNumBits++;
    }
    return 1;
}

/* 0x80030f94, size 0x14c */
void GetPasswordString(char *password)
{
    unsigned int sum; /* r3 */
    unsigned short i; /* r12 */
    unsigned short b; /* r30 */
    unsigned short bit; /* r7 */
    unsigned short numBits; /* r8 */
    unsigned short numChars; /* r9 */
    unsigned char data; /* r10 */

    sum = CalcChecksum();
    for (i = 0, b = gDataBits; i < gChecksumBits; i++, b++) {
        gData[b >> 3] &= ~(1 << (b & 7));
        gData[b >> 3] |= (sum & 1) << (b & 7);
        sum >>= 1;
    }
    numBits = gDataBits + gChecksumBits;
    numChars = 0;
    for (i = 0, bit = 0, data = 0; i < numBits; i++) {
        data |= ((gData[i >> 3] >> (i & 7)) & 1) << bit;
        if (bit == 4) {
            password[numChars++] = gPasswordAlphabet[data];
            bit = 0;
            data = 0;
        } else {
            bit++;
        }
    }
    if (bit) {
        password[numChars++] = gPasswordAlphabet[data];
    }
    password[numChars] = 0;
}

/* 0x800310e0, size 0x210 */
int SetPasswordString(char *password)
{
    unsigned int sum; /* r9 */
    unsigned int data; /* r3 */
    unsigned short i; /* r3 */
    unsigned short b; /* r10 */

    for (i = 0; i < gNumBytes; i++) {
        gData[i] = 0;
    }
    gNumBits = 0;
    gDataBits += gChecksumBits;
    while (*password) {
        data = gInversePasswordAlphabet[*password++];
        if (data == 0xff) {
            gDataBits -= gChecksumBits;
            return 0;
        }
        if (!StorePasswordElement(data, 5)) {
            gDataBits -= gChecksumBits;
            return 0;
        }
    }
    gDataBits -= gChecksumBits;
    sum = CalcChecksum();
    sum &= (1 << gChecksumBits) - 1;
    data = 0;
    for (i = 0, b = gDataBits; i < gChecksumBits; i++, b++) {
        data |= ((gData[b >> 3] >> (b & 7)) & 1) << i;
    }
    if (sum != data) {
        return 0;
    }
    gNumBits = 0;
    return 1;
}

/* 0x800312f0, size 0x70 */
static unsigned int CalcChecksum(void)
{
    unsigned int sum; /* r3 */
    unsigned int add; /* r0 */
    unsigned int b; /* r9 */

    if (!gChecksumBits) {
        return 0;
    }
    for (b = 0, sum = 0; b < gDataBits; b++) {
        add = gData[b >> 3] & (1 << (b & 7));
        while (add >= (1 << gChecksumBits)) {
            add >>= gChecksumBits;
        }
        sum += add;
    }
    return sum;
}

