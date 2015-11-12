/****************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   � 2004 Polycom, Inc.
**
**   All rights reserved.
**
****************************************************************************/

/****************************************************************************
  Filename:    tables.c    

  Purpose:     Contains tables used by G.722.1 Annex C
		
  Design Notes:

****************************************************************************/

/***************************************************************************
 Include files                                                           
***************************************************************************/
#include "defs.h"

const Word16 int_region_standard_deviation_table[REGION_POWER_TABLE_SIZE] = {
    0,     0,   0, 0, 0, 0, 0, 0, 0, 0,
    0,     0,   0, 0, 0, 0, 0, 0, 0, 0,
    0,     0,   1, 1, 1, 1, 2, 3, 4, 6,
    8,    11,   16, 23, 32, 45, 64, 91, 128, 181,
  256,   362,  512, 724, 1024, 1448, 2048, 2896, 4096, 5793,
 8192, 11585, 16384, 23170, 0,0,0,0,0,0,
 0,0,0,0};

const Word16 standard_deviation_inverse_table[REGION_POWER_TABLE_SIZE] = {
  32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
  32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
  32767, 32767, 32767, 32767, 32767, 23170, 16384, 11585, 8192, 5793,
  4096, 2896, 2048, 1448, 1024, 724, 512, 362, 256, 181,
  128, 91, 64, 45, 32, 23, 16, 11, 8, 6,
  4, 3, 2, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 0, 0};


const Word16 step_size_inverse_table[NUM_CATEGORIES]={
	23167,16384,11585,8192,5793,4096,2896,2896
};
 

const Word8 vector_dimension[NUM_CATEGORIES] =  { 2, 2, 2, 4, 4, 5, 5, 1};
const Word8 number_of_vectors[NUM_CATEGORIES] = {10,10,10, 5, 5, 4, 4,20};
/* The last category isn't really coded with scalar quantization. */

const Word8 max_bin[NUM_CATEGORIES] = {13, 9, 6, 4, 3, 2, 1, 1};

const Word16 max_bin_plus_one_inverse[NUM_CATEGORIES] = 
{
	2341,3277,4682,6554,8193,10923,16385,16385
};

/*
 * Release 1.2.
 * Add new version of int_dead_zone[] to go with
 * changes to vector_huffman() in encoder.c.
 * 
 */

/************** See new version of table below
Word16 int_dead_zone[NUM_CATEGORIES]=
{
	9830,10813,11796,12780,13763,14746,16384,16384
};
***************/

/******** New version of table added in Release 1.2 ********/
const Word16 int_dead_zone[NUM_CATEGORIES]=         
{
2457, 2703, 2949, 3195, 3440, 3686, 4096, 4096
};

 
/*
 * Release 1.2.
 * Added this new table to go with
 * changes to vector_huffman() in encoder.c,
 * accompanies the new table for int_dead_zone[].
 */

const Word8 int_dead_zone_low_bits[NUM_CATEGORIES]=
{
2, 1, 0, 0, 3, 2, 0, 0
};


const Word16 samples_to_rmlt_window[DCT_LENGTH]=
{   44,   134,   224,   314,   404,   494,   584,   674,   764,   853,
   943,  1033,  1123,  1213,  1302,  1392,  1482,  1571,  1661,  1750,
  1840,  1929,  2019,  2108,  2197,  2286,  2376,  2465,  2554,  2643,
  2732,  2821,  2909,  2998,  3087,  3175,  3264,  3352,  3441,  3529,
  3617,  3705,  3793,  3881,  3969,  4057,  4144,  4232,  4319,  4407,
  4494,  4581,  4668,  4755,  4842,  4928,  5015,  5101,  5188,  5274,
  5360,  5446,  5532,  5617,  5703,  5788,  5873,  5959,  6043,  6128,
  6213,  6297,  6382,  6466,  6550,  6634,  6718,  6801,  6885,  6968,
  7051,  7134,  7217,  7299,  7382,  7464,  7546,  7628,  7709,  7791,
  7872,  7953,  8034,  8115,  8195,  8276,  8356,  8436,  8515,  8595,
  8674,  8753,  8832,  8911,  8989,  9068,  9146,  9223,  9301,  9378,
  9455,  9532,  9609,  9685,  9762,  9838,  9913,  9989, 10064, 10139,
 10214, 10288, 10363, 10437, 10510, 10584, 10657, 10730, 10803, 10875,
 10948, 11020, 11091, 11163, 11234, 11305, 11375, 11446, 11516, 11586,
 11655, 11724, 11793, 11862, 11930, 11998, 12066, 12134, 12201, 12268,
 12334, 12401, 12467, 12532, 12598, 12663, 12728, 12792, 12857, 12920,
 12984, 13047, 13110, 13173, 13235, 13297, 13359, 13420, 13481, 13542,
 13602, 13662, 13722, 13782, 13841, 13899, 13958, 14016, 14074, 14131,
 14188, 14245, 14301, 14357, 14413, 14468, 14523, 14578, 14632, 14686,
 14739, 14793, 14845, 14898, 14950, 15002, 15053, 15104, 15155, 15205,
 15255, 15305, 15354, 15403, 15451, 15500, 15547, 15595, 15642, 15688,
 15734, 15780, 15826, 15871, 15916, 15960, 16004, 16047, 16091, 16133,
 16176, 16218, 16259, 16300, 16341, 16382, 16422, 16461, 16501, 16540,
 16578, 16616, 16654, 16691, 16728, 16764, 16800, 16836, 16871, 16906,
 16940, 16974, 17008, 17041, 17074, 17106, 17138, 17170, 17201, 17232,
 17262, 17292, 17321, 17350, 17379, 17407, 17435, 17462, 17489, 17516,
 17542, 17567, 17593, 17617, 17642, 17666, 17689, 17713, 17735, 17758,
 17779, 17801, 17822, 17842, 17863, 17882, 17901, 17920, 17939, 17957,
 17974, 17991, 18008, 18024, 18040, 18055, 18070, 18085, 18099, 18113,
 18126, 18139, 18151, 18163, 18174, 18185, 18196, 18206, 18216, 18225,
 18234, 18242, 18250, 18257, 18265, 18271, 18277, 18283, 18288, 18293,
 18298, 18302, 18305, 18308, 18311, 18313, 18315, 18316, 18317, 18317,
};

const Word16 rmlt_to_samples_window[DCT_LENGTH]=
{  44,    133,   222,   310,   399,   488,   577,   666,   754,   843,
   932,   1020,  1109,  1198,  1286,  1375,  1464,  1552,  1641,  1729,
   1817,  1906,  1994,  2082,  2171,  2259,  2347,  2435,  2523,  2611,
   2699,  2786,  2874,  2962,  3049,  3137,  3224,  3312,  3399,  3486,
   3573,  3660,  3747,  3834,  3921,  4008,  4094,  4181,  4267,  4353,
   4439,  4526,  4611,  4697,  4783,  4869,  4954,  5040,  5125,  5210,
   5295,  5380,  5465,  5549,  5634,  5718,  5802,  5886,  5970,  6054,
   6138,  6221,  6304,  6388,  6471,  6553,  6636,  6719,  6801,  6883,
   6965,  7047,  7129,  7211,  7292,  7373,  7454,  7535,  7616,  7696,
   7777,  7857,  7937,  8016,  8096,  8175,  8254,  8333,  8412,  8491,
   8569,  8647,  8725,  8803,  8880,  8957,  9035,  9111,  9188,  9264,
   9341,  9417,  9492,  9568,  9643,  9718,  9793,  9868,  9942, 10016,
  10090, 10163, 10237, 10310, 10383, 10455, 10528, 10600, 10672, 10743,
  10815, 10886, 10957, 11027, 11098, 11168, 11237, 11307, 11376, 11445,
  11514, 11582, 11650, 11718, 11785, 11853, 11920, 11986, 12053, 12119,
  12185, 12250, 12315, 12380, 12445, 12509, 12573, 12637, 12701, 12764,
  12826, 12889, 12951, 13013, 13075, 13136, 13197, 13257, 13318, 13378,
  13437, 13497, 13556, 13614, 13673, 13731, 13788, 13846, 13903, 13959,
  14016, 14072, 14128, 14183, 14238, 14292, 14347, 14401, 14454, 14508,
  14561, 14613, 14665, 14717, 14769, 14820, 14871, 14921, 14971, 15021,
  15070, 15119, 15168, 15216, 15264, 15311, 15359, 15405, 15452, 15498,
  15544, 15589, 15634, 15678, 15722, 15766, 15810, 15853, 15895, 15938,
  15979, 16021, 16062, 16103, 16143, 16183, 16223, 16262, 16300, 16339,
  16377, 16414, 16452, 16488, 16525, 16561, 16596, 16632, 16666, 16701,
  16735, 16768, 16801, 16834, 16867, 16899, 16930, 16961, 16992, 17022,
  17052, 17082, 17111, 17140, 17168, 17196, 17223, 17250, 17277, 17303,
  17329, 17354, 17379, 17404, 17428, 17452, 17475, 17498, 17520, 17542,
  17564, 17585, 17606, 17626, 17646, 17665, 17684, 17703, 17721, 17739,
  17756, 17773, 17790, 17806, 17821, 17836, 17851, 17865, 17879, 17893,
  17906, 17918, 17931, 17942, 17954, 17965, 17975, 17985, 17995, 18004,
  18012, 18021, 18028, 18036, 18043, 18049, 18055, 18061, 18066, 18071,
  18076, 18079, 18083, 18086, 18089, 18091, 18093, 18094, 18095, 18095,
};

#ifdef SIREN14
const Word16 max_samples_to_rmlt_window[MAX_DCT_LENGTH]={
0,		43,		89,		133,	178,	222,	268,	314,	357,	403,
447,	493,	538,	582,	628,	671,	717,	763,	807,	853,
896,	942,	987,	1031,	1077,	1121,	1166,	1212,	1256,	1301,
1345,	1390,	1436,	1480,	1526,	1569,	1615,	1660,	1704,	1749,
1793,	1838,	1884,	1928,	1973,	2016,	2062,	2107,	2151,	2196,
2239,	2285,	2331,	2374,	2419,	2463,	2508,	2553,	2597,	2642,
2685,	2730,	2776,	2819,	2864,	2908,	2952,	2998,	3041,	3086,
3129,	3174,	3219,	3263,	3307,	3350,	3396,	3440,	3483,	3528,
3571,	3616,	3661,	3704,	3748,	3791,	3836,	3881,	3923,	3968,
4011,	4055,	4100,	4143,	4187,	4230,	4274,	4318,	4362,	4406,
4448,	4493,	4537,	4580,	4624,	4666,	4710,	4755,	4797,	4841,
4883,	4927,	4971,	5013,	5057,	5099,	5144,	5187,	5229,	5273,
5315,	5359,	5402,	5444,	5488,	5530,	5573,	5617,	5658,	5702,
5743,	5787,	5830,	5871,	5915,	5956,	6000,	6043,	6084,	6127,
6169,	6211,	6254,	6296,	6339,	6380,	6423,	6465,	6507,	6549,
6590,	6633,	6675,	6716,	6759,	6799,	6842,	6884,	6925,	6967,
7007,	7050,	7092,	7132,	7175,	7215,	7257,	7299,	7339,	7381,
7421,	7462,	7504,	7544,	7586,	7626,	7667,	7709,	7749,	7790,
7830,	7871,	7912,	7952,	7993,	8032,	8073,	8114,	8153,	8194,
8234,	8275,	8315,	8355,	8395,	8434,	8474,	8515,	8554,	8594,
8632,	8673,	8713,	8752,	8792,	8830,	8871,	8910,	8949,	8989,
9027,	9066,	9106,	9144,	9184,	9221,	9261,	9300,	9338,	9378,
9415,	9454,	9493,	9531,	9570,	9607,	9646,	9685,	9722,	9761,
9798,	9836,	9875,	9912,	9950,	9987,	10025,	10064,	10100,	10138,
10175,	10213,	10250,	10287,	10325,	10361,	10398,	10436,	10472,	10510,
10545,	10583,	10620,	10656,	10692,	10728,	10766,	10803,	10838,	10874,
10910,	10947,	10983,	11018,	11055,	11089,	11126,	11162,	11197,	11233,
11268,	11303,	11340,	11374,	11410,	11444,	11480,	11515,	11549,	11585,
11619,	11654,	11689,	11723,	11758,	11791,	11826,	11861,	11895,	11930,
11963,	11997,	12032,	12065,	12099,	12132,	12166,	12201,	12233,	12267,
12300,	12333,	12367,	12400,	12433,	12465,	12499,	12532,	12563,	12597,
12629,	12662,	12695,	12727,	12759,	12790,	12823,	12856,	12887,	12920,
12951,	12983,	13016,	13046,	13078,	13109,	13141,	13173,	13203,	13235,
13266,	13296,	13328,	13358,	13389,	13419,	13450,	13481,	13510,	13541,
13571,	13602,	13632,	13661,	13692,	13721,	13751,	13781,	13810,	13840,
13869,	13898,	13929,	13957,	13986,	14015,	14044,	14073,	14101,	14130,
14158,	14187,	14216,	14244,	14272,	14300,	14328,	14357,	14384,	14412,
14439,	14468,	14495,	14522,	14550,	14577,	14604,	14632,	14658,	14686,
14711,	14739,	14765,	14792,	14819,	14844,	14871,	14897,	14923,	14949,
14975,	15001,	15027,	15053,	15079,	15103,	15129,	15155,	15180,	15205,
15229,	15255,	15280,	15304,	15329,	15353,	15378,	15403,	15426,	15451,
15475,	15499,	15523,	15546,	15570,	15594,	15618,	15641,	15664,	15688,
15711,	15734,	15757,	15780,	15802,	15825,	15848,	15871,	15892,	15915,
15937,	15960,	15982,	16003,	16026,	16047,	16069,	16090,	16112,	16133,
16154,	16175,	16197,	16217,	16239,	16259,	16279,	16301,	16320,	16341,
16361,	16382,	16402,	16421,	16441,	16461,	16481,	16501,	16520,	16539,
16558,	16578,	16597,	16615,	16635,	16653,	16672,	16691,	16709,	16728,
16746,	16764,	16782,	16800,	16818,	16835,	16853,	16871,	16888,	16905,
16923,	16940,	16957,	16974,	16991,	17008,	17024,	17041,	17057,	17074,
17090,	17106,	17122,	17138,	17154,	17169,	17185,	17201,	17216,	17231,
17246,	17262,	17277,	17291,	17306,	17321,	17336,	17350,	17364,	17379,
17393,	17407,	17421,	17435,	17449,	17462,	17476,	17490,	17502,	17515,
17528,	17542,	17554,	17567,	17580,	17592,	17605,	17618,	17629,	17642,
17653,	17666,	17678,	17689,	17701,	17712,	17724,	17736,	17746,	17757,
17768,	17779,	17790,	17800,	17811,	17822,	17832,	17842,	17852,	17862,
17872,	17882,	17892,	17902,	17911,	17920,	17930,	17938,	17947,	17956,
17965,	17974,	17983,	17991,	17999,	18008,	18016,	18025,	18032,	18040,
18047,	18055,	18063,	18070,	18078,	18085,	18092,	18099,	18106,	18112,
18119,	18126,	18132,	18138,	18144,	18151,	18157,	18163,	18168,	18174,
18179,	18185,	18191,	18196,	18201,	18206,	18211,	18216,	18220,	18225,
18229,	18234,	18238,	18242,	18246,	18250,	18254,	18257,	18260,	18264,
18268,	18271,	18274,	18277,	18280,	18283,	18286,	18288,	18291,	18293,
18295,	18297,	18300,	18301,	18303,	18305,	18306,	18308,	18309,	18311,
18312,	18312,	18314,	18315,	18315,	18316,	18316,	18317,	18317,	18317
};

const Word16 max_rmlt_to_samples_window[MAX_DCT_LENGTH]={
0,		43,		88,		131,	176,	219,	265,	310,	353,	398,
442,	487,	532,	575,	620,	663,	709,	754,	797,	842,
885,	931,	975,	1019,	1064,	1107,	1152,	1197,	1240,	1286,
1329,	1373,	1419,	1462,	1507,	1550,	1595,	1640,	1683,	1728,
1771,	1816,	1861,	1904,	1949,	1992,	2037,	2081,	2125,	2170,
2212,	2258,	2302,	2345,	2390,	2433,	2477,	2522,	2565,	2610,
2652,	2697,	2742,	2784,	2829,	2872,	2916,	2961,	3004,	3048,
3091,	3136,	3180,	3223,	3267,	3310,	3354,	3399,	3441,	3485,
3528,	3572,	3616,	3659,	3703,	3745,	3790,	3834,	3876,	3920,
3962,	4006,	4050,	4093,	4136,	4179,	4222,	4266,	4309,	4352,
4394,	4438,	4482,	4524,	4568,	4610,	4653,	4697,	4739,	4782,
4824,	4867,	4911,	4953,	4996,	5038,	5081,	5124,	5166,	5209,
5251,	5294,	5337,	5378,	5421,	5463,	5506,	5548,	5590,	5633,
5674,	5717,	5759,	5800,	5843,	5884,	5927,	5970,	6011,	6053,
6094,	6136,	6178,	6219,	6262,	6302,	6345,	6387,	6428,	6470,
6510,	6552,	6594,	6635,	6677,	6717,	6759,	6801,	6841,	6883,
6922,	6964,	7006,	7046,	7087,	7127,	7169,	7210,	7250,	7291,
7331,	7372,	7413,	7453,	7494,	7533,	7574,	7615,	7655,	7695,
7735,	7776,	7816,	7855,	7896,	7935,	7975,	8016,	8054,	8095,
8134,	8174,	8214,	8253,	8293,	8332,	8371,	8412,	8450,	8490,
8528,	8568,	8607,	8646,	8685,	8723,	8763,	8802,	8840,	8879,
8917,	8956,	8995,	9033,	9072,	9109,	9148,	9187,	9225,	9264,
9301,	9340,	9378,	9415,	9454,	9491,	9529,	9567,	9604,	9642,
9679,	9717,	9755,	9791,	9829,	9866,	9903,	9941,	9977,	10015,
10051,	10089,	10126,	10162,	10199,	10235,	10272,	10309,	10345,	10382,
10417,	10454,	10491,	10526,	10563,	10598,	10635,	10672,	10706,	10742,
10778,	10814,	10850,	10885,	10921,	10955,	10991,	11027,	11061,	11097,
11131,	11166,	11202,	11236,	11271,	11305,	11340,	11376,	11409,	11444,
11478,	11513,	11547,	11580,	11615,	11648,	11683,	11717,	11751,	11785,
11817,	11852,	11886,	11918,	11952,	11985,	12018,	12053,	12085,	12118,
12150,	12184,	12217,	12249,	12282,	12314,	12347,	12380,	12411,	12444,
12476,	12508,	12541,	12572,	12604,	12635,	12668,	12700,	12731,	12763,
12794,	12826,	12858,	12888,	12920,	12950,	12982,	13013,	13043,	13074,
13105,	13135,	13166,	13196,	13227,	13257,	13287,	13317,	13347,	13377,
13407,	13437,	13467,	13496,	13525,	13555,	13585,	13614,	13643,	13672,
13701,	13730,	13760,	13787,	13817,	13845,	13873,	13903,	13930,	13959,
13987,	14015,	14043,	14071,	14099,	14126,	14154,	14183,	14209,	14237,
14264,	14292,	14319,	14346,	14373,	14400,	14427,	14454,	14480,	14507,
14533,	14560,	14586,	14612,	14639,	14664,	14691,	14717,	14742,	14768,
14793,	14819,	14845,	14870,	14896,	14920,	14945,	14971,	14996,	15020,
15044,	15070,	15094,	15118,	15143,	15167,	15192,	15216,	15239,	15263,
15287,	15311,	15335,	15358,	15382,	15405,	15428,	15452,	15474,	15498,
15520,	15543,	15566,	15588,	15611,	15633,	15656,	15678,	15700,	15722,
15744,	15766,	15788,	15809,	15831,	15852,	15874,	15895,	15916,	15937,
15958,	15979,	16000,	16020,	16041,	16061,	16082,	16103,	16122,	16143,
16162,	16183,	16203,	16222,	16242,	16261,	16281,	16300,	16319,	16339,
16357,	16377,	16396,	16414,	16433,	16451,	16470,	16488,	16506,	16525,
16542,	16561,	16579,	16596,	16614,	16631,	16649,	16667,	16683,	16700,
16717,	16735,	16752,	16768,	16785,	16801,	16818,	16834,	16850,	16867,
16883,	16899,	16915,	16930,	16945,	16961,	16977,	16992,	17007,	17022,
17037,	17052,	17067,	17081,	17096,	17111,	17126,	17140,	17154,	17168,
17182,	17196,	17209,	17223,	17237,	17250,	17264,	17277,	17290,	17303,
17315,	17329,	17341,	17354,	17367,	17379,	17391,	17404,	17415,	17428,
17439,	17451,	17463,	17475,	17486,	17497,	17509,	17520,	17531,	17542,
17552,	17563,	17574,	17584,	17595,	17605,	17616,	17626,	17636,	17646,
17655,	17665,	17675,	17684,	17694,	17703,	17712,	17721,	17730,	17739,
17747,	17756,	17764,	17773,	17781,	17789,	17798,	17806,	17813,	17821,
17829,	17836,	17843,	17851,	17858,	17866,	17872,	17879,	17886,	17893,
17899,	17906,	17912,	17918,	17924,	17931,	17937,	17942,	17948,	17953,
17959,	17964,	17970,	17975,	17980,	17985,	17990,	17995,	17999,	18004,
18008,	18012,	18016,	18021,	18025,	18028,	18032,	18036,	18039,	18043,
18046,	18049,	18052,	18055,	18058,	18061,	18064,	18067,	18069,	18071,
18073,	18075,	18078,	18079,	18081,	18083,	18084,	18086,	18087,	18089,
18090,	18090,	18091,	18092,	18093,	18094,	18094,	18095,	18095,	18095
};

#endif
