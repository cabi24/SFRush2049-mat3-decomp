/* unzip.c - In-memory Huffman inflater.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\unzip.c; assembly: build/2049LE/asm/GameSource/src/unzip.s */
#include "rush/rush_protos.h"

static void init_zip_mem(unsigned char *region, int regionsize);
static unsigned char *alloc_zip_mem(int amount);
static int huft_build(unsigned int *b, unsigned int n, unsigned int s, unsigned short *d, unsigned short *e, huft **t, int *m);
static int inflate_codes(huft *tl, huft *td, int bl, int bd);
static int inflate_stored(void);
static int inflate_fixed(void);
static int inflate_dynamic(void);
static int inflate_block(int *e);
static int inflate(void);
static unsigned int get_buffer(void);

static unsigned int border[19] = { 0x10, 0x11, 0x12, 0, 8, 7, 9, 6, 0xa, 5, 0xb, 4, 0xc, 3, 0xd, 2, 0xe, 1, 0xf };
static unsigned short cplens[31] = {
    3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xd, 0xf, 0x11, 0x13, 0x17, 0x1b, 0x1f, 0x23, 0x2b, 0x33, 0x3b,
    0x43, 0x53, 0x63, 0x73, 0x83, 0xa3, 0xc3, 0xe3, 0x102, 0, 0
};
static unsigned short cplext[31] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0x63,
    0x63
};
static unsigned short cpdist[30] = {
    1, 2, 3, 4, 5, 7, 9, 0xd, 0x11, 0x19, 0x21, 0x31, 0x41, 0x61, 0x81, 0xc1, 0x101, 0x181, 0x201,
    0x301, 0x401, 0x601, 0x801, 0xc01, 0x1001, 0x1801, 0x2001, 0x3001, 0x4001, 0x6001
};
static unsigned short cpdext[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 0xa, 0xa, 0xb, 0xb, 0xc, 0xc,
    0xd, 0xd
};
static unsigned short mask_bits[17] = {
    0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff,
    0xffff
};
static int lbits = 9;
static int dbits = 6;
static int zmmax = 0;
static int zmused = 0;
static unsigned char *zmregion = 0;
static unsigned int bk = 0;
static unsigned int bb = 0;
static unsigned char *unzip_buffer = 0;
static unsigned char *last_incoming = 0;
static unsigned char *incoming = 0;
static int s_buf[3000];

/* The original input path reads little-endian pairs and advances before
   combining them. It tests only the pair's first byte against the end. */
#define NEXT_PAIR() (incoming < last_incoming ? \
    (incoming += 2, (unsigned int)incoming[-2] | ((unsigned int)incoming[-1] << 8)) : get_buffer())
#define NEEDBITS(n) { while (k < (n)) { b |= NEXT_PAIR() << k; k += 16; } }
#define DUMPBITS(n) { b >>= (n); k -= (n); }

/* 0x80052460, size 0x14 */
static void init_zip_mem(unsigned char *region, int regionsize)
{
    zmregion = region;
    zmmax = regionsize;
    zmused = 0;
}

/* 0x80052474, size 0x1c */
static unsigned char *alloc_zip_mem(int amount)
{
    zmused += amount;
    return zmregion + (zmused - amount);
}

/* 0x80052490, size 0x580 */
static int huft_build(unsigned int *b, unsigned int n, unsigned int s, unsigned short *d, unsigned short *e, huft **t, int *m)
{
    unsigned int a; /* r18 */
    unsigned int c[17]; /* stack */
    unsigned int f; /* r5 */
    int g; /* r17 */
    int h; /* r16 */
    unsigned int i; /* r6 */
    unsigned int j; /* r7 */
    int k; /* r15 */
    int l; /* r25 */
    unsigned int *p; /* r7 */
    huft *q; /* r3 */
    huft r; /* stack */
    huft *u[16]; /* stack */
    unsigned int *v; /* r0 */
    int w; /* r31 */
    unsigned int x[17]; /* stack */
    unsigned int *xp; /* r5 */
    int y; /* stack */
    unsigned int z; /* r0 */
    memset(c, 0, sizeof(c));
    p = b;
    i = n;
    do { c[*p++]++; } while (--i);
    if (c[0] == n) { *t = 0; *m = 0; return 0; }
    l = *m;
    for (j = 1; j <= 16; j++) if (c[j]) break;
    k = j;
    if ((unsigned)l < j) l = j;
    for (i = 16; i; i--) if (c[i]) break;
    g = i;
    if ((unsigned)l > i) l = i;
    *m = l;
    for (y = 1 << j; j < i; j++, y <<= 1)
        if ((y -= c[j]) < 0) return 2;
    if ((y -= c[i]) < 0) return 2;
    c[i] += y;
    x[1] = j = 0;
    p = c + 1;
    xp = x + 2;
    while (--i) *xp++ = (j += *p++);
    v = (unsigned int *)alloc_zip_mem(288 * sizeof(unsigned int));
    p = b;
    i = 0;
    do { if ((j = *p++) != 0) v[x[j]++] = i; } while (++i < n);
    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = 0;
    q = 0;
    z = 0;
    for (; k <= g; k++) {
        a = c[k];
        while (a--) {
            while (k > w + l) {
                h++;
                w += l;
                z = g - w;
                z = z > (unsigned)l ? l : z;
                if ((f = 1 << (j = k - w)) > a + 1) {
                    f -= a + 1;
                    xp = c + k;
                    while (++j < z) {
                        if ((f <<= 1) <= *++xp) break;
                        f -= *xp;
                    }
                }
                z = 1 << j;
                q = (huft *)alloc_zip_mem((z + 1) * sizeof(huft));
                if (q == 0) return 3;
                *t = q + 1;
                *(t = &q->v.t) = 0;
                u[h] = ++q;
                if (h) {
                    x[h] = i;
                    r.b = l;
                    r.e = 16 + j;
                    r.v.t = q;
                    j = i >> (w - l);
                    u[h - 1][j] = r;
                }
            }
            r.b = k - w;
            if (p >= v + n) r.e = 99;
            else if (*p < s) {
                r.e = *p < 256 ? 16 : 15;
                r.v.n = *p++;
            } else {
                r.e = e[*p - s];
                r.v.n = d[*p++ - s];
            }
            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f) q[j] = r;
            for (j = 1 << (k - 1); i & j; j >>= 1) i ^= j;
            i ^= j;
            while ((i & ((1 << w) - 1)) != x[h]) { h--; w -= l; }
        }
    }
    return y != 0 && g != 1;
}

/* 0x80052a10, size 0x438 */
static int inflate_codes(huft *tl, huft *td, int bl, int bd)
{
    unsigned int e; /* r21 */
    unsigned int n; /* r23 */
    int back; /* r5 */
    huft *t; /* r22 */
    unsigned int ml; /* r24 */
    unsigned int md; /* r25 */
    unsigned int b; /* r30 */
    unsigned int k; /* r31 */
    b = bb;
    k = bk;
    ml = mask_bits[bl];
    md = mask_bits[bd];
    for (;;) {
        NEEDBITS(bl);
        if ((e = (t = tl + (b & ml))->e) > 16) {
            do {
                if (e == 99) return 1;
                DUMPBITS(t->b);
                e -= 16;
                NEEDBITS(e);
            } while ((e = (t = t->v.t + (b & mask_bits[e]))->e) > 16);
        }
        DUMPBITS(t->b);
        if (e == 16) *unzip_buffer++ = t->v.n;
        else {
            if (e == 15) break;
            NEEDBITS(e);
            n = t->v.n + (b & mask_bits[e]);
            DUMPBITS(e);
            NEEDBITS(bd);
            if ((e = (t = td + (b & md))->e) > 16) {
                do {
                    if (e == 99) return 1;
                    DUMPBITS(t->b);
                    e -= 16;
                    NEEDBITS(e);
                } while ((e = (t = t->v.t + (b & mask_bits[e]))->e) > 16);
            }
            DUMPBITS(t->b);
            NEEDBITS(e);
            back = -t->v.n - (b & mask_bits[e]);
            DUMPBITS(e);
            while (n--) { *unzip_buffer = unzip_buffer[back]; unzip_buffer++; }
        }
    }
    bb = b;
    bk = k;
    return 0;
}

/* 0x80052e48, size 0x174 */
static int inflate_stored(void)
{
    unsigned int n; /* r3 */
    unsigned int b; /* r30 */
    unsigned int k; /* r29 */
    k = bk;
    b = bb;
    n = k & 7;
    DUMPBITS(n);
    NEEDBITS(16);
    n = b & 0xffff;
    DUMPBITS(16);
    NEEDBITS(16);
    if (n != ((~b) & 0xffff)) return 1;
    DUMPBITS(16);
    while (n--) {
        NEEDBITS(8);
        *unzip_buffer++ = b;
        DUMPBITS(8);
    }
    bb = b;
    bk = k;
    return 0;
}

/* 0x80052fbc, size 0x388 */
static int inflate_fixed(void)
{
    int i; /* r7 */
    huft *tl; /* stack */
    huft *td; /* stack */
    int bl; /* stack */
    int bd; /* stack */
    unsigned int *l; /* r30 */
    l = (unsigned int *)alloc_zip_mem(288 * sizeof(unsigned int));
    for (i = 0; i < 144; i++) l[i] = 8;
    for (; i < 256; i++) l[i] = 9;
    for (; i < 280; i++) l[i] = 7;
    for (; i < 288; i++) l[i] = 8;
    bl = 7;
    if ((i = huft_build(l, 288, 257, cplens, cplext, &tl, &bl)) != 0) {
        zmused = 0;
        return i;
    }
    for (i = 0; i < 30; i++) l[i] = 5;
    bd = 5;
    if ((i = huft_build(l, 30, 0, cpdist, cpdext, &td, &bd)) > 1) {
        zmused = 0;
        return i;
    }
    if ((i = inflate_codes(tl, td, bl, bd)) != 0) return i < 0 ? i : 1;
    zmused = 0;
    return 0;
}

/* 0x80053344, size 0x71c */
static int inflate_dynamic(void)
{
    int i; /* r0 */
    unsigned int j; /* r26 */
    unsigned int l; /* r21 */
    unsigned int m; /* r25 */
    unsigned int n; /* r26 */
    huft *tl; /* stack */
    huft *td; /* stack */
    int bl; /* stack */
    int bd; /* stack */
    unsigned int nb; /* r21 */
    unsigned int nl; /* r23 */
    unsigned int nd; /* r22 */
    unsigned int *ll; /* r31 */
    unsigned int b; /* r29 */
    unsigned int k; /* r30 */
    ll = (unsigned int *)alloc_zip_mem(316 * sizeof(unsigned int));
    b = bb;
    k = bk;
    NEEDBITS(5); nl = 257 + (b & 31); DUMPBITS(5);
    NEEDBITS(5); nd = 1 + (b & 31); DUMPBITS(5);
    NEEDBITS(4); nb = 4 + (b & 15); DUMPBITS(4);
    if (nl > 286 || nd > 30) return 1;
    for (j = 0; j < nb; j++) { NEEDBITS(3); ll[border[j]] = b & 7; DUMPBITS(3); }
    for (; j < 19; j++) ll[border[j]] = 0;
    bl = 7;
    if ((i = huft_build(ll, 19, 19, 0, 0, &tl, &bl)) != 0) { zmused = 0; return i; }
    m = mask_bits[bl];
    n = nl + nd;
    i = l = 0;
    while ((unsigned)i < n) {
        NEEDBITS(bl);
        j = (td = tl + (b & m))->b;
        DUMPBITS(j);
        j = td->v.n;
        if (j < 16) ll[i++] = l = j;
        else if (j == 16) {
            NEEDBITS(2); j = 3 + (b & 3); DUMPBITS(2);
            if ((unsigned)i + j > n) return 1;
            while (j--) ll[i++] = l;
        } else if (j == 17) {
            NEEDBITS(3); j = 3 + (b & 7); DUMPBITS(3);
            if ((unsigned)i + j > n) return 1;
            while (j--) ll[i++] = 0;
            l = 0;
        } else {
            NEEDBITS(7); j = 11 + (b & 127); DUMPBITS(7);
            if ((unsigned)i + j > n) return 1;
            while (j--) ll[i++] = 0;
            l = 0;
        }
    }
    zmused = 316 * sizeof(unsigned int);
    bb = b;
    bk = k;
    bl = lbits;
    if ((i = huft_build(ll, nl, 257, cplens, cplext, &tl, &bl)) != 0) { zmused = 0; return i; }
    bd = dbits;
    if ((i = huft_build(ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0) { zmused = 0; return i; }
    if ((i = inflate_codes(tl, td, bl, bd)) != 0) return i < 0 ? i : 1;
    zmused = 0;
    return 0;
}

/* 0x80053a60, size 0x120 */
static int inflate_block(int *e)
{
    unsigned int t; /* r4 */
    unsigned int b; /* r31 */
    unsigned int k; /* r30 */
    b = bb;
    k = bk;
    NEEDBITS(1);
    *e = b & 1;
    DUMPBITS(1);
    NEEDBITS(2);
    t = b & 3;
    DUMPBITS(2);
    bb = b;
    bk = k;
    if (t == 2) return inflate_dynamic();
    if (t == 0) return inflate_stored();
    if (t == 1) return inflate_fixed();
    return 1;
}

/* 0x80053b80, size 0x50 */
static int inflate(void)
{
    int e; /* stack */
    bk = 0;
    bb = 0;
    do { if (inflate_block(&e)) return 1; } while (!e);
    return 0;
}

/* 0x80053bd0, size 0x8 */
static unsigned int get_buffer(void)
{
    return 0;
}

/* 0x80053bd8, size 0x58 */
unsigned int unzipdata(unsigned char *src, unsigned int srclen, unsigned char *dest)
{
    unzip_buffer = dest;
    incoming = src;
    last_incoming = src + srclen;
    init_zip_mem((unsigned char *)s_buf, sizeof(s_buf));
    inflate();
    return unzip_buffer - dest;
}
