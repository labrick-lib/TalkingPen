#include "EsFFTConst.h"

#if (8000 == MINI5_SAMPLERATE)
    #if !MINI5_USE_FFTW
/* Cosine table */
ivConst ivInt16 g_pwFFTCosTable[192] =
{
    32767,
    32757,
    32728,
    32678,
    32609,
    32521,
    32412,
    32285,
    32137,
    31971,
    31785,
    31580,
    31356,
    31113,
    30852,
    30571,
    30273,
    29956,
    29621,
    29268,
    28898,
    28510,
    28105,
    27683,
    27245,
    26790,
    26319,
    25832,
    25329,
    24811,
    24279,
    23731,
    23170,
    22594,
    22005,
    21403,
    20787,
    20159,
    19519,
    18868,
    18204,
    17530,
    16846,
    16151,
    15446,
    14732,
    14010,
    13279,
    12539,
    11793,
    11039,
    10278,
    9512,
    8739,
    7962,
    7179,
    6393,
    5602,
    4808,
    4011,
    3212,
    2410,
    1608,
    804,
    0,
    -804,
    -1608,
    -2410,
    -3212,
    -4011,
    -4808,
    -5602,
    -6393,
    -7179,
    -7962,
    -8739,
    -9512,
    -10278,
    -11039,
    -11793,
    -12539,
    -13279,
    -14010,
    -14732,
    -15446,
    -16151,
    -16846,
    -17530,
    -18204,
    -18868,
    -19519,
    -20159,
    -20787,
    -21403,
    -22005,
    -22594,
    -23170,
    -23731,
    -24279,
    -24811,
    -25329,
    -25832,
    -26319,
    -26790,
    -27245,
    -27683,
    -28105,
    -28510,
    -28898,
    -29268,
    -29621,
    -29956,
    -30273,
    -30571,
    -30852,
    -31113,
    -31356,
    -31580,
    -31785,
    -31971,
    -32137,
    -32285,
    -32412,
    -32521,
    -32609,
    -32678,
    -32728,
    -32757,
    -32767,
    -32757,
    -32728,
    -32678,
    -32609,
    -32521,
    -32412,
    -32285,
    -32137,
    -31971,
    -31785,
    -31580,
    -31356,
    -31113,
    -30852,
    -30571,
    -30273,
    -29956,
    -29621,
    -29268,
    -28898,
    -28510,
    -28105,
    -27683,
    -27245,
    -26790,
    -26319,
    -25832,
    -25329,
    -24811,
    -24279,
    -23731,
    -23170,
    -22594,
    -22005,
    -21403,
    -20787,
    -20159,
    -19519,
    -18868,
    -18204,
    -17530,
    -16846,
    -16151,
    -15446,
    -14732,
    -14010,
    -13279,
    -12539,
    -11793,
    -11039,
    -10278,
    -9512,
    -8739,
    -7962,
    -7179,
    -6393,
    -5602,
    -4808,
    -4011,
    -3212,
    -2410,
    -1608,
    -804
};

/* 8 Bit reverse table,仅存偶数 */
ivConst ivUInt16 g_ucReverseTable[100] =
{
    0x00,	0x40,	0x20,	0x60,
    0x10,	0x50,	0x30,	0x70,
    0x08,	0x48,	0x28,	0x68,
    0x18,	0x58,	0x38,	0x78,
    0x04,	0x44,	0x24,	0x64,
    0x14,	0x54,	0x34,	0x74,
    0x0C,	0x4C,	0x2C,	0x6C,
    0x1C,	0x5C,	0x3C,	0x7C,
    0x02,	0x42,	0x22,	0x62,
    0x12,	0x52,	0x32,	0x72,
    0x0A,	0x4A,	0x2A,	0x6A,
    0x1A,	0x5A,	0x3A,	0x7A,
    0x06,	0x46,	0x26,	0x66,
    0x16,	0x56,	0x36,	0x76,
    0x0E,	0x4E,	0x2E,	0x6E,
    0x1E,	0x5E,	0x3E,	0x7E, 
    0x01,   0x41, 	0x21,	0x61, 
    0x11 ,	0x51 ,	0x31 ,	0x71 ,
    0x09 ,	0x49 ,	0x29 ,	0x69 ,
    0x19 ,	0x59 ,	0x39 ,	0x79 ,
    0x05 ,	0x45 ,	0x25 ,	0x65 ,
    0x15 ,	0x55 ,	0x35 ,	0x75 ,
    0x0D ,	0x4D ,	0x2D ,	0x6D ,
    0x1D ,	0x5D ,	0x3D ,	0x7D ,
    0x03 ,	0x43 ,	0x23 ,	0x63 
};
    #else
/* 8 Bit reverse table,仅存偶数 */
ivConst ivUInt16 g_ucReverseTable[100] =
{
    0x00, 0x10, 0x20, 0x30, 
    0x40, 0x50, 0x60, 0x70, 
    0x01, 0x11, 0x21, 0x31, 
    0x41, 0x51, 0x61, 0x71, 
    0x02, 0x12, 0x22, 0x32, 
    0x42, 0x52, 0x62, 0x72, 
    0x03, 0x13, 0x23, 0x33, 
    0x43, 0x53, 0x63, 0x73, 
    0x04, 0x14, 0x24, 0x34, 
    0x44, 0x54, 0x64, 0x74, 
    0x05, 0x15, 0x25, 0x35, 
    0x45, 0x55, 0x65, 0x75, 
    0x06, 0x16, 0x26, 0x36, 
    0x46, 0x56, 0x66, 0x76, 
    0x07, 0x17, 0x27, 0x37, 
    0x47, 0x57, 0x67, 0x77, 
    0x08, 0x18, 0x28, 0x38, 
    0x48, 0x58, 0x68, 0x78, 
    0x09, 0x19, 0x29, 0x39, 
    0x49, 0x59, 0x69, 0x79, 
    0x0a, 0x1a, 0x2a, 0x3a, 
    0x4a, 0x5a, 0x6a, 0x7a, 
    0x0b, 0x1b, 0x2b, 0x3b, 
    0x4b, 0x5b, 0x6b, 0x7b, 
    0x0c, 0x1c, 0x2c, 0x3c
};

ivConst ivInt16 g_fCosTab[256] =
{
    32767,32758,32729,32679,32610,32522,32413,32286,32138,31972,31786,31581,31357,31114,30853,30572,
    30274,29957,29622,29269,28899,28511,28106,27684,27246,26791,26320,25833,25330,24812,24279,23732,
    23170,22595,22006,21403,20788,20160,19520,18868,18205,17531,16846,16151,15447,14733,14010,13279,
    12540,11793,11039,10279,9512,8740,7962,7180,6393,5602,4808,4011,3212,2411,1608,804,
    0,-804,-1608,-2411,-3212,-4011,-4808,-5602,-6393,-7180,-7962,-8740,-9512,-10279,-11039,-11793,
    -12540,-13279,-14010,-14733,-15447,-16151,-16846,-17531,-18205,-18868,-19520,-20160,-20788,-21403,-22006,-22595,
    -23170,-23732,-24279,-24812,-25330,-25833,-26320,-26791,-27246,-27684,-28106,-28511,-28899,-29269,-29622,-29957,
    -30274,-30572,-30853,-31114,-31357,-31581,-31786,-31972,-32138,-32286,-32413,-32522,-32610,-32679,-32729,-32758,
    -32768,-32758,-32729,-32679,-32610,-32522,-32413,-32286,-32138,-31972,-31786,-31581,-31357,-31114,-30853,-30572,
    -30274,-29957,-29622,-29269,-28899,-28511,-28106,-27684,-27246,-26791,-26320,-25833,-25330,-24812,-24279,-23732,
    -23170,-22595,-22006,-21403,-20788,-20160,-19520,-18868,-18205,-17531,-16846,-16151,-15447,-14733,-14010,-13279,
    -12540,-11793,-11039,-10279,-9512,-8740,-7962,-7180,-6393,-5602,-4808,-4011,-3212,-2411,-1608,-804,
    0,804,1608,2411,3212,4011,4808,5602,6393,7180,7962,8740,9512,10279,11039,11793,
    12540,13279,14010,14733,15447,16151,16846,17531,18205,18868,19520,20160,20788,21403,22006,22595,
    23170,23732,24279,24812,25330,25833,26320,26791,27246,27684,28106,28511,28899,29269,29622,29957,
    30274,30572,30853,31114,31357,31581,31786,31972,32138,32286,32413,32522,32610,32679,32729,32758
};
    #endif /* #if !MINI5_USE_FFTW */
#elif (16000 == MINI5_SAMPLERATE)
    #if !MINI5_USE_FFTW
/* Cosine table */
ivConst ivInt16 g_pwFFTCosTable[384] =
{
    32767,
    32765,
    32758,
    32745,
    32728,
    32706,
    32679,
    32647,
    32610,
    32568,
    32521,
    32469,
    32413,
    32351,
    32285,
    32214,
    32138,
    32057,
    31971,
    31881,
    31785,
    31685,
    31581,
    31471,
    31357,
    31237,
    31114,
    30985,
    30852,
    30714,
    30572,
    30425,
    30273,
    30117,
    29956,
    29791,
    29621,
    29447,
    29269,
    29086,
    28898,
    28707,
    28511,
    28310,
    28106,
    27897,
    27684,
    27466,
    27245,
    27020,
    26790,
    26557,
    26319,
    26077,
    25832,
    25583,
    25330,
    25073,
    24812,
    24547,
    24279,
    24007,
    23732,
    23453,
    23170,
    22884,
    22594,
    22301,
    22005,
    21706,
    21403,
    21097,
    20787,
    20475,
    20159,
    19841,
    19519,
    19195,
    18868,
    18537,
    18204,
    17869,
    17530,
    17189,
    16846,
    16499,
    16151,
    15800,
    15446,
    15090,
    14732,
    14372,
    14010,
    13645,
    13278,
    12910,
    12539,
    12167,
    11793,
    11416,
    11039,
    10659,
    10278,
    9896,
    9512,
    9126,
    8739,
    8351,
    7961,
    7571,
    7179,
    6786,
    6392,
    5997,
    5602,
    5205,
    4808,
    4409,
    4011,
    3611,
    3211,
    2811,
    2410,
    2009,
    1607,
    1206,
    804,
    402,
    0,
    -402,
    -804,
    -1206,
    -1607,
    -2009,
    -2410,
    -2811,
    -3211,
    -3611,
    -4011,
    -4409,
    -4808,
    -5205,
    -5602,
    -5997,
    -6392,
    -6786,
    -7179,
    -7571,
    -7961,
    -8351,
    -8739,
    -9126,
    -9512,
    -9896,
    -10278,
    -10659,
    -11039,
    -11416,
    -11793,
    -12167,
    -12539,
    -12910,
    -13278,
    -13645,
    -14010,
    -14372,
    -14732,
    -15090,
    -15446,
    -15800,
    -16151,
    -16499,
    -16846,
    -17189,
    -17530,
    -17869,
    -18204,
    -18537,
    -18868,
    -19195,
    -19519,
    -19841,
    -20159,
    -20475,
    -20787,
    -21097,
    -21403,
    -21706,
    -22005,
    -22301,
    -22594,
    -22884,
    -23170,
    -23453,
    -23732,
    -24007,
    -24279,
    -24547,
    -24812,
    -25073,
    -25330,
    -25583,
    -25832,
    -26077,
    -26319,
    -26557,
    -26790,
    -27020,
    -27245,
    -27466,
    -27684,
    -27897,
    -28106,
    -28310,
    -28511,
    -28707,
    -28898,
    -29086,
    -29269,
    -29447,
    -29621,
    -29791,
    -29956,
    -30117,
    -30273,
    -30425,
    -30572,
    -30714,
    -30852,
    -30985,
    -31114,
    -31237,
    -31357,
    -31471,
    -31581,
    -31685,
    -31785,
    -31881,
    -31971,
    -32057,
    -32138,
    -32214,
    -32285,
    -32351,
    -32413,
    -32469,
    -32521,
    -32568,
    -32610,
    -32647,
    -32679,
    -32706,
    -32728,
    -32745,
    -32758,
    -32765,
    -32767,
    -32765,
    -32758,
    -32745,
    -32728,
    -32706,
    -32679,
    -32647,
    -32610,
    -32568,
    -32521,
    -32469,
    -32413,
    -32351,
    -32285,
    -32214,
    -32138,
    -32057,
    -31971,
    -31881,
    -31785,
    -31685,
    -31581,
    -31471,
    -31357,
    -31237,
    -31114,
    -30985,
    -30852,
    -30714,
    -30572,
    -30425,
    -30273,
    -30117,
    -29956,
    -29791,
    -29621,
    -29447,
    -29269,
    -29086,
    -28898,
    -28707,
    -28511,
    -28310,
    -28106,
    -27897,
    -27684,
    -27466,
    -27245,
    -27020,
    -26790,
    -26557,
    -26319,
    -26077,
    -25832,
    -25583,
    -25330,
    -25072,
    -24812,
    -24547,
    -24279,
    -24007,
    -23732,
    -23453,
    -23170,
    -22884,
    -22594,
    -22301,
    -22005,
    -21706,
    -21403,
    -21097,
    -20787,
    -20475,
    -20159,
    -19841,
    -19519,
    -19195,
    -18868,
    -18537,
    -18204,
    -17869,
    -17530,
    -17189,
    -16846,
    -16499,
    -16151,
    -15800,
    -15446,
    -15090,
    -14732,
    -14372,
    -14010,
    -13645,
    -13278,
    -12910,
    -12539,
    -12167,
    -11793,
    -11416,
    -11039,
    -10659,
    -10278,
    -9896,
    -9512,
    -9126,
    -8739,
    -8351,
    -7961,
    -7571,
    -7179,
    -6786,
    -6392,
    -5997,
    -5602,
    -5205,
    -4808,
    -4409,
    -4011,
    -3611,
    -3211,
    -2811,
    -2410,
    -2009,
    -1607,
    -1206,
    -804,
    -402
};

/* 9 Bit reverse table,仅存偶数 */
ivConst ivUInt16 g_ucReverseTable[200] =
{
    0x00,  0x80,  0x40,  0xc0,  0x20,  0xa0,  0x60,  0xe0, 
    0x10,  0x90,  0x50,  0xd0,  0x30,  0xb0,  0x70,  0xf0, 
    0x08,  0x88,  0x48,  0xc8,  0x28,  0xa8,  0x68,  0xe8, 
    0x18,  0x98,  0x58,  0xd8,  0x38,  0xb8,  0x78,  0xf8, 
    0x04,  0x84,  0x44,  0xc4,  0x24,  0xa4,  0x64,  0xe4, 
    0x14,  0x94,  0x54,  0xd4,  0x34,  0xb4,  0x74,  0xf4, 
    0x0c,  0x8c,  0x4c,  0xcc,  0x2c,  0xac,  0x6c,  0xec, 
    0x1c,  0x9c,  0x5c,  0xdc,  0x3c,  0xbc,  0x7c,  0xfc, 
    0x02,  0x82,  0x42,  0xc2,  0x22,  0xa2,  0x62,  0xe2, 
    0x12,  0x92,  0x52,  0xd2,  0x32,  0xb2,  0x72,  0xf2, 
    0x0a,  0x8a,  0x4a,  0xca,  0x2a,  0xaa,  0x6a,  0xea, 
    0x1a,  0x9a,  0x5a,  0xda,  0x3a,  0xba,  0x7a,  0xfa, 
    0x06,  0x86,  0x46,  0xc6,  0x26,  0xa6,  0x66,  0xe6, 
    0x16,  0x96,  0x56,  0xd6,  0x36,  0xb6,  0x76,  0xf6, 
    0x0e,  0x8e,  0x4e,  0xce,  0x2e,  0xae,  0x6e,  0xee, 
    0x1e,  0x9e,  0x5e,  0xde,  0x3e,  0xbe,  0x7e,  0xfe, 
    0x01,  0x81,  0x41,  0xc1,  0x21,  0xa1,  0x61,  0xe1, 
    0x11,  0x91,  0x51,  0xd1,  0x31,  0xb1,  0x71,  0xf1, 
    0x09,  0x89,  0x49,  0xc9,  0x29,  0xa9,  0x69,  0xe9, 
    0x19,  0x99,  0x59,  0xd9,  0x39,  0xb9,  0x79,  0xf9, 
    0x05,  0x85,  0x45,  0xc5,  0x25,  0xa5,  0x65,  0xe5, 
    0x15,  0x95,  0x55,  0xd5,  0x35,  0xb5,  0x75,  0xf5, 
    0x0d,  0x8d,  0x4d,  0xcd,  0x2d,  0xad,  0x6d,  0xed, 
    0x1d,  0x9d,  0x5d,  0xdd,  0x3d,  0xbd,  0x7d,  0xfd, 
    0x03,  0x83,  0x43,  0xc3,  0x23,  0xa3,  0x63,  0xe3
};
    #else
/* 9 Bit reverse table,仅存偶数 */
ivConst ivUInt16 g_ucReverseTable[200] =
{
    0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 
    0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0, 
    0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 
    0x81, 0x91, 0xa1, 0xb1, 0xc1, 0xd1, 0xe1, 0xf1, 
    0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 
    0x82, 0x92, 0xa2, 0xb2, 0xc2, 0xd2, 0xe2, 0xf2, 
    0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73, 
    0x83, 0x93, 0xa3, 0xb3, 0xc3, 0xd3, 0xe3, 0xf3, 
    0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 
    0x84, 0x94, 0xa4, 0xb4, 0xc4, 0xd4, 0xe4, 0xf4, 
    0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75, 
    0x85, 0x95, 0xa5, 0xb5, 0xc5, 0xd5, 0xe5, 0xf5, 
    0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76, 
    0x86, 0x96, 0xa6, 0xb6, 0xc6, 0xd6, 0xe6, 0xf6, 
    0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77, 
    0x87, 0x97, 0xa7, 0xb7, 0xc7, 0xd7, 0xe7, 0xf7, 
    0x08, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78, 
    0x88, 0x98, 0xa8, 0xb8, 0xc8, 0xd8, 0xe8, 0xf8, 
    0x09, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x79, 
    0x89, 0x99, 0xa9, 0xb9, 0xc9, 0xd9, 0xe9, 0xf9, 
    0x0a, 0x1a, 0x2a, 0x3a, 0x4a, 0x5a, 0x6a, 0x7a, 
    0x8a, 0x9a, 0xaa, 0xba, 0xca, 0xda, 0xea, 0xfa, 
    0x0b, 0x1b, 0x2b, 0x3b, 0x4b, 0x5b, 0x6b, 0x7b, 
    0x8b, 0x9b, 0xab, 0xbb, 0xcb, 0xdb, 0xeb, 0xfb, 
    0x0c, 0x1c, 0x2c, 0x3c, 0x4c, 0x5c, 0x6c, 0x7c
};

ivConst ivInt16 g_fCosTab[512] = 
{
    32767,32766,32758,32746,32729,32706,32679,32647,32610,32568,32522,32470,32413,32352,32286,32214,
    32138,32058,31972,31881,31786,31686,31581,31471,31357,31238,31114,30986,30853,30715,30572,30425,
    30274,30118,29957,29792,29622,29448,29269,29086,28899,28707,28511,28311,28106,27897,27684,27467,
    27246,27020,26791,26557,26320,26078,25833,25583,25330,25073,24812,24548,24279,24008,23732,23453,
    23170,22884,22595,22302,22006,21706,21403,21097,20788,20475,20160,19841,19520,19195,18868,18538,
    18205,17869,17531,17190,16846,16500,16151,15800,15447,15091,14733,14373,14010,13646,13279,12910,
    12540,12167,11793,11417,11039,10660,10279,9896,9512,9127,8740,8351,7962,7571,7180,6787,
    6393,5998,5602,5205,4808,4410,4011,3612,3212,2811,2411,2009,1608,1206,804,402,
    0,-402,-804,-1206,-1608,-2009,-2411,-2811,-3212,-3612,-4011,-4410,-4808,-5205,-5602,-5998,
    -6393,-6787,-7180,-7571,-7962,-8351,-8740,-9127,-9512,-9896,-10279,-10660,-11039,-11417,-11793,-12167,
    -12540,-12910,-13279,-13646,-14010,-14373,-14733,-15091,-15447,-15800,-16151,-16500,-16846,-17190,-17531,-17869,
    -18205,-18538,-18868,-19195,-19520,-19841,-20160,-20475,-20788,-21097,-21403,-21706,-22006,-22302,-22595,-22884,
    -23170,-23453,-23732,-24008,-24279,-24548,-24812,-25073,-25330,-25583,-25833,-26078,-26320,-26557,-26791,-27020,
    -27246,-27467,-27684,-27897,-28106,-28311,-28511,-28707,-28899,-29086,-29269,-29448,-29622,-29792,-29957,-30118,
    -30274,-30425,-30572,-30715,-30853,-30986,-31114,-31238,-31357,-31471,-31581,-31686,-31786,-31881,-31972,-32058,
    -32138,-32214,-32286,-32352,-32413,-32470,-32522,-32568,-32610,-32647,-32679,-32706,-32729,-32746,-32758,-32766,
    -32768,-32766,-32758,-32746,-32729,-32706,-32679,-32647,-32610,-32568,-32522,-32470,-32413,-32352,-32286,-32214,
    -32138,-32058,-31972,-31881,-31786,-31686,-31581,-31471,-31357,-31238,-31114,-30986,-30853,-30715,-30572,-30425,
    -30274,-30118,-29957,-29792,-29622,-29448,-29269,-29086,-28899,-28707,-28511,-28311,-28106,-27897,-27684,-27467,
    -27246,-27020,-26791,-26557,-26320,-26078,-25833,-25583,-25330,-25073,-24812,-24548,-24279,-24008,-23732,-23453,
    -23170,-22884,-22595,-22302,-22006,-21706,-21403,-21097,-20788,-20475,-20160,-19841,-19520,-19195,-18868,-18538,
    -18205,-17869,-17531,-17190,-16846,-16500,-16151,-15800,-15447,-15091,-14733,-14373,-14010,-13646,-13279,-12910,
    -12540,-12167,-11793,-11417,-11039,-10660,-10279,-9896,-9512,-9127,-8740,-8351,-7962,-7571,-7180,-6787,
    -6393,-5998,-5602,-5205,-4808,-4410,-4011,-3612,-3212,-2811,-2411,-2009,-1608,-1206,-804,-402,
    0,402,804,1206,1608,2009,2411,2811,3212,3612,4011,4410,4808,5205,5602,5998,
    6393,6787,7180,7571,7962,8351,8740,9127,9512,9896,10279,10660,11039,11417,11793,12167,
    12540,12910,13279,13646,14010,14373,14733,15091,15447,15800,16151,16500,16846,17190,17531,17869,
    18205,18538,18868,19195,19520,19841,20160,20475,20788,21097,21403,21706,22006,22302,22595,22884,
    23170,23453,23732,24008,24279,24548,24812,25073,25330,25583,25833,26078,26320,26557,26791,27020,
    27246,27467,27684,27897,28106,28311,28511,28707,28899,29086,29269,29448,29622,29792,29957,30118,
    30274,30425,30572,30715,30853,30986,31114,31238,31357,31471,31581,31686,31786,31881,31972,32058,
    32138,32214,32286,32352,32413,32470,32522,32568,32610,32647,32679,32706,32729,32746,32758,32766
};
    #endif /* #if !MINI5_USE_FFTW */
#endif

