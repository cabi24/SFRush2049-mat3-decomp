/* zip.c - zlib deflate (trees.c + deflate.c) in-memory compressor. Status: Matching (24/24 functions).
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\zip.c; assembly: build/2049LE/asm/GameSource/src/zip.s */
#include "rush/rush_protos.h"

/* zlib (trees.c / deflate.c) constants and macros, as in zlib 1.1.x without ZLIB_DEBUG */
#define MAX_BITS 15
#define LENGTH_CODES 29
#define LITERALS 256
#define L_CODES (LITERALS + 1 + LENGTH_CODES)
#define D_CODES 30
#define BL_CODES 19
#define HEAP_SIZE (2 * L_CODES + 1)
#define END_BLOCK 256
#define REP_3_6 16
#define REPZ_3_10 17
#define REPZ_11_138 18
#define Buf_size (8 * 2 * sizeof(char))
#define MIN_MATCH 3
#define MAX_MATCH 258
#define MIN_LOOKAHEAD (MAX_MATCH + MIN_MATCH + 1)
#define MAX_DIST(s) ((s)->w_size - MIN_LOOKAHEAD)
#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES 2
#define TOO_FAR 4096
#define NIL 0

#define Freq fc.freq
#define Code fc.code
#define Dad dl.dad
#define Len dl.len

#define put_byte(s, c) { (s)->pending_buf[(s)->pending++] = (c); }
#define put_short(s, w) { \
    put_byte(s, (unsigned char)((w) & 0xff)); \
    put_byte(s, (unsigned char)((unsigned short)(w) >> 8)); \
}
#define send_bits(s, value, length) \
{ int len = length; \
  if (s->bi_valid > (int)Buf_size - len) { \
    int val = value; \
    s->bi_buf |= (val << s->bi_valid); \
    put_short(s, s->bi_buf); \
    s->bi_buf = (unsigned short)val >> (Buf_size - s->bi_valid); \
    s->bi_valid += len - Buf_size; \
  } else { \
    s->bi_buf |= (value) << s->bi_valid; \
    s->bi_valid += len; \
  } \
}
#define send_code(s, c, tree) send_bits(s, tree[c].Code, tree[c].Len)
#define d_code(dist) ((dist) < 256 ? dist_code[dist] : dist_code[256 + ((dist) >> 7)])
#define pqremove(s, tree, top) \
{ \
    top = s->heap[1]; \
    s->heap[1] = s->heap[s->heap_len--]; \
    pqdownheap(s, tree, 1); \
}
#define smaller(tree, n, m, depth) \
   (tree[n].Freq < tree[m].Freq || \
   (tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))
#define UPDATE_HASH(s, h, c) (h = (((h) << s->hash_shift) ^ (c)) & s->hash_mask)
#define INSERT_STRING(s, str, match_head) \
   (UPDATE_HASH(s, s->ins_h, s->window[(str) + (MIN_MATCH - 1)]), \
    s->prev[(str) & s->w_mask] = match_head = s->head[s->ins_h], \
    s->head[s->ins_h] = (unsigned short)(str))
#define CLEAR_HASH(s) \
    s->head[s->hash_size - 1] = NIL; \
    memset((char *)s->head, 0, (unsigned)(s->hash_size - 1) * sizeof(*s->head));

#define FLUSH_BLOCK_ONLY(s, eof) { \
    _tr_flush_block(s, (s->block_start >= 0L ? (char *)&s->window[(unsigned)s->block_start] : (char *)0), \
                    (unsigned)((long)s->strstart - s->block_start), (eof)); \
    s->block_start = s->strstart; \
    flush_pending(s->strm); \
}
#define FLUSH_BLOCK(s, eof) { \
    FLUSH_BLOCK_ONLY(s, eof); \
    if (s->strm->avail_out == 0) return (eof) ? 2 : 0; \
}

static void tr_static_init(void);
static void init_block(internal_state *s);
static void pqdownheap(internal_state *s, ct_data_s *tree, int k);
static void gen_bitlen(internal_state *s, tree_desc_s *desc);
static void gen_codes(ct_data_s *tree, int max_code, unsigned short *bl_count);
static void build_tree(internal_state *s, tree_desc_s *desc);
static void scan_tree(internal_state *s, ct_data_s *tree, int max_code);
static void send_tree(internal_state *s, ct_data_s *tree, int max_code);
static int build_bl_tree(internal_state *s);
static void send_all_trees(internal_state *s, int lcodes, int dcodes, int blcodes);
static void compress_block(internal_state *s, ct_data_s *ltree, ct_data_s *dtree);
static unsigned int bi_reverse(unsigned int code, int len);
static void bi_flush(internal_state *s);
static void bi_windup(internal_state *s);
static void copy_block(internal_state *s, char *buf, unsigned int len, int header);
static void flush_pending(z_stream_s *strm);
static int read_buf(z_stream_s *strm, char *buf, unsigned int size);
static unsigned int longest_match(internal_state *s, unsigned short cur_match);
static void fill_window(internal_state *s);

static config_s configuration_table[10] = {
    4, 4, 0x10, 0x10, 4, 4, 0x10, 0x10, 4, 4, 0x10, 0x10, 4, 4, 0x10, 0x10, 4, 4, 0x10, 0x10, 8,
    0x10, 0x20, 0x20, 8, 0x10, 0x80, 0x80, 8, 0x20, 0x80, 0x100, 0x20, 0x80, 0x102, 0x400, 0x20,
    0x102, 0x102, 0x1000
};
static int extra_lbits[29] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};
static int extra_dbits[30] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13,
    13
};
static int extra_blbits[19] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 7 };
static unsigned char bl_order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
static ct_data_s static_ltree[288];
static ct_data_s static_dtree[30];
static unsigned char dist_code[512];
static unsigned char length_code[256];
static int base_length[29];
static int base_dist[30];
static static_tree_desc_s static_l_desc = { (void *)&static_ltree, (void *)&extra_lbits, 257, 286, 15 };
static static_tree_desc_s static_d_desc = { (void *)&static_dtree, (void *)&extra_dbits, 0, 30, 15 };
static static_tree_desc_s static_bl_desc = { 0, (void *)&extra_blbits, 0, 19, 7 };
static char debug_info[1024];

/* 0x8005ecc4, size 0x5f0 */
static void tr_static_init(void)
{
    static int static_init_done = 0;
    int n; /* r7 */
    int bits; /* r0 */
    int length; /* r8 */
    int code; /* r9 */
    int dist; /* r5 */
    unsigned short bl_count[16];

    if (static_init_done) return;

    length = 0;
    for (code = 0; code < LENGTH_CODES - 1; code++) {
        base_length[code] = length;
        for (n = 0; n < (1 << extra_lbits[code]); n++) {
            length_code[length++] = (unsigned char)code;
        }
    }
    length_code[length - 1] = (unsigned char)code;

    dist = 0;
    for (code = 0; code < 16; code++) {
        base_dist[code] = dist;
        for (n = 0; n < (1 << extra_dbits[code]); n++) {
            dist_code[dist++] = (unsigned char)code;
        }
    }
    dist >>= 7;
    for (; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        for (n = 0; n < (1 << (extra_dbits[code] - 7)); n++) {
            dist_code[256 + dist++] = (unsigned char)code;
        }
    }

    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
    n = 0;
    while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
    while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
    while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
    while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
    gen_codes(static_ltree, L_CODES + 1, bl_count);

    for (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse((unsigned int)n, 5);
    }
    static_init_done = 1;
}

/* 0x8005f2b4, size 0x8c */
void _tr_init(internal_state *s)
{
    tr_static_init();

    s->compressed_len = 0;

    s->l_desc.dyn_tree = s->dyn_ltree;
    s->l_desc.stat_desc = &static_l_desc;

    s->d_desc.dyn_tree = s->dyn_dtree;
    s->d_desc.stat_desc = &static_d_desc;

    s->bl_desc.dyn_tree = s->bl_tree;
    s->bl_desc.stat_desc = &static_bl_desc;

    s->bi_buf = 0;
    s->bi_valid = 0;
    s->last_eob_len = 8;

    init_block(s);
}

/* 0x8005f340, size 0x234 */
static void init_block(internal_state *s)
{
    int n; /* r6 */

    for (n = 0; n < L_CODES; n++) s->dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES; n++) s->dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n++) s->bl_tree[n].Freq = 0;

    s->dyn_ltree[END_BLOCK].Freq = 1;
    s->opt_len = s->static_len = 0;
    s->last_lit = s->matches = 0;
}

/* 0x8005f574, size 0xd4 */
static void pqdownheap(internal_state *s, ct_data_s *tree, int k)
{
    int v = s->heap[k]; /* r6 */
    int j = k << 1; /* r7 */

    while (j <= s->heap_len) {
        if (j < s->heap_len && smaller(tree, s->heap[j + 1], s->heap[j], s->depth)) {
            j++;
        }
        if (smaller(tree, v, s->heap[j], s->depth)) break;
        s->heap[k] = s->heap[j];
        k = j;
        j <<= 1;
    }
    s->heap[k] = v;
}

/* 0x8005f648, size 0x248 */
static void gen_bitlen(internal_state *s, tree_desc_s *desc)
{
    ct_data_s *tree = desc->dyn_tree; /* r5 */
    int max_code = desc->max_code; /* r0 */
    ct_data_s *stree = desc->stat_desc->static_tree; /* r6 */
    int *extra = desc->stat_desc->extra_bits; /* r7 */
    int base = desc->stat_desc->extra_base; /* r8 */
    int max_length = desc->stat_desc->max_length; /* r9 */
    int h; /* r10 */
    int n; /* r27 */
    int m; /* r6 */
    int bits; /* r0 */
    int xbits; /* r26 */
    unsigned short f;
    int overflow = 0; /* r11 */

    for (bits = 0; bits <= MAX_BITS; bits++) s->bl_count[bits] = 0;

    tree[s->heap[s->heap_max]].Len = 0;

    for (h = s->heap_max + 1; h < HEAP_SIZE; h++) {
        n = s->heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) bits = max_length, overflow++;
        tree[n].Len = (unsigned short)bits;

        if (n > max_code) continue;

        s->bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n - base];
        f = tree[n].Freq;
        s->opt_len += (unsigned int)f * (bits + xbits);
        if (stree) s->static_len += (unsigned int)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    do {
        bits = max_length - 1;
        while (s->bl_count[bits] == 0) bits--;
        s->bl_count[bits]--;
        s->bl_count[bits + 1] += 2;
        s->bl_count[max_length]--;
        overflow -= 2;
    } while (overflow > 0);

    for (bits = max_length; bits != 0; bits--) {
        n = s->bl_count[bits];
        while (n != 0) {
            m = s->heap[--h];
            if (m > max_code) continue;
            if (tree[m].Len != (unsigned int)bits) {
                s->opt_len += ((int)bits - (int)tree[m].Len) * (int)tree[m].Freq;
                tree[m].Len = (unsigned short)bits;
            }
            n--;
        }
    }
}

/* 0x8005f890, size 0xf8 */
static void gen_codes(ct_data_s *tree, int max_code, unsigned short *bl_count)
{
    unsigned short next_code[16]; /* stack */
    unsigned short code = 0; /* r8 */
    int bits; /* r0 */
    int n; /* r28 */

    for (bits = 1; bits <= MAX_BITS; bits++) {
        next_code[bits] = code = (code + bl_count[bits - 1]) << 1;
    }

    for (n = 0; n <= max_code; n++) {
        int len = tree[n].Len; /* r4 */
        if (len == 0) continue;
        tree[n].Code = bi_reverse(next_code[len]++, len);
    }
}

/* 0x8005f988, size 0x278 */
static void build_tree(internal_state *s, tree_desc_s *desc)
{
    ct_data_s *tree = desc->dyn_tree; /* r28 */
    ct_data_s *stree = desc->stat_desc->static_tree; /* r3 */
    int elems = desc->stat_desc->elems; /* r29 */
    int n; /* r8 */
    int m; /* r8 */
    int max_code = -1; /* r27 */
    int node; /* r0 */

    s->heap_len = 0, s->heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            s->heap[++(s->heap_len)] = max_code = n;
            s->depth[n] = 0;
        } else {
            tree[n].Len = 0;
        }
    }

    while (s->heap_len < 2) {
        node = s->heap[++(s->heap_len)] = (max_code < 2 ? ++max_code : 0);
        tree[node].Freq = 1;
        s->depth[node] = 0;
        s->opt_len--;
        if (stree) s->static_len -= stree[node].Len;
    }
    desc->max_code = max_code;

    for (n = s->heap_len / 2; n >= 1; n--) pqdownheap(s, tree, n);

    node = elems;
    do {
        pqremove(s, tree, n);
        m = s->heap[1];

        s->heap[--(s->heap_max)] = n;
        s->heap[--(s->heap_max)] = m;

        tree[node].Freq = tree[n].Freq + tree[m].Freq;
        s->depth[node] = (unsigned char)((s->depth[n] >= s->depth[m] ? s->depth[n] : s->depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (unsigned short)node;
        s->heap[1] = node++;
        pqdownheap(s, tree, 1);
    } while (s->heap_len >= 2);

    s->heap[--(s->heap_max)] = s->heap[1];

    gen_bitlen(s, desc);
    gen_codes(tree, max_code, s->bl_count);
}

/* 0x8005fc00, size 0x120 */
static void scan_tree(internal_state *s, ct_data_s *tree, int max_code)
{
    int n; /* r0 */
    int prevlen = -1; /* r8 */
    int curlen; /* r7 */
    int nextlen = tree[0].Len; /* r9 */
    int count = 0; /* r10 */
    int max_count = 7; /* r11 */
    int min_count = 4; /* r12 */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code + 1].Len = (unsigned short)0xffff;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen;
        nextlen = tree[n + 1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            s->bl_tree[curlen].Freq += count;
        } else if (curlen != 0) {
            if (curlen != prevlen) s->bl_tree[curlen].Freq++;
            s->bl_tree[REP_3_6].Freq++;
        } else if (count <= 10) {
            s->bl_tree[REPZ_3_10].Freq++;
        } else {
            s->bl_tree[REPZ_11_138].Freq++;
        }
        count = 0;
        prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* 0x8005fd20, size 0x5d4 */
static void send_tree(internal_state *s, ct_data_s *tree, int max_code)
{
    int n; /* r0 */
    int prevlen = -1; /* r8 */
    int curlen; /* r0 */
    int nextlen = tree[0].Len; /* r6 */
    int count = 0; /* r7 */
    int max_count = 7; /* r9 */
    int min_count = 4; /* r10 */

    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen;
        nextlen = tree[n + 1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { send_code(s, curlen, s->bl_tree); } while (--count != 0);
        } else if (curlen != 0) {
            if (curlen != prevlen) {
                send_code(s, curlen, s->bl_tree);
                count--;
            }
            send_code(s, REP_3_6, s->bl_tree);
            send_bits(s, count - 3, 2);
        } else if (count <= 10) {
            send_code(s, REPZ_3_10, s->bl_tree);
            send_bits(s, count - 3, 3);
        } else {
            send_code(s, REPZ_11_138, s->bl_tree);
            send_bits(s, count - 11, 7);
        }
        count = 0;
        prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* 0x800602f4, size 0x168 */
static int build_bl_tree(internal_state *s)
{
    int max_blindex; /* r3 */

    scan_tree(s, s->dyn_ltree, s->l_desc.max_code);
    scan_tree(s, s->dyn_dtree, s->d_desc.max_code);

    build_tree(s, &s->bl_desc);

    for (max_blindex = BL_CODES - 1; max_blindex >= 3; max_blindex--) {
        if (s->bl_tree[bl_order[max_blindex]].Len != 0) break;
    }
    s->opt_len += 3 * (max_blindex + 1) + 5 + 5 + 4;

    return max_blindex;
}

/* 0x8006045c, size 0x2e8 */
static void send_all_trees(internal_state *s, int lcodes, int dcodes, int blcodes)
{
    int rank; /* r0 */

    send_bits(s, lcodes - 257, 5);
    send_bits(s, dcodes - 1, 5);
    send_bits(s, blcodes - 4, 4);
    for (rank = 0; rank < blcodes; rank++) {
        send_bits(s, s->bl_tree[bl_order[rank]].Len, 3);
    }

    send_tree(s, s->dyn_ltree, lcodes - 1);
    send_tree(s, s->dyn_dtree, dcodes - 1);
}

/* 0x80060744, size 0xd8 */
void _tr_stored_block(internal_state *s, char *buf, unsigned int stored_len, int eof)
{
    send_bits(s, (STORED_BLOCK << 1) + eof, 3);
    s->compressed_len = (s->compressed_len + 3 + 7) & ~7L;
    s->compressed_len += (stored_len + 4) << 3;
    copy_block(s, buf, stored_len, 1);
}

/* 0x8006081c, size 0x2ac */
unsigned int _tr_flush_block(internal_state *s, char *buf, unsigned int stored_len, int eof)
{
    unsigned int opt_lenb; /* r3 */
    unsigned int static_lenb; /* r5 */
    int max_blindex = 0; /* r4 */

    if (s->level > 0) {
        build_tree(s, &s->l_desc);
        build_tree(s, &s->d_desc);
        max_blindex = build_bl_tree(s);
        opt_lenb = (s->opt_len + 3 + 7) >> 3;
        static_lenb = (s->static_len + 3 + 7) >> 3;
        if (static_lenb <= opt_lenb) opt_lenb = static_lenb;
    } else {
        opt_lenb = static_lenb = stored_len + 5;
    }
    if (stored_len + 4 <= opt_lenb && buf != (char *)0) {
        _tr_stored_block(s, buf, stored_len, eof);
    } else if (static_lenb == opt_lenb) {
        send_bits(s, (STATIC_TREES << 1) + eof, 3);
        compress_block(s, static_ltree, static_dtree);
        s->compressed_len += 3 + s->static_len;
    } else {
        send_bits(s, (DYN_TREES << 1) + eof, 3);
        send_all_trees(s, s->l_desc.max_code + 1, s->d_desc.max_code + 1, max_blindex + 1);
        compress_block(s, s->dyn_ltree, s->dyn_dtree);
        s->compressed_len += 3 + s->opt_len;
    }
    init_block(s);
    if (eof) {
        bi_windup(s);
        s->compressed_len += 7;
    }
    return s->compressed_len >> 3;
}

/* 0x80060ac8, size 0x1cc */
int _tr_tally(internal_state *s, unsigned int dist, unsigned int lc)
{
    unsigned int out_length; /* r6 */
    unsigned int in_length; /* r7 */
    int dcode; /* r0 */

    s->d_buf[s->last_lit] = (unsigned short)dist;
    s->l_buf[s->last_lit++] = (unsigned char)lc;
    if (dist == 0) {
        s->dyn_ltree[lc].Freq++;
    } else {
        s->matches++;
        dist--;
        s->dyn_ltree[length_code[lc] + LITERALS + 1].Freq++;
        s->dyn_dtree[d_code(dist)].Freq++;
    }
    if (s->level > 2 && (s->last_lit & 0xfff) == 0) {
        out_length = s->last_lit * 8L;
        in_length = s->strstart - s->block_start;
        for (dcode = 0; dcode < D_CODES; dcode++) {
            out_length += s->dyn_dtree[dcode].Freq * (5L + extra_dbits[dcode]);
        }
        out_length >>= 3;
        if (s->matches < s->last_lit / 2 && out_length < in_length / 2) return 1;
    }
    return s->last_lit == s->lit_bufsize - 1;
}

/* 0x80060c94, size 0x4b8 */
static void compress_block(internal_state *s, ct_data_s *ltree, ct_data_s *dtree)
{
    unsigned int dist; /* r9 */
    int lc; /* r8 */
    unsigned int lx = 0; /* r10 */
    unsigned int code;
    int extra; /* r11 */

    if (s->last_lit != 0) do {
        dist = s->d_buf[lx];
        lc = s->l_buf[lx++];
        if (dist == 0) {
            send_code(s, lc, ltree);
        } else {
            code = length_code[lc];
            send_code(s, code + LITERALS + 1, ltree);
            extra = extra_lbits[code];
            if (extra != 0) {
                lc -= base_length[code];
                send_bits(s, lc, extra);
            }
            dist--;
            code = d_code(dist);
            send_code(s, code, dtree);
            extra = extra_dbits[code];
            if (extra != 0) {
                dist -= base_dist[code];
                send_bits(s, dist, extra);
            }
        }
    } while (lx < s->last_lit);

    send_code(s, END_BLOCK, ltree);
    s->last_eob_len = ltree[END_BLOCK].Len;
}

/* 0x8006114c, size 0x24 */
static unsigned int bi_reverse(unsigned int code, int len)
{
    unsigned int res = 0; /* r5 */

    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

/* stripped by the linker (no data) */
static void bi_flush(internal_state *s)
{
    if (s->bi_valid == 16) {
        put_short(s, s->bi_buf);
        s->bi_buf = 0;
        s->bi_valid = 0;
    } else if (s->bi_valid >= 8) {
        put_byte(s, (unsigned char)s->bi_buf);
        s->bi_buf >>= 8;
        s->bi_valid -= 8;
    }
}

/* 0x80061170, size 0x74 */
static void bi_windup(internal_state *s)
{
    if (s->bi_valid > 8) {
        put_short(s, s->bi_buf);
    } else if (s->bi_valid > 0) {
        put_byte(s, (unsigned char)s->bi_buf);
    }
    s->bi_buf = 0;
    s->bi_valid = 0;
}

/* 0x800611e4, size 0x168 */
static void copy_block(internal_state *s, char *buf, unsigned int len, int header)
{
    bi_windup(s);
    s->last_eob_len = 8;
    if (header) {
        put_short(s, (unsigned short)len);
        put_short(s, (unsigned short)~len);
    }
    while (len--) {
        put_byte(s, *buf++);
    }
}

/* 0x8006134c, size 0x26c */
unsigned int zip(unsigned char *mem, unsigned int len, unsigned char *compr, unsigned int comprLen, int level, int winbits, int memlevel)
{
    z_stream_s c_stream; /* stack */
    internal_state *s; /* r0 */
    unsigned short *overlay; /* r0 */

    c_stream.next_in = mem;
    c_stream.avail_in = len;
    c_stream.next_out = compr;
    c_stream.avail_out = comprLen;
    s = AllocateMemoryHeapHigh(0, sizeof(internal_state));
    c_stream.total_in = 0;
    c_stream.total_out = 0;
    c_stream.state = s;
    s->strm = &c_stream;
    s->w_bits = winbits;
    s->w_size = 1 << s->w_bits;
    s->w_mask = s->w_size - 1;
    s->hash_bits = memlevel + 7;
    s->hash_size = 1 << s->hash_bits;
    s->hash_mask = s->hash_size - 1;
    s->hash_shift = (s->hash_bits + MIN_MATCH - 1) / MIN_MATCH;
    s->window = AllocateMemoryHeapHigh(0, s->w_size * 2);
    s->prev = AllocateMemoryHeapHigh(0, s->w_size * 2);
    s->head = AllocateMemoryHeapHigh(0, s->hash_size * 2);
    s->lit_bufsize = 1 << (memlevel + 6);
    overlay = AllocateMemoryHeapHigh(0, s->lit_bufsize * 4);
    s->pending_buf = (unsigned char *)overlay;
    s->d_buf = overlay + s->lit_bufsize / 2;
    s->l_buf = s->pending_buf + 3 * s->lit_bufsize;
    s->level = level;
    s->good_match = configuration_table[level].good_length;
    s->max_lazy_match = configuration_table[level].max_lazy;
    s->nice_match = configuration_table[level].nice_length;
    s->max_chain_length = configuration_table[level].max_chain;

    s = c_stream.state;
    s->pending = 0;
    s->pending_out = s->pending_buf;
    _tr_init(s);

    s->window_size = 2 * s->w_size;
    CLEAR_HASH(s);
    s->strstart = 0;
    s->block_start = 0;
    s->lookahead = 0;
    s->match_length = s->prev_length = MIN_MATCH - 1;
    s->match_available = 0;
    s->ins_h = 0;

    deflate_slow(c_stream.state);

    if (s->pending_buf) appFree(s->pending_buf);
    if (s->head) appFree(s->head);
    if (s->prev) appFree(s->prev);
    if (s->window) appFree(s->window);
    if (s) appFree(s);
    return c_stream.total_out;
}

/* 0x800615b8, size 0xbc */
static void flush_pending(z_stream_s *strm)
{
    unsigned int len = strm->state->pending; /* r31 */

    if (len > strm->avail_out) len = strm->avail_out;
    if (len == 0) return;
    memcpy(strm->next_out, strm->state->pending_out, len);
    strm->next_out += len;
    strm->state->pending_out += len;
    strm->total_out += len;
    strm->avail_out -= len;
    strm->state->pending -= len;
    if (strm->state->pending == 0) {
        strm->state->pending_out = strm->state->pending_buf;
    }
}

/* 0x80061674, size 0x88 */
static int read_buf(z_stream_s *strm, char *buf, unsigned int size)
{
    unsigned int len = strm->avail_in; /* r31 */

    if (len > size) len = size;
    if (len == 0) return 0;
    strm->avail_in -= len;
    memcpy(buf, strm->next_in, len);
    strm->next_in += len;
    strm->total_in += len;
    return len;
}

/* 0x800616fc, size 0x1d0 */
static unsigned int longest_match(internal_state *s, unsigned short cur_match)
{
    unsigned int chain_length = s->max_chain_length; /* r5 */
    unsigned char *scan = s->window + s->strstart; /* r6 */
    unsigned char *match; /* r7 */
    int len; /* r7 */
    int best_len = s->prev_length; /* r0 */
    int nice_match = s->nice_match; /* r8 */
    unsigned short limit = s->strstart > (unsigned short)MAX_DIST(s) ? s->strstart - (unsigned short)MAX_DIST(s) : NIL; /* r9 */
    unsigned short *prev = s->prev; /* r10 */
    unsigned int wmask = s->w_mask; /* r11 */
    unsigned char *strend = s->window + s->strstart + MAX_MATCH; /* r12 */
    unsigned char scan_end1 = scan[best_len - 1]; /* r31 */
    unsigned char scan_end = scan[best_len]; /* r30 */

    if (s->prev_length >= s->good_match) {
        chain_length >>= 2;
    }
    if ((unsigned int)nice_match > s->lookahead) nice_match = s->lookahead;

    do {
        match = s->window + cur_match;
        if (match[best_len] != scan_end || match[best_len - 1] != scan_end1 || *match != *scan ||
            *++match != scan[1])
            continue;
        scan += 2, match++;
        do {
        } while (*++scan == *++match && *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match && scan < strend);
        len = MAX_MATCH - (int)(strend - scan);
        scan = strend - MAX_MATCH;
        if (len > best_len) {
            s->match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
            scan_end1 = scan[best_len - 1];
            scan_end = scan[best_len];
        }
    } while ((cur_match = prev[cur_match & wmask]) > limit && --chain_length != 0);

    if ((unsigned int)best_len <= s->lookahead) return best_len;
    return s->lookahead;
}

/* 0x800618cc, size 0x1d8 */
static void fill_window(internal_state *s)
{
    unsigned int n; /* r4 */
    unsigned int m; /* r3 */
    unsigned short *p; /* r5 */
    unsigned int more; /* r29 */
    unsigned int wsize = s->w_size; /* r31 */

    do {
        more = s->window_size - s->lookahead - s->strstart;
        if (more == 0 && s->strstart == 0 && s->lookahead == 0) {
            more = wsize;
        } else if (more == (unsigned int)-1) {
            more--;
        } else if (s->strstart >= wsize + MAX_DIST(s)) {
            memcpy(s->window, s->window + wsize, wsize);
            s->match_start -= wsize;
            s->strstart -= wsize;
            s->block_start -= wsize;
            n = s->hash_size;
            p = &s->head[n];
            do {
                m = *--p;
                *p = (unsigned short)(m >= wsize ? m - wsize : NIL);
            } while (--n);
            n = wsize;
            p = &s->prev[n];
            do {
                m = *--p;
                *p = (unsigned short)(m >= wsize ? m - wsize : NIL);
            } while (--n);
            more += wsize;
        }
        if (s->strm->avail_in == 0) return;
        n = read_buf(s->strm, (char *)s->window + s->strstart + s->lookahead, more);
        s->lookahead += n;
        if (s->lookahead >= MIN_MATCH) {
            s->ins_h = s->window[s->strstart];
            UPDATE_HASH(s, s->ins_h, s->window[s->strstart + 1]);
        }
    } while (s->lookahead < MIN_LOOKAHEAD && s->strm->avail_in != 0);
}

/* 0x80061aa4, size 0x41c */
int deflate_slow(internal_state *s)
{
    unsigned short hash_head = NIL; /* r31 */
    int bflush; /* r0 */
    unsigned int max_insert; /* r30 */

    for (;;) {
        if (s->lookahead < MIN_LOOKAHEAD) {
            fill_window(s);
            if (s->lookahead == 0) break;
        }
        if (s->lookahead >= MIN_MATCH) {
            INSERT_STRING(s, s->strstart, hash_head);
        }
        s->prev_length = s->match_length, s->prev_match = s->match_start;
        s->match_length = MIN_MATCH - 1;
        if (hash_head != NIL && s->prev_length < s->max_lazy_match &&
            s->strstart - hash_head <= MAX_DIST(s)) {
            s->match_length = longest_match(s, hash_head);
            if (s->match_length <= 5 &&
                (s->match_length == MIN_MATCH && s->strstart - s->match_start > TOO_FAR)) {
                s->match_length = MIN_MATCH - 1;
            }
        }
        if (s->prev_length >= MIN_MATCH && s->match_length <= s->prev_length) {
            max_insert = s->strstart + s->lookahead - MIN_MATCH;
            bflush = _tr_tally(s, s->strstart - 1 - s->prev_match, s->prev_length - MIN_MATCH);
            s->lookahead -= s->prev_length - 1;
            s->prev_length -= 2;
            do {
                if (++s->strstart <= max_insert) {
                    INSERT_STRING(s, s->strstart, hash_head);
                }
            } while (--s->prev_length != 0);
            s->match_available = 0;
            s->match_length = MIN_MATCH - 1;
            s->strstart++;
            if (bflush) FLUSH_BLOCK(s, 0);
        } else if (s->match_available) {
            bflush = _tr_tally(s, 0, s->window[s->strstart - 1]);
            if (bflush) {
                FLUSH_BLOCK_ONLY(s, 0);
            }
            s->strstart++;
            s->lookahead--;
            if (s->strm->avail_out == 0) return 0;
        } else {
            s->match_available = 1;
            s->strstart++;
            s->lookahead--;
        }
    }
    if (s->match_available) {
        _tr_tally(s, 0, s->window[s->strstart - 1]);
        s->match_available = 0;
    }
    FLUSH_BLOCK(s, 1);
    return 1;
}

