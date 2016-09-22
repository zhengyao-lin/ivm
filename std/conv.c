/*
	The 'ivm_conv_dtoa' and its related functions are based on the original version of fpconv by Andreas Samoljuk

	The MIT License

	Copyright (c) 2013 Andreas Samoljuk

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
	LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdint.h>

#include "pub/type.h"
#include "pub/const.h"
#include "pub/com.h"

#include "conv.h"

#define NPOWERS			87
#define STEPPOWERS		8
#define FIRSTPOWER		-348 /* 10 ^ -348 */

#define EXPMAX			-32
#define EXPMIN			-60

typedef struct fp_t {
	ivm_uint64_t frac;
	ivm_int_t exp;
} fp_t;

static fp_t powers_ten[] = {
	{ 18054884314459144840U, -1220 }, { 13451937075301367670U, -1193 },
	{ 10022474136428063862U, -1166 }, { 14934650266808366570U, -1140 },
	{ 11127181549972568877U, -1113 }, { 16580792590934885855U, -1087 },
	{ 12353653155963782858U, -1060 }, { 18408377700990114895U, -1034 },
	{ 13715310171984221708U, -1007 }, { 10218702384817765436U, -980 },
	{ 15227053142812498563U, -954 },  { 11345038669416679861U, -927 },
	{ 16905424996341287883U, -901 },  { 12595523146049147757U, -874 },
	{ 9384396036005875287U,  -847 },  { 13983839803942852151U, -821 },
	{ 10418772551374772303U, -794 },  { 15525180923007089351U, -768 },
	{ 11567161174868858868U, -741 },  { 17236413322193710309U, -715 },
	{ 12842128665889583758U, -688 },  { 9568131466127621947U,  -661 },
	{ 14257626930069360058U, -635 },  { 10622759856335341974U, -608 },
	{ 15829145694278690180U, -582 },  { 11793632577567316726U, -555 },
	{ 17573882009934360870U, -529 },  { 13093562431584567480U, -502 },
	{ 9755464219737475723U,  -475 },  { 14536774485912137811U, -449 },
	{ 10830740992659433045U, -422 },  { 16139061738043178685U, -396 },
	{ 12024538023802026127U, -369 },  { 17917957937422433684U, -343 },
	{ 13349918974505688015U, -316 },  { 9946464728195732843U,  -289 },
	{ 14821387422376473014U, -263 },  { 11042794154864902060U, -236 },
	{ 16455045573212060422U, -210 },  { 12259964326927110867U, -183 },
	{ 18268770466636286478U, -157 },  { 13611294676837538539U, -130 },
	{ 10141204801825835212U, -103 },  { 15111572745182864684U, -77 },
	{ 11258999068426240000U, -50 },   { 16777216000000000000U, -24 },
	{ 12500000000000000000U,   3 },   { 9313225746154785156U,   30 },
	{ 13877787807814456755U,  56 },   { 10339757656912845936U,  83 },
	{ 15407439555097886824U, 109 },   { 11479437019748901445U, 136 },
	{ 17105694144590052135U, 162 },   { 12744735289059618216U, 189 },
	{ 9495567745759798747U,  216 },   { 14149498560666738074U, 242 },
	{ 10542197943230523224U, 269 },   { 15709099088952724970U, 295 },
	{ 11704190886730495818U, 322 },   { 17440603504673385349U, 348 },
	{ 12994262207056124023U, 375 },   { 9681479787123295682U,  402 },
	{ 14426529090290212157U, 428 },   { 10748601772107342003U, 455 },
	{ 16016664761464807395U, 481 },   { 11933345169920330789U, 508 },
	{ 17782069995880619868U, 534 },   { 13248674568444952270U, 561 },
	{ 9871031767461413346U,  588 },   { 14708983551653345445U, 614 },
	{ 10959046745042015199U, 641 },   { 16330252207878254650U, 667 },
	{ 12166986024289022870U, 694 },   { 18130221999122236476U, 720 },
	{ 13508068024458167312U, 747 },   { 10064294952495520794U, 774 },
	{ 14996968138956309548U, 800 },   { 11173611982879273257U, 827 },
	{ 16649979327439178909U, 853 },   { 12405201291620119593U, 880 },
	{ 9242595204427927429U,  907 },   { 13772540099066387757U, 933 },
	{ 10261342003245940623U, 960 },   { 15290591125556738113U, 986 },
	{ 11392378155556871081U, 1013 },  { 16975966327722178521U, 1039 },
	{ 12648080533535911531U, 1066 }
};

IVM_PRIVATE
fp_t
_find_cachedpow10(ivm_int_t exp, ivm_int_t *k)
{
	const ivm_double_t one_log_ten = 0.30102999566398114;

	ivm_int_t approx = -(exp + NPOWERS) * one_log_ten;
	ivm_int_t idx = (approx - FIRSTPOWER) / STEPPOWERS;

	while (1) {
		ivm_int_t current = exp + powers_ten[idx].exp + 64;

		if (current < EXPMIN) {
			idx++;
			continue;
		}

		if (current > EXPMAX) {
			idx--;
			continue;
		}

		*k = (FIRSTPOWER + idx * STEPPOWERS);

		return powers_ten[idx];
	}
}

#define FRACMASK	0x000FFFFFFFFFFFFFU
#define EXPMASK		0x7FF0000000000000U
#define HIDDENBIT	0x0010000000000000U
#define SIGNMASK	0x8000000000000000U
#define EXPBIAS		(1023 + 52)

#define ABSV(n) ((n) < 0 ? -(n) : (n))
#define MINV(a, b) ((a) < (b) ? (a) : (b))

IVM_PRIVATE
ivm_uint64_t tens[] = {
	10000000000000000000U, 1000000000000000000U, 100000000000000000U,
	10000000000000000U, 1000000000000000U, 100000000000000U,
	10000000000000U, 1000000000000U, 100000000000U,
	10000000000U, 1000000000U, 100000000U,
	10000000U, 1000000U, 100000U,
	10000U, 1000U, 100U,
	10U, 1U
};

IVM_PRIVATE
IVM_INLINE
ivm_uint64_t
_get_dbits(ivm_double_t d)
{
	union {
		ivm_double_t dbl;
		ivm_uint64_t i;
	} dbl_bits = { d };

	return dbl_bits.i;
}

IVM_PRIVATE
fp_t
_fp_build(ivm_double_t d)
{
	ivm_uint64_t bits = _get_dbits(d);

	fp_t fp;
	fp.frac = bits & FRACMASK;
	fp.exp = (bits & EXPMASK) >> 52;

	if (fp.exp) {
		fp.frac += HIDDENBIT;
		fp.exp -= EXPBIAS;
	} else {
		fp.exp = -EXPBIAS + 1;
	}

	return fp;
}

IVM_PRIVATE
void
_normalize(fp_t *fp)
{
	while ((fp->frac & HIDDENBIT) == 0) {
		fp->frac <<= 1;
		fp->exp--;
	}

	ivm_int_t shift = 64 - 52 - 1;
	fp->frac <<= shift;
	fp->exp -= shift;

	return;
}

IVM_PRIVATE
void
_get_normalized_boundaries(fp_t *fp, fp_t *lower, fp_t *upper)
{
	upper->frac = (fp->frac << 1) + 1;
	upper->exp  = fp->exp - 1;

	while ((upper->frac & (HIDDENBIT << 1)) == 0) {
		upper->frac <<= 1;
		upper->exp--;
	}

	ivm_int_t u_shift = 64 - 52 - 2;

	upper->frac <<= u_shift;
	upper->exp = upper->exp - u_shift;


	ivm_int_t l_shift = fp->frac == HIDDENBIT ? 2 : 1;

	lower->frac = (fp->frac << l_shift) - 1;
	lower->exp = fp->exp - l_shift;


	lower->frac <<= lower->exp - upper->exp;
	lower->exp = upper->exp;

	return;
}

IVM_PRIVATE
fp_t
_fp_multiply(fp_t *a, fp_t *b)
{
	const ivm_uint64_t lomask = 0x00000000FFFFFFFF;

	ivm_uint64_t ah_bl = (a->frac >> 32)    * (b->frac & lomask);
	ivm_uint64_t al_bh = (a->frac & lomask) * (b->frac >> 32);
	ivm_uint64_t al_bl = (a->frac & lomask) * (b->frac & lomask);
	ivm_uint64_t ah_bh = (a->frac >> 32)    * (b->frac >> 32);

	ivm_uint64_t tmp = (ah_bl & lomask) + (al_bh & lomask) + (al_bl >> 32); 
	/* round up */
	tmp += 1U << 31;

	fp_t fp = {
		ah_bh + (ah_bl >> 32) + (al_bh >> 32) + (tmp >> 32),
		a->exp + b->exp + 64
	};

	return fp;
}

IVM_PRIVATE
void
_round_digit(ivm_char_t *digits,
			 ivm_int_t ndigits,
			 ivm_uint64_t delta,
			 ivm_uint64_t rem,
			 ivm_uint64_t kappa,
			 ivm_uint64_t frac)
{
	while (rem < frac && delta - rem >= kappa &&
		   (rem + kappa < frac || frac - rem > rem + kappa - frac)) {
		digits[ndigits - 1]--;
		rem += kappa;
	}
}

IVM_PRIVATE
ivm_int_t
_generate_digits(fp_t *fp,
				 fp_t *upper,
				 fp_t *lower,
				 ivm_char_t *digits,
				 ivm_int_t *k)
{
	ivm_uint64_t wfrac = upper->frac - fp->frac;
	ivm_uint64_t delta = upper->frac - lower->frac;

	fp_t one = {
		.frac = 1ULL << -upper->exp,
		.exp = upper->exp
	};

	ivm_uint64_t part1 = upper->frac >> -one.exp;
	ivm_uint64_t part2 = upper->frac & (one.frac - 1);

	ivm_int_t idx = 0, kappa = 10;
	ivm_uint64_t *divp;

	/* 1000000000 */
	for (divp = tens + 10; kappa > 0; divp++) {
		ivm_uint64_t div = *divp;
		ivm_uint_t digit = part1 / div;

		if (digit || idx) {
			digits[idx++] = digit + '0';
		}

		part1 -= digit * div;
		kappa--;

		ivm_uint64_t tmp = (part1 << -one.exp) + part2;

		if (tmp <= delta) {
			*k += kappa;
			_round_digit(digits, idx, delta, tmp, div << -one.exp, wfrac);

			return idx;
		}
	}

	/* 10 */
	ivm_uint64_t *unit = tens + 18;

	while (1) {
		part2 *= 10;
		delta *= 10;
		kappa--;

		ivm_uint_t digit = part2 >> -one.exp;

		if (digit || idx) {
			digits[idx++] = digit + '0';
		}

		part2 &= one.frac - 1;

		if (part2 < delta) {
			*k += kappa;
			_round_digit(digits, idx, delta, part2, one.frac, wfrac * *unit);

			break;
		}

		unit--;
	}

	return idx;
}

IVM_PRIVATE
ivm_int_t
_grisu2(ivm_double_t d, ivm_char_t *digits, ivm_int_t *kp)
{
	fp_t w = _fp_build(d);

	fp_t lower, upper;
	_get_normalized_boundaries(&w, &lower, &upper);

	_normalize(&w);

	ivm_int_t k;
	fp_t cp = _find_cachedpow10(upper.exp, &k);

	w     = _fp_multiply(&w,     &cp);
	upper = _fp_multiply(&upper, &cp);
	lower = _fp_multiply(&lower, &cp);

	lower.frac++;
	upper.frac--;

	*kp = -k;

	return _generate_digits(&w, &upper, &lower, digits, kp);
}

IVM_PRIVATE
ivm_int_t
_emit_digits(ivm_char_t *digits,
			 ivm_int_t ndigits,
			 ivm_char_t *dest,
			 ivm_int_t k,
			 ivm_bool_t neg)
{
	ivm_int_t exp = ABSV(k + ndigits - 1);

	/* write plain integer */
	if (k >= 0 && (exp < (ndigits + 7))) {
		memcpy(dest, digits, ndigits);
		memset(dest + ndigits, '0', k);

		return ndigits + k;
	}

	/* write decimal w/o scientific notation */
	if (k < 0 && (k > -7 || exp < 4)) {
		ivm_int_t offset = ndigits - ABSV(k);

		if (offset <= 0) {
			/* fp < 1.0 -> write leading zero */
			offset = -offset;
			dest[0] = '0';
			dest[1] = '.';
			memset(dest + 2, '0', offset);
			memcpy(dest + offset + 2, digits, ndigits);

			return ndigits + 2 + offset;
		} else {
			/* fp > 1.0 */
			memcpy(dest, digits, offset);
			dest[offset] = '.';
			memcpy(dest + offset + 1, digits + offset, ndigits - offset);

			return ndigits + 1;
		}
	}

	/* write decimal w/ scientific notation */
	ndigits = MINV(ndigits, 18 - neg);

	ivm_int_t idx = 0;
	dest[idx++] = digits[0];

	if (ndigits > 1) {
		dest[idx++] = '.';
		memcpy(dest + idx, digits + 1, ndigits - 1);
		idx += ndigits - 1;
	}

	dest[idx++] = 'e';

	ivm_char_t sign = k + ndigits - 1 < 0 ? '-' : '+';
	dest[idx++] = sign;

	ivm_int_t cent = 0;

	if (exp > 99) {
		cent = exp / 100;
		dest[idx++] = cent + '0';
		exp -= cent * 100;
	}

	if (exp > 9) {
		ivm_int_t dec = exp / 10;
		dest[idx++] = dec + '0';
		exp -= dec * 10;
	} else if (cent) {
		dest[idx++] = '0';
	}

	dest[idx++] = exp % 10 + '0';

	return idx;
}

IVM_PRIVATE
ivm_int_t
_filter_special(ivm_double_t fp, ivm_char_t *dest)
{
	if (fp == 0.0) {
		dest[0] = '0';
		return 1;
	}

	ivm_uint64_t bits = _get_dbits(fp);

	ivm_bool_t nan = (bits & EXPMASK) == EXPMASK;

	if (!nan) {
		return 0;
	}

	if (bits & FRACMASK) {
		dest[0] = 'n'; dest[1] = 'a'; dest[2] = 'n';

	} else {
		dest[0] = 'i'; dest[1] = 'n'; dest[2] = 'f';
	}

	return 3;
}

ivm_int_t
ivm_conv_dtoa(ivm_double_t d, ivm_char_t dest[25])
{
	ivm_char_t digits[18];

	ivm_int_t str_len = 0;
	ivm_bool_t neg = IVM_FALSE;

	if (_get_dbits(d) & SIGNMASK) {
		dest[0] = '-';
		str_len++;
		neg = IVM_TRUE;
	}

	ivm_int_t spec = _filter_special(d, dest + str_len);

	if (spec) {
		str_len += spec;
	} else {
		ivm_int_t k = 0;
		ivm_int_t ndigits = _grisu2(d, digits, &k);

		str_len += _emit_digits(digits, ndigits, dest + str_len, k, neg);
	}

	dest[str_len] = '\0';

	return str_len;
}
