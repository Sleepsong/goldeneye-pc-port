/*
 * PRNG — ported verbatim from src/random.s (MIPS assembly, not compiled for
 * the PC). This is a REAL implementation, not a stub: the game uses it for
 * genuine gameplay logic, so a no-op would silently corrupt behaviour.
 *
 *   - randomGetNextFrom() feeds the CRC in src/game/crc.c
 *   - g_randomSeed is persisted in replay state (src/game/ramromreplay.c)
 *   - randomGetNext() drives RANDOMFRAC()/RANDOMGETNEXT_F32() (bondconstants.h)
 *
 * Also ports src/game/chrObjRandom.s: the character-AI PRNG (chroid.c),
 * which runs the identical transform on its own seed variable.
 *
 * The seed is stored as a full u64 and the transform genuinely operates on
 * all 64 bits of the register (see the dsll32/dsrl32 note below) — the
 * stored seed is NOT truncated to 32 bits between calls, matching what the
 * real `sd $a0, g_randomSeed` in random.s stores. The initial seed is the
 * two .words in random.s: 0xAB8D9F77 (hi) / 0x81280783 (lo). Only the low
 * 32 bits are ever returned to callers (`dsll32 $v0,$a0,0; dsra32
 * $v0,$v0,0` sign-extends the low half into $v0, of which callers only use
 * the u32 truncation).
 *
 * D284 (M-140): the original port of this file had dsll32/dsrl32 backwards
 * — it shifted by the literal immediate `n` and masked to 32 bits, i.e. it
 * modelled them as 32-bit shift instructions. Real MIPS64 dsll32/dsrl32 are
 * "shift by n+32", operating on the FULL 64-bit register with no masking:
 * they exist specifically to encode a 32..63 shift amount in a 5-bit field.
 * The bug was bit-exact-but-wrong: every generated value differed from the
 * real N64 stream from the very first draw, silently desyncing loot RNG, AI
 * variance, and replay determinism in every release to date. Verified two
 * independent ways (a forked review agent's numeric trace, and a hand trace
 * of random.s against this file, both in the M-138 review session).
 */

#include <ultra64.h>
#include <random.h>

u64 g_randomSeed = 0xAB8D9F7781280783ULL;

/*
 * MIPS64 shift helpers, matching the .s instructions exactly:
 *   dsll32 rd,rs,n  ->  (rs << (n+32))   (full 64-bit register, no masking)
 *   dsrl32 rd,rs,n  ->  (rs >> (n+32))   (full 64-bit register, unsigned)
 * Plain `<<`/`>>` on u64 match dsll/dsrl (full 64-bit shifts, shift amount
 * used as-is, 0-31).
 */
static inline u64 dsll32(u64 x, int n) { return x << (n + 32); }
static inline u64 dsrl32(u64 x, int n) { return x >> (n + 32); }

/*
 * Advance the global seed and return the new low 32 bits.
 * Mirrors randomGetNext in random.s line-by-line.
 */
u32 randomGetNext(void)
{
    u64 a0 = g_randomSeed;
    u64 a1, a2;

    a2 = dsll32(a0, 0x1f);   /* dsll32 $a2, $a0, 0x1f */
    a1 = a0 << 0x1f;         /* dsll   $a1, $a0, 0x1f */
    a2 = a2 >> 0x1f;         /* dsrl   $a2, $a2, 0x1f */
    a1 = dsrl32(a1, 0);      /* dsrl32 $a1, $a1, 0    */
    a0 = dsll32(a0, 0xc);    /* dsll32 $a0, $a0, 0xc  */
    a2 = a2 | a1;            /* or     $a2, $a2, $a1  */
    a0 = dsrl32(a0, 0);      /* dsrl32 $a0, $a0, 0    */
    a2 = a2 ^ a0;            /* xor    $a2, $a2, $a0  */
    a0 = a2 >> 0x14;         /* dsrl   $a0, $a2, 0x14 */
    a0 = a0 & 0xfff;         /* andi   $a0, $a0, 0xfff*/
    a0 = a0 ^ a2;            /* xor    $a0, $a0, $a2  */

    g_randomSeed = a0;       /* sd     $a0, g_randomSeed */
    return (u32)a0;          /* v0 = a0 & 0xFFFFFFFF    */
}

/* Set the seed (the .s adds 1 before storing). */
void randomSetSeed(u32 seed)
{
    g_randomSeed = (u64)seed + 1;   /* daddiu $a0,$a0,1; sd $a0, g_randomSeed */
}

/*
 * Same transform as randomGetNext, but applied to *seed (a caller-owned u64)
 * instead of the global. Mirrors randomGetNextFrom in random.s.
 */
u32 randomGetNextFrom(u64 *seed)
{
    u64 a3 = *seed;          /* ld     $a3, ($a0) */
    u64 a1, a2;

    a2 = dsll32(a3, 0x1f);   /* dsll32 $a2, $a3, 0x1f */
    a1 = a3 << 0x1f;         /* dsll   $a1, $a3, 0x1f */
    a2 = a2 >> 0x1f;         /* dsrl   $a2, $a2, 0x1f */
    a1 = dsrl32(a1, 0);      /* dsrl32 $a1, $a1, 0    */
    a3 = dsll32(a3, 0xc);    /* dsll32 $a3, $a3, 0xc  */
    a2 = a2 | a1;            /* or     $a2, $a2, $a1  */
    a3 = dsrl32(a3, 0);      /* dsrl32 $a3, $a3, 0    */
    a2 = a2 ^ a3;            /* xor    $a2, $a2, $a3  */
    a3 = a2 >> 0x14;         /* dsrl   $a3, $a2, 0x14 */
    a3 = a3 & 0xfff;         /* andi   $a3, $a3, 0xfff*/
    a3 = a3 ^ a2;            /* xor    $a3, $a3, $a2  */

    *seed = a3;              /* sd     $a3, ($a0) */
    return (u32)a3;          /* v0 = a3 & 0xFFFFFFFF */
}

/* --- Character-object PRNG (src/game/chrObjRandom.s) ---------------------- */
/* Same xorshift as randomGetNext, separate state. Seed is two .words in the   */
/* .s: 0xAB8D9F77 (hi) / 0x81280783 (lo) — same initial value as g_randomSeed. */

u64 g_chrObjRandomSeed = 0xAB8D9F7781280783ULL;

u32 chrObjRandomGetNext(void)
{
    u64 a0 = g_chrObjRandomSeed;
    u64 a1, a2;

    a2 = dsll32(a0, 0x1f);   /* dsll32 $a2, $a0, 0x1f */
    a1 = a0 << 0x1f;         /* dsll   $a1, $a0, 0x1f */
    a2 = a2 >> 0x1f;         /* dsrl   $a2, $a2, 0x1f */
    a1 = dsrl32(a1, 0);      /* dsrl32 $a1, $a1, 0    */
    a0 = dsll32(a0, 0xc);    /* dsll32 $a0, $a0, 0xc  */
    a2 = a2 | a1;            /* or     $a2, $a2, $a1  */
    a0 = dsrl32(a0, 0);      /* dsrl32 $a0, $a0, 0    */
    a2 = a2 ^ a0;            /* xor    $a2, $a2, $a0  */
    a0 = a2 >> 0x14;         /* dsrl   $a0, $a2, 0x14 */
    a0 = a0 & 0xfff;         /* andi   $a0, $a0, 0xfff*/
    a0 = a0 ^ a2;            /* xor    $a0, $a0, $a2  */

    g_chrObjRandomSeed = a0; /* sd     $a0, g_chrObjRandomSeed */
    return (u32)a0;          /* v0 = a0 & 0xFFFFFFFF            */
}

void chrObjRandomSetSeed(u32 seed)
{
    g_chrObjRandomSeed = (u64)seed + 1;   /* daddiu $a0,$a0,1; sd $a0, ... */
}
