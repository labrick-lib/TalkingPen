/* ***************************************************************************# 
* 文件名     ：ivEsrFFTW.c
* 文件功能   ：Fixed-point FFTW (Fast Fourier Transform In The West). 
* 作者       ：djqu 
* 创建时间   ：2013.05.10
* 项目名称   ：AitalkMini 5.0
* 备注       ：Support 256 and 512-point FFT of real number, as well as 128 and 256-point FFT of complex number.
*---------------------------------------------------------------------# 
* 历史记录： 
* 日期       作者       备注 

*---------------------------------------------------------------------# 
**************************************************************************** */ 
#include "EsFFTConst.h"
#include "EsFFT.h"

#if !MINI5_USE_FFTW
ivUInt16 EsFixedFFTCore(PFFTDesc pFFTDesc, ivInt16 s16Exponent)
{
    ivInt16  i,j,k,n,m;
    ivInt16  nBlockEnd, nBlockSize;	
    ivPCInt16 ps16FFTReal0,ps16FFTImage0;
    ivInt16  s16RealTmp, s16ImagTmp;
    ivInt16   s16Real0, s16Imag0;
    ivInt16 s16Exponent2 = s16Exponent;// - ESR_EXPONENT_FFT;
    ivInt16 s16Exponent3 = 0;
    ivInt16 wCont;
    nBlockEnd = 2;   
    ps16FFTReal0 = g_pwFFTCosTable;
#if (8000 == MINI5_SAMPLERATE)
    ps16FFTImage0 = &(g_pwFFTCosTable[64]);

	/* ------------------------------------------------------------------- */	
	for ( nBlockSize = 4,m = 6; nBlockSize <= TRANSFORM_HALFFFTNUM_DEF; nBlockSize <<= 1,m-- )
#elif (16000 == MINI5_SAMPLERATE)
	ps16FFTImage0 = &(g_pwFFTCosTable[128]);

	/* ------------------------------------------------------------------- */	
	for ( nBlockSize = 4,m = 7; nBlockSize <= TRANSFORM_HALFFFTNUM_DEF; nBlockSize <<= 1,m-- )
#endif
	{
		ivInt16 bRShift;

		if(4 == m){
			bRShift = 0;
		}
		else if(s16Exponent2 > 0){
			bRShift = 0;

			s16Exponent2--;
		}
		else{
			bRShift = 1;
			s16Exponent3++;
		}

		/* int sMax = 0; */
		for ( n = 0; n < nBlockEnd; n++ )
		{
			wCont = (ivInt16)(n<<m);

			/*W(N/2,k)=W(N,2k)*/
			/* 0.15 */
			s16Real0 = ps16FFTReal0[wCont];
			s16Imag0 = ps16FFTImage0[wCont];

			if(bRShift){
				for (i = 0; i < TRANSFORM_HALFFFTNUM_DEF; i = i + nBlockSize/*i += nBlockSize*/ )				
				{
					j = i + n;
					k = j + nBlockEnd;

					/* 乘法 int16*int16 */
					s16ImagTmp = (ivInt16)(((ivInt32)s16Real0*(ivInt32)(pFFTDesc[k].nImageOut) + (ivInt32)s16Imag0*(ivInt32)(pFFTDesc[k].nRealOut))>>16);
					s16RealTmp = (ivInt16)(((ivInt32)s16Real0*(ivInt32)(pFFTDesc[k].nRealOut) - (ivInt32)s16Imag0*(ivInt32)(pFFTDesc[k].nImageOut))>>16);  

					pFFTDesc[k].nRealOut = (pFFTDesc[j].nRealOut>>1) - s16RealTmp;
					/* sMax |= pwRealOut[k] ^(pwRealOut[k]>>15); */
					pFFTDesc[j].nRealOut = (pFFTDesc[j].nRealOut>>1) + s16RealTmp; 
					/* sMax |= pwRealOut[j] ^(pwRealOut[j]>>15); */
					pFFTDesc[k].nImageOut = (pFFTDesc[j].nImageOut>>1) - s16ImagTmp;
					/* sMax |= pwImageOut[k] ^(pwImageOut[k]>>15); */					
					pFFTDesc[j].nImageOut = (pFFTDesc[j].nImageOut>>1) + s16ImagTmp;
					/* sMax |= pwImageOut[j]^(pwImageOut[j]>>15); */
				}
			}
			else{
				for (i = 0; i < TRANSFORM_HALFFFTNUM_DEF; i = i + nBlockSize/*i += nBlockSize*/ )				
				{
					j = i + n;
					k = j + nBlockEnd;	

					/* 乘法 int16*int16 */
					s16ImagTmp = (ivInt16)((((ivInt32)s16Real0*(ivInt32)(pFFTDesc[k].nImageOut)<<1) + ((ivInt32)s16Imag0*(ivInt32)(pFFTDesc[k].nRealOut)<<1))>>16);
					s16RealTmp = (ivInt16)((((ivInt32)s16Real0*(ivInt32)(pFFTDesc[k].nRealOut)<<1) - ((ivInt32)s16Imag0*(ivInt32)(pFFTDesc[k].nImageOut)<<1))>>16);  

					pFFTDesc[k].nRealOut = pFFTDesc[j].nRealOut - s16RealTmp;
					/* sMax |= pwRealOut[k]^(pwRealOut[k]>>15); */
					pFFTDesc[j].nRealOut = pFFTDesc[j].nRealOut + s16RealTmp; 
					/* sMax |= pwRealOut[j]^(pwRealOut[j]>>15); */
					pFFTDesc[k].nImageOut = pFFTDesc[j].nImageOut - s16ImagTmp;
					/* sMax |= pwImageOut[k]^(pwImageOut[k]>>15); */
					pFFTDesc[j].nImageOut = pFFTDesc[j].nImageOut + s16ImagTmp; 	
					/* sMax |= pwImageOut[j]^(pwImageOut[j]>>15); */
				}
			}
		}

		nBlockEnd = nBlockSize; 
	}

	pFFTDesc[TRANSFORM_HALFFFTNUM_DEF].nRealOut = pFFTDesc[0].nRealOut;
	pFFTDesc[TRANSFORM_HALFFFTNUM_DEF].nImageOut = pFFTDesc[0].nImageOut;

	for(j=0,k=TRANSFORM_HALFFFTNUM_DEF; j < (TRANSFORM_HALFFFTNUM_DEF>>1); j++,--k)
	{
		ivInt16 s16RealYk, s16ImagYk, s16RealZk, s16ImagZk;
		s16Real0 = ps16FFTReal0[j];
		s16Imag0 = ps16FFTImage0[j];

		/* Calculate two points once. The reason is when calculate Filter Bank,
		first half of 256 points is enough, so here equal to save half of 
		butterfly algorithm result */

		s16RealYk = (pFFTDesc[j].nRealOut + pFFTDesc[k].nRealOut)>>1;
		s16ImagZk = (pFFTDesc[k].nRealOut - pFFTDesc[j].nRealOut)>>1;
		s16ImagYk = (pFFTDesc[j].nImageOut - pFFTDesc[k].nImageOut)>>1;
		s16RealZk = (pFFTDesc[j].nImageOut + pFFTDesc[k].nImageOut)>>1;				

		/* 乘法 int16*int16 */
		s16ImagTmp = (ivInt16)(((((ivInt32)s16RealZk*(ivInt32)s16Imag0)<<1)+(((ivInt32)s16ImagZk*(ivInt32)s16Real0)<<1))>>16);
		s16RealTmp = (ivInt16)(((((ivInt32)s16RealZk*(ivInt32)s16Real0)<<1)-(((ivInt32)s16ImagZk*(ivInt32)s16Imag0)<<1))>>16);		

		pFFTDesc[j].nRealOut = s16RealTmp + s16RealYk;
		pFFTDesc[j].nImageOut = s16ImagTmp + s16ImagYk;
		pFFTDesc[k].nRealOut = -s16RealTmp + s16RealYk;		
		pFFTDesc[k].nImageOut = s16ImagTmp - s16ImagYk;
	}

	s16Exponent = - s16Exponent3;   /* 输入FFT真实Q */
	return s16Exponent;
}

#else
#define	 FFTW_BLOCK				(16)
//Q15
#define fCos_PI_16		0x7D8A	//+0.980785280403230449126182236134239036973933731
#define fCos_2PI_16		0x7642	//+0.923879532511286756128183189396788286822416626
#define fCos_3PI_16		0x6A6E	//+0.831469612302545237078788377617905756738560812
#define fCos_4PI_16		0x5A82	//+0.707106781186547524400844362104849039284835938
#define fCos_5PI_16		0x471D	//+0.555570233019602224742830813948532874374937191
#define fCos_6PI_16		0x30FC	//+0.382683432365089771728459984030398866761344562
#define fCos_7PI_16		0x18F9	//+0.195090322016128267848284868477022240927691618
#define STACKTEST			(0)
#if STACKTEST 
/*构建一个栈区*/
#define STACK_SIZE_MAX		(8 * 1024)
#define STACK_FILLING		0xFFFFFFFFU
static unsigned int * InitStack();
static int GetStackSize(unsigned int *stack);

/*构建一个栈区*/
static unsigned int * InitStack() {
	unsigned int * stack = NULL;
	unsigned int * p = NULL;
	int i = 0;
	int j;

	//__asm {
	//	mov stack, sp;
	//} 

	stack = (unsigned int *)&j;

	p = stack;
	for (i = 0; i < STACK_SIZE_MAX; i++) {
		/*在Windows下,栈是向低地址扩展的数据*/
		*p -- = STACK_FILLING;
	}
	return stack;
}

static int GetStackSize(unsigned int *stack) {
	int i = 0; // 补尝Init 3个变量的空间
	stack -= STACK_SIZE_MAX - 1;
	while(*stack++ == 0xFFFFFFFFU) {
		++ i;
	}
	return (STACK_SIZE_MAX - i) * sizeof(int);
}
#endif
static void FFTW16_Step_Block(PFFTDesc p, ivInt16 nQ)
{
	PFFTDesc pData = p;

	ivInt32 T_0_2_r_Q2, T_2_2_r_Q2, T_2_2_i_Q2, T_0_2_i_Q2, T_3_2_r_Q2, T_1_2_r_Q2, T_1_2_i_Q2, T_3_2_i_Q2, T_12_2_r_Q2, T_14_2_i_n_Q2, T_12_2_i_Q2, T_15_2_i_n_Q2, T_13_2_r_Q2, T_15_2_r_n_Q2, T_14_2_r_n_Q2;
	ivInt32 T_13_2_i_Q2, T_4_2_r_Q2, T_6_2_i_Q2, T_6_2_r_n_Q2, T_4_2_i_Q2, T_5_1_i_w_Q2, T_7_1_r_w_Q2, T_5_1_r_w_Q2, T_7_1_i_w_Q2, T_8_2_r_Q2, T_10_2_i_Q2, T_8_2_i_Q2, T_11_2_i_q_Q2, T_11_2_r_q_Q2, T_9_2_r_q_Q2;
	ivInt32 T_10_2_r_Q2, T_9_2_i_q_Q2;
	{
		ivInt32 T_0_1_r_Q1, T_1_1_r_Q1, T_0_1_i_Q1, T_1_1_i_Q1, T_2_1_r_Q1, T_3_1_r_Q1, T_2_1_i_Q1, T_3_1_i_Q1;
		{
			//(x[0],x[8],0,0)->(T_0_1,T_1_1)
			ivInt32 RA, RB, IA, IB;
			RA = (pData[0].nRealOut << nQ) + 4;
			RB = pData[8].nRealOut << nQ;
			T_0_1_r_Q1 = RA + RB;
			T_1_1_r_Q1 = RA - RB;
			IA = (pData[0].nImageOut << nQ) + 4;
			IB = pData[8].nImageOut << nQ;
			T_0_1_i_Q1 = IA + IB;
			T_1_1_i_Q1 = IA - IB;
		}
		{
			//(x[4],x[12],0,0)->(T_2_1,T_3_1)
			ivInt32 RA, RB, IA, IB;
			RA = pData[4].nRealOut << nQ;
			RB =pData[12].nRealOut << nQ;
			T_2_1_r_Q1 = RA + RB;
			T_3_1_r_Q1 = RA - RB;
			IA = pData[4].nImageOut << nQ;
			IB = pData[12].nImageOut << nQ;
			T_2_1_i_Q1 = IA + IB;
			T_3_1_i_Q1 = IA - IB;
		}
		// (T_0_1,T_1_1,T_2_1,T_3_1)*(0,0,0,-j) -> (T_0_2,T_1_2,T_2_2,T_3_2)
		T_0_2_r_Q2 = T_0_1_r_Q1 + T_2_1_r_Q1;
		T_2_2_r_Q2 = T_0_1_r_Q1 - T_2_1_r_Q1;
		T_2_2_i_Q2 = T_0_1_i_Q1 - T_2_1_i_Q1;
		T_0_2_i_Q2 = T_0_1_i_Q1 + T_2_1_i_Q1;
		T_3_2_r_Q2 = T_1_1_r_Q1 - T_3_1_i_Q1;
		T_1_2_r_Q2 = T_1_1_r_Q1 + T_3_1_i_Q1;
		T_1_2_i_Q2 = T_1_1_i_Q1 - T_3_1_r_Q1;
		T_3_2_i_Q2 = T_3_1_r_Q1 + T_1_1_i_Q1;
	}
	{
		ivInt32 T_14_1_r_Q1, T_15_1_i_Q1, T_15_1_r_Q1, T_14_1_i_Q1, T_12_1_r_Q1, T_13_1_r_Q1, T_13_1_i_Q1, T_12_1_i_Q1;
		{
			// (x[7],x[15],0,0)->(T_14_1,T_15_1)
			ivInt32 RB, RA, IB, IA;
			RB = pData[15].nRealOut << nQ;
			RA = pData[7].nRealOut << nQ;
			T_14_1_r_Q1 = RB + RA;
			T_15_1_i_Q1 = RB - RA;
			IB = pData[15].nImageOut << nQ;
			IA = pData[7].nImageOut << nQ;
			T_15_1_r_Q1 = IB - IA;
			T_14_1_i_Q1 = IB + IA;
		}
		{
			// (x[3],x[11],0,0)->(T_12_1,T_13_1)
			ivInt32 RA, RB, IA, IB;
			RA = pData[3].nRealOut << nQ;
			RB = pData[11].nRealOut << nQ;
			T_12_1_r_Q1 = RA + RB;
			T_13_1_r_Q1 = RA - RB;
			IA = pData[3].nImageOut << nQ;
			IB = pData[11].nImageOut << nQ;
			T_13_1_i_Q1 = IA - IB;
			T_12_1_i_Q1 = IA + IB;
		}
		// T_12_2,T_13_2,T_14_2,T_15_2
		T_12_2_r_Q2 = T_14_1_r_Q1 + T_12_1_r_Q1;
		T_14_2_i_n_Q2 = T_14_1_i_Q1 - T_12_1_i_Q1;
		T_12_2_i_Q2 = T_14_1_i_Q1 + T_12_1_i_Q1;
		T_15_2_i_n_Q2 = T_15_1_i_Q1 - T_13_1_i_Q1;
		T_13_2_r_Q2 = T_13_1_r_Q1 + T_15_1_r_Q1;
		T_15_2_r_n_Q2 = T_15_1_r_Q1 - T_13_1_r_Q1;
		T_14_2_r_n_Q2 = T_14_1_r_Q1 - T_12_1_r_Q1;
		T_13_2_i_Q2 = T_15_1_i_Q1 + T_13_1_i_Q1;
	}
	{
		ivInt32 T_4_1_r_Q1, T_5_1_r_Q1, T_4_1_i_Q1, T_5_1_i_Q1, T_6_1_r_Q1, T_7_1_r_n_Q1, T_6_1_i_Q1, T_7_1_i_n_Q1;
		{
			// (x[2],x[10],0,0)->(T_2_1,T_10_1)
			ivInt32 RA, RB, IA, IB;
			RA = pData[2].nRealOut << nQ;
			RB = pData[10].nRealOut << nQ;
			T_4_1_r_Q1 = RA + RB;
			T_5_1_r_Q1 = RA - RB;
			IA = pData[2].nImageOut << nQ;
			IB = pData[10].nImageOut << nQ;
			T_4_1_i_Q1 = IA + IB;
			T_5_1_i_Q1 = IA - IB;
		}
		{
			// (x[6],x[14],0,0)->(T_6_1,T_14_1)
			ivInt32 RB, RA, IB, IA;
			RB = pData[14].nRealOut << nQ;
			RA = pData[6].nRealOut << nQ;
			T_6_1_r_Q1 = RB + RA;
			T_7_1_r_n_Q1 = RB - RA;
			IB = pData[14].nImageOut << nQ;
			IA = pData[6].nImageOut << nQ;
			T_6_1_i_Q1 = IB + IA;
			T_7_1_i_n_Q1 = IB - IA;
		}
		T_4_2_r_Q2 = T_4_1_r_Q1 + T_6_1_r_Q1;
		T_6_2_i_Q2 = T_4_1_i_Q1 - T_6_1_i_Q1;
		T_6_2_r_n_Q2 = T_6_1_r_Q1 - T_4_1_r_Q1;
		T_4_2_i_Q2 = T_4_1_i_Q1 + T_6_1_i_Q1;
		T_5_1_i_w_Q2 = T_5_1_i_Q1 - T_5_1_r_Q1; 
		T_7_1_r_w_Q2 = T_7_1_r_n_Q1 - T_7_1_i_n_Q1;
		T_5_1_r_w_Q2 = T_5_1_r_Q1 + T_5_1_i_Q1;
		T_7_1_i_w_Q2 = T_7_1_r_n_Q1 + T_7_1_i_n_Q1; //(-1*-1, di xiao _n_)
	}
	{
		ivInt32 T_8_1_r_Q1, T_9_1_r_Q1, T_9_1_i_Q1, T_8_1_i_Q1, T_10_1_r_Q1, T_11_1_r_Q1, T_11_1_i_Q1, T_10_1_i_Q1;
		{
			ivInt32 RA, RB, IA, IB;
			RA = pData[1].nRealOut << nQ;
			RB = pData[9].nRealOut << nQ;
			T_8_1_r_Q1 = RA + RB;
			T_9_1_r_Q1 = RA - RB;
			IA = pData[1].nImageOut << nQ;
			IB = pData[9].nImageOut << nQ;
			T_9_1_i_Q1 = IA - IB;
			T_8_1_i_Q1 = IA + IB;
		}
		{
			ivInt32 RA, RB, IA, IB;
			RA = pData[5].nRealOut << nQ;
			RB = pData[13].nRealOut << nQ;
			T_10_1_r_Q1 = RA + RB;
			T_11_1_r_Q1 = RA - RB;
			IA = pData[5].nImageOut << nQ;
			IB = pData[13].nImageOut << nQ;
			T_11_1_i_Q1 = IA - IB;
			T_10_1_i_Q1 = IA + IB;
		}
		T_8_2_r_Q2 = T_8_1_r_Q1 + T_10_1_r_Q1;
		T_10_2_i_Q2 = T_8_1_i_Q1 - T_10_1_i_Q1;
		T_8_2_i_Q2 = T_8_1_i_Q1 + T_10_1_i_Q1;
		T_11_2_i_q_Q2 = T_11_1_r_Q1 + T_9_1_i_Q1; // -j
		T_11_2_r_q_Q2 = T_9_1_r_Q1 - T_11_1_i_Q1;
		T_9_2_r_q_Q2 = T_9_1_r_Q1 + T_11_1_i_Q1;
		T_10_2_r_Q2 = T_8_1_r_Q1 - T_10_1_r_Q1;
		T_9_2_i_q_Q2 = T_9_1_i_Q1 - T_11_1_r_Q1;
	}
	{
		ivInt32 T_0_3_r_Q3, T_8_3_r_Q3, T_0_3_i_Q3, T_8_3_i_Q3;
		T_0_3_r_Q3 = T_0_2_r_Q2 + T_4_2_r_Q2;
		T_8_3_r_Q3 = T_8_2_r_Q2 + T_12_2_r_Q2;
		p[8].nRealOut = (T_0_3_r_Q3 - T_8_3_r_Q3) >> 4;
		p[0].nRealOut = (T_0_3_r_Q3 + T_8_3_r_Q3) >> 4;
		T_0_3_i_Q3 = T_0_2_i_Q2 + T_4_2_i_Q2;
		T_8_3_i_Q3 = T_8_2_i_Q2 + T_12_2_i_Q2;
		p[8].nImageOut = (T_0_3_i_Q3 - T_8_3_i_Q3) >> 4;
		p[0].nImageOut = (T_0_3_i_Q3 + T_8_3_i_Q3) >> 4;
	}
	{
		ivInt32 T_12_3_i_Q3, T_4_3_i_Q3, T_4_3_r_Q3, T_12_3_r_Q3;
		T_12_3_i_Q3 = T_12_2_r_Q2 - T_8_2_r_Q2; // multiply -j, Real and Imag swaped
		T_4_3_i_Q3 = T_0_2_i_Q2 - T_4_2_i_Q2;
		p[4].nImageOut = (T_12_3_i_Q3 + T_4_3_i_Q3) >> 4;
		p[12].nImageOut = (T_4_3_i_Q3 - T_12_3_i_Q3) >> 4;
		T_4_3_r_Q3 = T_0_2_r_Q2 - T_4_2_r_Q2;
		T_12_3_r_Q3 = T_8_2_i_Q2 - T_12_2_i_Q2;
		p[12].nRealOut= (T_4_3_r_Q3 - T_12_3_r_Q3) >> 4;
		p[4].nRealOut = (T_4_3_r_Q3 + T_12_3_r_Q3) >> 4;
	}
	{
		ivInt32 T_2_3_r_Q3, T_6_3_i_Q3, T_10_3_r_Q3, T_14_3_i_Q3, T1Y_Q3, T23_Q3;
		// multiply -j, Real and Imag swaped
		T_2_3_r_Q3 = T_2_2_r_Q2 + T_6_2_i_Q2; 
		T_6_3_i_Q3 = T_2_2_i_Q2 - T_6_2_r_n_Q2;
		T1Y_Q3 = T_10_2_r_Q2 + T_10_2_i_Q2;
		T23_Q3 = T_14_2_r_n_Q2 - T_14_2_i_n_Q2;
		T_10_3_r_Q3 = FMUL_Q(fCos_4PI_16,(T1Y_Q3 + T23_Q3)>>3,3);
		T_14_3_i_Q3 = FMUL_Q(fCos_4PI_16,(T23_Q3 - T1Y_Q3)>>3,3);
		p[10].nRealOut= (T_2_3_r_Q3 - T_10_3_r_Q3) >> 4;
		p[6].nImageOut  = (T_6_3_i_Q3 + T_14_3_i_Q3) >> 4;
		p[2].nRealOut = (T_2_3_r_Q3 + T_10_3_r_Q3) >> 4;
		p[14].nImageOut  = (T_6_3_i_Q3 - T_14_3_i_Q3) >> 4;
	}
	{
		ivInt32 T_6_3_r_Q3, T_2_3_i_Q3, T_14_3_r_Q3, T_10_3_i_Q3, T2a_Q3, T2b_Q3;
		T_6_3_r_Q3 = T_2_2_r_Q2 - T_6_2_i_Q2;
		T_2_3_i_Q3 = T_6_2_r_n_Q2 + T_2_2_i_Q2;
		T2a_Q3 = T_10_2_i_Q2 - T_10_2_r_Q2;
		T2b_Q3 = T_14_2_r_n_Q2 + T_14_2_i_n_Q2;
		T_14_3_r_Q3 = FMUL_Q(fCos_4PI_16,(T2a_Q3 - T2b_Q3)>>3,3);
		T_10_3_i_Q3 = FMUL_Q(fCos_4PI_16,(T2a_Q3 + T2b_Q3)>>3,3);
		p[14].nRealOut = (T_6_3_r_Q3 - T_14_3_r_Q3) >> 4;
		p[2].nImageOut = (T_2_3_i_Q3 + T_10_3_i_Q3) >> 4;
		p[6].nRealOut = (T_6_3_r_Q3 + T_14_3_r_Q3) >> 4;
		p[10].nImageOut = (T_2_3_i_Q3 - T_10_3_i_Q3) >> 4;
	}
	{
		ivInt32 T_3_3_r_Q3, T_7_3_r_Q3, T_7_3_i_Q3, T_3_3_i_Q3, T_11_3_r_Q3, T_15_3_i_Q3, T_15_3_r_Q3, T_11_3_i_Q3, T_7_2_i_Q2, T_7_2_r_n_Q2;
		T_7_2_i_Q2 = FMUL_Q(fCos_4PI_16,(T_5_1_i_w_Q2 - T_7_1_i_w_Q2)>>2,2);
		T_3_3_r_Q3 = T_3_2_r_Q2 + T_7_2_i_Q2; 
		T_7_3_r_Q3 = T_3_2_r_Q2 - T_7_2_i_Q2;
		T_7_2_r_n_Q2 = FMUL_Q(fCos_4PI_16,(T_7_1_r_w_Q2 - T_5_1_r_w_Q2)>>2,2);
		// multiply -j
		T_7_3_i_Q3 = T_3_2_i_Q2 - T_7_2_r_n_Q2;
		T_3_3_i_Q3 = T_3_2_i_Q2 + T_7_2_r_n_Q2;
		{
			
			ivInt32 T_11_2_r_w_Q2, T1h_Q2, T_11_2_i_w_Q2, T1t_Q2;
			T_11_2_r_w_Q2 = FMA_Q(fCos_2PI_16, T_11_2_i_q_Q2 >> 1, fCos_6PI_16 * (T_11_2_r_q_Q2 >> 1),1);
			T1h_Q2 = FNMS_Q(fCos_2PI_16, (T_13_2_r_Q2 >> 1), fCos_6PI_16 * (T_15_2_i_n_Q2 >> 1),1);
			T_11_3_r_Q3 = T_11_2_r_w_Q2 + T1h_Q2;
			T_15_3_i_Q3 = T1h_Q2 - T_11_2_r_w_Q2;
			T_11_2_i_w_Q2 = FNMS_Q(fCos_2PI_16, (T_11_2_r_q_Q2 >> 1), fCos_6PI_16 * (T_11_2_i_q_Q2 >> 1),1);
			T1t_Q2 = FMA_Q(fCos_6PI_16, (T_13_2_r_Q2 >> 1), fCos_2PI_16 * (T_15_2_i_n_Q2 >> 1),1);
			T_15_3_r_Q3 = T_11_2_i_w_Q2 - T1t_Q2;
			T_11_3_i_Q3 = T_11_2_i_w_Q2 + T1t_Q2;
		}
		p[11].nRealOut = (T_3_3_r_Q3 - T_11_3_r_Q3) >> 4;
		p[11].nImageOut = (T_3_3_i_Q3 - T_11_3_i_Q3) >> 4;
		p[3].nRealOut = (T_3_3_r_Q3 + T_11_3_r_Q3) >> 4;
		p[3].nImageOut = (T_3_3_i_Q3 + T_11_3_i_Q3) >> 4;
		p[15].nImageOut = (T_7_3_i_Q3 - T_15_3_i_Q3) >> 4;
		p[15].nRealOut = (T_7_3_r_Q3 - T_15_3_r_Q3) >> 4;
		p[7].nImageOut = (T_7_3_i_Q3 + T_15_3_i_Q3) >> 4;
		p[7].nRealOut = (T_7_3_r_Q3 + T_15_3_r_Q3) >> 4;
	}
	{
		ivInt32 T_1_3_r_Q3, T_5_3_r_Q3, T_5_3_i_Q3, T_1_3_i_Q3, T_9_3_r_Q3, T_13_3_i_Q3, T_13_3_r_Q3, T_9_3_i_Q3, T_5_2_r_Q2, T_5_2_i_Q2;
		T_5_2_r_Q2 = FMUL_Q(fCos_4PI_16,((T_5_1_r_w_Q2 + T_7_1_r_w_Q2)>>1),1);
		T_1_3_r_Q3 = T_1_2_r_Q2 + T_5_2_r_Q2;
		T_5_3_r_Q3 = T_1_2_r_Q2 - T_5_2_r_Q2;
		T_5_2_i_Q2 = FMUL_Q(fCos_4PI_16,((T_5_1_i_w_Q2 + T_7_1_i_w_Q2)>>1),1);
		T_5_3_i_Q3 = T_1_2_i_Q2 - T_5_2_i_Q2;
		T_1_3_i_Q3 = T_1_2_i_Q2 + T_5_2_i_Q2;
		{
			ivInt32 T1C_Q2, T1F_Q2, T1M_Q2, T1N_Q2;
			T1C_Q2 = FMA_Q(fCos_6PI_16, T_9_2_i_q_Q2>>1, fCos_2PI_16 * (T_9_2_r_q_Q2>>1),1);
			T1F_Q2 = FNMS_Q(fCos_6PI_16, T_15_2_r_n_Q2 >> 1, fCos_2PI_16 * (T_13_2_i_Q2>>1),1);
			T_9_3_r_Q3 = T1C_Q2 + T1F_Q2;
			T_13_3_i_Q3 = T1F_Q2 - T1C_Q2;
			T1M_Q2 = FNMS_Q(fCos_6PI_16, T_9_2_r_q_Q2 >> 1, fCos_2PI_16 * (T_9_2_i_q_Q2 >> 1),1);
			T1N_Q2 = FMA_Q(fCos_2PI_16, T_15_2_r_n_Q2 >> 1, fCos_6PI_16 * (T_13_2_i_Q2>>1),1);
			T_13_3_r_Q3 = T1M_Q2 - T1N_Q2;
			T_9_3_i_Q3 = T1M_Q2 + T1N_Q2;
		}
		p[9].nRealOut = (T_1_3_r_Q3 - T_9_3_r_Q3) >> 4;
		p[9].nImageOut = (T_1_3_i_Q3 - T_9_3_i_Q3) >> 4;
		p[1].nRealOut = (T_1_3_r_Q3 + T_9_3_r_Q3) >> 4;
		p[1].nImageOut = (T_1_3_i_Q3 + T_9_3_i_Q3) >> 4;
		p[13].nImageOut = (T_5_3_i_Q3 - T_13_3_i_Q3) >> 4;
		p[13].nRealOut = (T_5_3_r_Q3 - T_13_3_r_Q3) >> 4;
		p[5].nImageOut = (T_5_3_i_Q3 + T_13_3_i_Q3) >> 4;
		p[5].nRealOut = (T_5_3_r_Q3 + T_13_3_r_Q3) >> 4;
	}
}

// Complex FFT to Real FFT
void FFT_Complex2Real(PFFTDesc p,ivInt32 nNode)
{
	ivInt32 nGap = TRANSFORM_FFTNUM_DEF/nNode/2;
	ivInt32 k,j;
	ivInt32 s16Real0,s16Imag0;
	ivInt32 s16RealTmp,s16ImagTmp;

	s16RealTmp = p[0].nRealOut;
	p[0].nRealOut += p[0].nImageOut;
	p[0].nImageOut = s16RealTmp-p[0].nImageOut;

	for (j = 1, k =  nNode-1; j<(nNode>>1);j++,--k)
	{
		ivInt32 s16RealYk, s16ImagYk, s16RealZk, s16ImagZk;

		s16Real0 = g_fCosTab[j*nGap];
		s16Imag0 = g_fCosTab[j*nGap+TRANSFORM_QUARTFFTNUM_DEF];

		s16RealYk = p[j].nRealOut + p[k].nRealOut;
		s16ImagZk = p[k].nRealOut - p[j].nRealOut;
		s16ImagYk = p[j].nImageOut - p[k].nImageOut;
		s16RealZk = p[j].nImageOut + p[k].nImageOut;

		s16ImagTmp = (s16RealZk*s16Imag0+s16ImagZk*s16Real0)>>15;
		s16RealTmp = (s16RealZk*s16Real0-s16ImagZk*s16Imag0)>>15;

		p[j].nRealOut = (s16RealTmp + s16RealYk + 1) >> 1;
		p[j].nImageOut = (s16ImagTmp + s16ImagYk + 1) >> 1;
		p[k].nRealOut = (-s16RealTmp + s16RealYk + 1) >> 1;
		p[k].nImageOut = (s16ImagTmp - s16ImagYk + 1) >> 1;
	}
	p[j].nImageOut = -p[j].nImageOut;
}

#if (256 == TRANSFORM_FFTNUM_DEF)

static void FFTW8_In_Block(PFFTDesc p,ivInt32 nRow)
{
	ivInt32 T_0_2_r_Q6, T_1_2_r_Q6, T_0_2_i_Q6, T_1_2_i_Q6, T_2_2_r_Q6, T_3_2_i_Q6, T_2_2_i_Q6, T_3_2_r_Q6;
	{
		ivInt32 T_0_1_r_Q5, T_1_1_r_Q5, T_0_1_i_Q5, T_1_1_i_Q5, T_2_1_r_Q5, T_3_1_i_Q5, T_2_1_i_Q5, T_3_1_r_Q5;
		{	
			ivInt32 X0_r_Q4, X0_i_Q4, X1_r_Q4, X1_i_Q4, X2_r_Q4, X2_i_Q4, X3_r_Q4, X3_i_Q4;
			//X0_r_Q4 = (p[0].nRealOut + 1) << 3;  // Q4
			//X0_i_Q4 = (p[0].nImageOut + 1) << 3;
			X0_r_Q4 = (p[0].nRealOut + 4) << FFTW_STEP_Q;  // Q4
			X0_i_Q4 = (p[0].nImageOut + 4) << FFTW_STEP_Q;
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[4*FFTW_BLOCK].nRealOut;
				IA = p[4*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[4*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[4*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF];
				X1_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X1_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[2*FFTW_BLOCK].nRealOut;
				IA = p[2*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[2*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[2*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF];
				X2_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X2_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[6*FFTW_BLOCK].nRealOut;
				IA = p[6*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[6*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[6*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF];
				X3_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X3_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}
			// (x[0],x[4],W0,W0)->(T_0_1,T_1_1)
			T_0_1_r_Q5 = X0_r_Q4 + X1_r_Q4;
			T_1_1_r_Q5 = X0_r_Q4 - X1_r_Q4;
			T_0_1_i_Q5 = X0_i_Q4 + X1_i_Q4;
			T_1_1_i_Q5 = X0_i_Q4 - X1_i_Q4;	
			// (x[2],x[6],W0,W2)->(T_2_1,T_3_1)
			T_2_1_r_Q5 = X2_r_Q4 + X3_r_Q4;
			T_3_1_i_Q5 = X3_r_Q4 - X2_r_Q4;
			T_2_1_i_Q5 = X2_i_Q4 + X3_i_Q4;
			T_3_1_r_Q5 = X2_i_Q4 - X3_i_Q4;

			// (T_0_1,T_2_1,W0,W0)->(T_0_2,T_2_2)
			T_0_2_r_Q6 = T_0_1_r_Q5 + T_2_1_r_Q5;
			T_2_2_r_Q6 = T_0_1_r_Q5 - T_2_1_r_Q5;
			T_0_2_i_Q6 = T_0_1_i_Q5 + T_2_1_i_Q5;
			T_2_2_i_Q6 = T_0_1_i_Q5 - T_2_1_i_Q5;
			// (T_1_1,T_3_1,W0,W0)->(T_1_2,T_3_2)
			T_1_2_r_Q6 = T_1_1_r_Q5 + T_3_1_r_Q5;
			T_3_2_r_Q6 = T_1_1_r_Q5 - T_3_1_r_Q5;
			T_1_2_i_Q6 = T_1_1_i_Q5 + T_3_1_i_Q5;
			T_3_2_i_Q6 = T_1_1_i_Q5 - T_3_1_i_Q5;
		}
	}
	{
		ivInt32 T_4_1_r_Q5, T_5_1_r_Q5, T_4_1_i_Q5, T_5_1_i_Q5, T_6_1_r_Q5, T_7_1_i_Q5, T_6_1_i_Q5, T_7_1_r_Q5;
		{ 
			ivInt32 X4_r_Q4, X4_i_Q4, X5_r_Q4, X5_i_Q4, X6_r_Q4, X6_i_Q4, X7_r_Q4, X7_i_Q4;
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[1*FFTW_BLOCK].nRealOut;
				IA = p[1*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[1*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[1*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF];
				X4_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X4_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[5*FFTW_BLOCK].nRealOut;
				IA = p[5*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[5*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[5*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF];
				X5_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X5_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[3*FFTW_BLOCK].nRealOut;
				IA = p[3*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[3*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[3*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF];
				X6_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X6_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}
			{
				ivInt32 RA, IA, Cos0, Sin0;
				RA = p[7*FFTW_BLOCK].nRealOut;
				IA = p[7*FFTW_BLOCK].nImageOut;
				// x[i] *= W(i*nRow,N_Out)
				Cos0 = g_fCosTab[7*TRANSFORM_FFTNUM_DEF/128*nRow];
				Sin0 = g_fCosTab[(7*TRANSFORM_FFTNUM_DEF/128*nRow+TRANSFORM_QUARTFFTNUM_DEF)&(TRANSFORM_FFTNUM_DEF-1)];
				X7_r_Q4 = FNMS_Q(Sin0, IA, Cos0 * RA, FFTW_STEP_Q);
				X7_i_Q4 = FMA_Q(Sin0, RA, Cos0 * IA, FFTW_STEP_Q);
			}

			// (T_4_1,T_5_1,W0,W0)->(T_0_2,T_2_2)
			T_4_1_r_Q5 = X4_r_Q4 + X5_r_Q4;
			T_5_1_r_Q5 = X4_r_Q4 - X5_r_Q4;
			T_4_1_i_Q5 = X4_i_Q4 + X5_i_Q4;
			T_5_1_i_Q5 = X4_i_Q4 - X5_i_Q4 + 32;	
			// (x[2],x[6],W0,W2)->(T_2_1,T_3_1)
			T_6_1_r_Q5 = X6_r_Q4 + X7_r_Q4;
			T_7_1_i_Q5 = X7_r_Q4 - X6_r_Q4;
			T_6_1_i_Q5 = X6_i_Q4 + X7_i_Q4;
			T_7_1_r_Q5 = X6_i_Q4 - X7_i_Q4;
		}
		//(T_4_1,T_6_1,W0,W8)->(T_4_2,T_6_2)
		{
			ivInt32 T_4_2_r_Q6, T_4_2_i_Q6, T_6_2_r_Q6, T_6_2_i_Q6;
			T_4_2_r_Q6 = T_4_1_r_Q5 + T_6_1_r_Q5;
			T_4_2_i_Q6 = T_4_1_i_Q5 + T_6_1_i_Q5;
			p[0*FFTW_BLOCK].nRealOut = (T_0_2_r_Q6 + T_4_2_r_Q6) >> FFTW_BLOCK_Q;
			p[0*FFTW_BLOCK].nImageOut = (T_0_2_i_Q6 + T_4_2_i_Q6) >> FFTW_BLOCK_Q;
			p[4*FFTW_BLOCK].nRealOut = (T_0_2_r_Q6 - T_4_2_r_Q6) >> FFTW_BLOCK_Q;
			p[4*FFTW_BLOCK].nImageOut = (T_0_2_i_Q6 - T_4_2_i_Q6) >> FFTW_BLOCK_Q;
			T_6_2_r_Q6 = T_4_1_i_Q5 - T_6_1_i_Q5;
			T_6_2_i_Q6 = T_6_1_r_Q5 - T_4_1_r_Q5;
			p[2*FFTW_BLOCK].nRealOut = (T_2_2_r_Q6 + T_6_2_r_Q6) >> FFTW_BLOCK_Q;
			p[2*FFTW_BLOCK].nImageOut = (T_2_2_i_Q6 + T_6_2_i_Q6) >> FFTW_BLOCK_Q;
			p[6*FFTW_BLOCK].nRealOut = (T_2_2_r_Q6 - T_6_2_r_Q6) >> FFTW_BLOCK_Q;
			p[6*FFTW_BLOCK].nImageOut = (T_2_2_i_Q6 - T_6_2_i_Q6) >> FFTW_BLOCK_Q;
		}

		{
			ivInt32 T_5_2_r_Q6, T_5_2_i_Q6, T_7_2_r_Q6, T_7_2_ni_Q6;
			{
				//(T_5_1,T_7_1,W4,W12)->(T_5_2,T_7_2)
				ivInt32 RA, RB, IA, IB;
				RA = T_5_1_r_Q5 + T_5_1_i_Q5;
				RB = T_7_1_r_Q5 + T_7_1_i_Q5;
				T_5_2_r_Q6 = FMUL_Q(fCos_4PI_16,(RA + RB) >> 7, 7);
				T_7_2_ni_Q6 = FMUL_Q(fCos_4PI_16,(RA - RB) >> 7, 7);
				IA = T_5_1_i_Q5 - T_5_1_r_Q5;
				IB = T_7_1_i_Q5 - T_7_1_r_Q5;
				T_5_2_i_Q6 = FMUL_Q(fCos_4PI_16,(IA + IB) >> 7, 7);
				T_7_2_r_Q6 = FMUL_Q(fCos_4PI_16,(IA - IB) >> 7, 7);
			}

			p[1*FFTW_BLOCK].nRealOut = (T_1_2_r_Q6 + T_5_2_r_Q6) >> FFTW_BLOCK_Q;
			p[1*FFTW_BLOCK].nImageOut = (T_1_2_i_Q6 + T_5_2_i_Q6) >> FFTW_BLOCK_Q;
			p[5*FFTW_BLOCK].nRealOut = (T_1_2_r_Q6 - T_5_2_r_Q6) >> FFTW_BLOCK_Q;
			p[5*FFTW_BLOCK].nImageOut = (T_1_2_i_Q6 - T_5_2_i_Q6) >> FFTW_BLOCK_Q;
			p[3*FFTW_BLOCK].nRealOut = (T_3_2_r_Q6 + T_7_2_r_Q6) >> FFTW_BLOCK_Q;
			p[3*FFTW_BLOCK].nImageOut = (T_3_2_i_Q6 - T_7_2_ni_Q6) >> FFTW_BLOCK_Q;
			p[7*FFTW_BLOCK].nRealOut = (T_3_2_r_Q6 - T_7_2_r_Q6) >> FFTW_BLOCK_Q;
			p[7*FFTW_BLOCK].nImageOut = (T_3_2_i_Q6 + T_7_2_ni_Q6) >> FFTW_BLOCK_Q;
		}
	}
}

void EsRealFFT256Core(PFFTDesc p,ivInt16 nQ)
{
    ivInt32 i;

    for(i = 0; i < 8; ++i){
        FFTW16_Step_Block(p+FFTW_BLOCK*i, nQ);
    }
    for(i = 0; i < FFTW_BLOCK; ++i){
        FFTW8_In_Block(p+i,i);
    }
    FFT_Complex2Real(p,128);

}

#elif (512 == TRANSFORM_FFTNUM_DEF)
static void FFTW16_In_Block(PFFTDesc p,ivInt32 nRow)
{
	ivInt32 T_0_1_r_Q5, T_1_1_i_Q5, T_1_1_r_Q5, T_0_1_i_Q5, T_2_1_r_Q5, T_3_1_r_Q5, T_3_1_i_Q5, T_2_1_i_Q5, T_4_1_r_Q5, T_4_1_i_Q5, T_5_1_i_w_Q6, T_5_1_r_w_Q6, T_6_1_r_Q5, T_6_1_i_Q5, T_7_1_i_w_Q6;
	ivInt32 T_7_1_r_w_Q6, T_14_1_r_Q5, T_12_1_r_Q5, T_14_2_r_n_Q6, T_14_1_i_Q5, T_12_1_i_Q5, T_14_2_i_n_Q6, T_15_2_i_n_Q6, T_13_2_i_Q6, T_13_2_r_Q6, T_15_2_r_n_Q6, T_8_1_r_Q5, T_10_1_r_Q5, T_10_2_r_Q6, T_8_1_i_Q5;
	ivInt32 T_10_1_i_Q5, T_10_2_i_Q6, T_11_2_i_q_Q6, T_9_2_i_q_Q6, T_11_2_r_q_Q6, T_9_2_r_q_Q6;
	{
		ivInt32 RA, IA, RB, IB;
		RA = (p[0].nRealOut + 8) << FFTW_STEP_Q;
		IA = (p[0].nImageOut + 8) << FFTW_STEP_Q;
		{
			ivInt32 r, i, Cos0, Sin0;

			r = p[8*FFTW_BLOCK].nRealOut;
			i = p[8*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[8*TRANSFORM_FFTNUM_DEF/256*nRow];		// cos0
			Sin0 = g_fCosTab[8*TRANSFORM_FFTNUM_DEF/256*nRow + TRANSFORM_QUARTFFTNUM_DEF]; // -sin0
			RB = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		T_0_1_r_Q5 = RA + RB;
		T_1_1_i_Q5 = IA - IB;
		T_1_1_r_Q5 = RA - RB;
		T_0_1_i_Q5 = IB + IA;
	}
	{
		ivInt32 RA, IA, RB, IB;
		{
			ivInt32 r, i, Cos0, Sin0;
			r =  p[4*FFTW_BLOCK].nRealOut;
			i = p[4*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[4*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 =  g_fCosTab[4*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RA = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[12*FFTW_BLOCK].nRealOut;
			i = p[12*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[12*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[12*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RB = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		T_2_1_r_Q5 = RA + RB;
		T_3_1_r_Q5 = RA - RB;
		T_3_1_i_Q5 = IA - IB;
		T_2_1_i_Q5 = IA + IB;
	}
	{
		ivInt32 RA, IA, RB, IB, T_5_1_i_Q5, T_5_1_r_Q5;
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[2*FFTW_BLOCK].nRealOut;
			i = p[2*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[2*TRANSFORM_FFTNUM_DEF/256*nRow];;
			Sin0 = g_fCosTab[2*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RA = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[10*FFTW_BLOCK].nRealOut;
			i = p[10*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[10*TRANSFORM_FFTNUM_DEF/256*nRow];;
			Sin0 = g_fCosTab[10*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RB = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		T_4_1_r_Q5 = RA + RB;
		T_4_1_i_Q5 = IA + IB;
		T_5_1_i_Q5 = IA - IB;
		T_5_1_r_Q5 = RA - RB;
		T_5_1_i_w_Q6 = T_5_1_i_Q5 - T_5_1_r_Q5;
		T_5_1_r_w_Q6 = T_5_1_r_Q5 + T_5_1_i_Q5;
	}
	{
		ivInt32 RB, IB, RA, IA, T_7_1_r_n_Q5, T_7_1_i_n_Q5;
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[14*FFTW_BLOCK].nRealOut;
			i = p[14*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[14*TRANSFORM_FFTNUM_DEF/256*nRow];;
			Sin0 = g_fCosTab[(14*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF)&(TRANSFORM_FFTNUM_DEF-1)];
			RB = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[6*FFTW_BLOCK].nRealOut;
			i = p[6*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[6*TRANSFORM_FFTNUM_DEF/256*nRow];;
			Sin0 = g_fCosTab[6*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RA = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		T_6_1_r_Q5 = RB + RA;
		T_6_1_i_Q5 = IB + IA;
		T_7_1_r_n_Q5 = RB - RA;
		T_7_1_i_n_Q5 = IB - IA;
		T_7_1_i_w_Q6 = T_7_1_r_n_Q5 + T_7_1_i_n_Q5;
		T_7_1_r_w_Q6 = T_7_1_r_n_Q5 - T_7_1_i_n_Q5; //(-1*-1, di xiao _n_)
	}
	{
		ivInt32 RB1, IB1, RB2, IB2, RA1, IA1, RA2, IA2;
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[15*FFTW_BLOCK].nRealOut;
			i = p[15*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[15*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[(15*TRANSFORM_FFTNUM_DEF/256*nRow + TRANSFORM_QUARTFFTNUM_DEF)&(TRANSFORM_FFTNUM_DEF-1)];
			RB1 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB1 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[11*FFTW_BLOCK].nRealOut;
			i = p[11*FFTW_BLOCK].nImageOut;;
			Cos0 = g_fCosTab[11*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[11*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RB2 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB2 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[7*FFTW_BLOCK].nRealOut;
			i = p[7*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[7*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[7*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RA1 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA1 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[3*FFTW_BLOCK].nRealOut;
			i = p[3*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[3*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[3*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RA2 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA2 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		T_14_1_r_Q5 = RB1 + RA1;
		T_12_1_r_Q5 = RA2 + RB2;
		T_14_2_r_n_Q6 = T_14_1_r_Q5 - T_12_1_r_Q5;
		T_14_1_i_Q5 = IB1 + IA1;
		T_12_1_i_Q5 = IA2 + IB2;
		T_14_2_i_n_Q6 = T_14_1_i_Q5 - T_12_1_i_Q5;
		{
			ivInt32 T_15_1_i_Q5, T_13_1_i_Q5, T_15_1_r_Q5, T_13_1_r_Q5;
			T_15_1_i_Q5 = RB1 - RA1;
			T_13_1_i_Q5 = IA2 - IB2;
			T_15_2_i_n_Q6 = T_15_1_i_Q5 - T_13_1_i_Q5;
			T_13_2_i_Q6 = T_15_1_i_Q5 + T_13_1_i_Q5;
			T_15_1_r_Q5 = IB1 - IA1;
			T_13_1_r_Q5 = RA2 - RB2;
			T_13_2_r_Q6 = T_15_1_r_Q5 + T_13_1_r_Q5;
			T_15_2_r_n_Q6 = T_15_1_r_Q5 - T_13_1_r_Q5;
		}
	}
	{
		ivInt32 RA1, IA1, RB2, IB2, RB1, IB1, RA2, IA2;
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[1*FFTW_BLOCK].nRealOut;
			i = p[1*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[1*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[1*TRANSFORM_FFTNUM_DEF/256*nRow + TRANSFORM_QUARTFFTNUM_DEF];
			RA1 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA1 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[13*FFTW_BLOCK].nRealOut;
			i = p[13*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[13*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[(13*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF)&(TRANSFORM_FFTNUM_DEF-1)];
			RB2 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB2 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[9*FFTW_BLOCK].nRealOut;
			i = p[9*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[9*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[9*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RB1 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IB1 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		{
			ivInt32 r, i, Cos0, Sin0;
			r = p[5*FFTW_BLOCK].nRealOut;
			i = p[5*FFTW_BLOCK].nImageOut;
			Cos0 = g_fCosTab[5*TRANSFORM_FFTNUM_DEF/256*nRow];
			Sin0 = g_fCosTab[5*TRANSFORM_FFTNUM_DEF/256*nRow+TRANSFORM_QUARTFFTNUM_DEF];
			RA2 = FNMS_Q(Sin0, i, Cos0 * r, FFTW_STEP_Q);
			IA2 = FMA_Q(Sin0, r, Cos0 * i, FFTW_STEP_Q);
		}
		T_8_1_r_Q5 = RA1 + RB1;
		T_10_1_r_Q5 = RA2 + RB2;
		T_10_2_r_Q6 = T_8_1_r_Q5 - T_10_1_r_Q5;
		T_8_1_i_Q5 = IA1 + IB1;
		T_10_1_i_Q5 = IA2 + IB2;
		T_10_2_i_Q6 = T_8_1_i_Q5 - T_10_1_i_Q5;
		{
			ivInt32 T_9_1_i_Q5, T_11_1_r_Q5, T_9_1_r_Q5, T_11_1_i_Q5;
			T_9_1_i_Q5 = IA1 - IB1;
			T_11_1_r_Q5 = RA2 - RB2;
			T_11_2_i_q_Q6 = T_9_1_i_Q5 + T_11_1_r_Q5;
			T_9_2_i_q_Q6 = T_9_1_i_Q5 - T_11_1_r_Q5;
			T_9_1_r_Q5 = RA1 - RB1;
			T_11_1_i_Q5 = IA2 - IB2;
			T_11_2_r_q_Q6 = T_9_1_r_Q5 - T_11_1_i_Q5;
			T_9_2_r_q_Q6 = T_9_1_r_Q5 + T_11_1_i_Q5;
		}
	}
	{
		ivInt32 T_3_3_r_Q7, T_7_3_r_Q7, T_3_3_i_Q7, T_7_3_i_Q7, T_11_3_r_Q7, T_15_3_i_Q7, T_15_3_r_Q7, T_11_3_i_Q7;
		{
			ivInt32 T_3_2_r_Q6, T_7_2_i_Q6, T_7_2_r_n_Q6, T_3_2_i_Q6;
			T_3_2_r_Q6 = T_1_1_r_Q5 - T_3_1_i_Q5;
			T_7_2_i_Q6 = FMUL_Q(fCos_4PI_16,(T_5_1_i_w_Q6 - T_7_1_i_w_Q6)>>6, 6);
			T_3_3_r_Q7 = T_3_2_r_Q6 + T_7_2_i_Q6;
			T_7_3_r_Q7 = T_3_2_r_Q6 - T_7_2_i_Q6;
			T_7_2_r_n_Q6 = FMUL_Q(fCos_4PI_16,(T_7_1_r_w_Q6 - T_5_1_r_w_Q6)>>6, 6);
			T_3_2_i_Q6 = T_3_1_r_Q5 + T_1_1_i_Q5;
			T_3_3_i_Q7 = T_7_2_r_n_Q6 + T_3_2_i_Q6;
			T_7_3_i_Q7 = T_3_2_i_Q6 - T_7_2_r_n_Q6;
		}
		{
			ivInt32 T_11_2_r_w_Q6, T25_Q6, T_11_2_i_w_Q6, T29_Q6;
			T_11_2_r_w_Q6 = FMA_Q(fCos_2PI_16, T_11_2_i_q_Q6 >> 6, fCos_6PI_16 * (T_11_2_r_q_Q6 >> 6), 6);
			T25_Q6 = FNMS_Q(fCos_2PI_16, T_13_2_r_Q6 >> 6, fCos_6PI_16 * (T_15_2_i_n_Q6 >> 6), 6);
			T_11_3_r_Q7 = T_11_2_r_w_Q6 + T25_Q6;
			T_15_3_i_Q7 = T25_Q6 - T_11_2_r_w_Q6;
			T_11_2_i_w_Q6 = FNMS_Q(fCos_2PI_16, T_11_2_r_q_Q6 >> 6, fCos_6PI_16 * (T_11_2_i_q_Q6 >> 6), 6);
			T29_Q6 = FMA_Q(fCos_6PI_16, T_13_2_r_Q6 >> 6, fCos_2PI_16 * (T_15_2_i_n_Q6 >> 6), 6);
			T_15_3_r_Q7 = T_11_2_i_w_Q6 - T29_Q6;
			T_11_3_i_Q7 = T_11_2_i_w_Q6 + T29_Q6;
		}
		p[11*FFTW_BLOCK].nRealOut= (T_3_3_r_Q7 - T_11_3_r_Q7) >> FFTW_BLOCK_Q;
		p[11*FFTW_BLOCK].nImageOut = (T_3_3_i_Q7 - T_11_3_i_Q7) >> FFTW_BLOCK_Q;
		p[3*FFTW_BLOCK].nRealOut = (T_3_3_r_Q7 + T_11_3_r_Q7) >> FFTW_BLOCK_Q;
		p[3*FFTW_BLOCK].nImageOut = (T_11_3_i_Q7 + T_3_3_i_Q7) >> FFTW_BLOCK_Q;
		p[15*FFTW_BLOCK].nRealOut = (T_7_3_r_Q7 - T_15_3_r_Q7) >> FFTW_BLOCK_Q;
		p[15*FFTW_BLOCK].nImageOut = (T_7_3_i_Q7 - T_15_3_i_Q7) >> FFTW_BLOCK_Q;
		p[7*FFTW_BLOCK].nRealOut = (T_7_3_r_Q7 + T_15_3_r_Q7) >> FFTW_BLOCK_Q;
		p[7*FFTW_BLOCK].nImageOut = (T_15_3_i_Q7 + T_7_3_i_Q7) >> FFTW_BLOCK_Q;
	}
	{
		ivInt32 T_2_3_r_Q7, T_6_3_r_Q7, T_2_3_i_Q7, T_6_3_i_Q7, T_10_3_r_Q7, T_14_3_i_Q7, T_14_3_r_Q7, T_10_3_i_Q7;
		{
			ivInt32 T_2_2_r_Q6, T_6_2_i_Q6, T_6_2_r_n_Q6, T_2_2_i_Q6;
			T_2_2_r_Q6 = T_0_1_r_Q5 - T_2_1_r_Q5;
			T_6_2_i_Q6 = T_4_1_i_Q5 - T_6_1_i_Q5;
			T_2_3_r_Q7 = T_2_2_r_Q6 + T_6_2_i_Q6;
			T_6_3_r_Q7 = T_2_2_r_Q6 - T_6_2_i_Q6;
			T_6_2_r_n_Q6 = T_6_1_r_Q5 - T_4_1_r_Q5;
			T_2_2_i_Q6 = T_0_1_i_Q5 - T_2_1_i_Q5;
			T_2_3_i_Q7 = T_6_2_r_n_Q6 + T_2_2_i_Q6;
			T_6_3_i_Q7 = T_2_2_i_Q6 - T_6_2_r_n_Q6;
		}
		{
			ivInt32 T2A_Q7, T2F_Q7, T2I_Q7, T2J_Q7;
			T2A_Q7 = T_10_2_r_Q6 + T_10_2_i_Q6;
			T2F_Q7 = T_14_2_r_n_Q6 - T_14_2_i_n_Q6;
			T_10_3_r_Q7 = FMUL_Q(fCos_4PI_16,(T2A_Q7 + T2F_Q7)>>7,7);
			T_14_3_i_Q7 = FMUL_Q(fCos_4PI_16,(T2F_Q7 - T2A_Q7)>>7,7);

			T2I_Q7 = T_10_2_i_Q6 - T_10_2_r_Q6;
			T2J_Q7 = T_14_2_r_n_Q6 + T_14_2_i_n_Q6;
			T_14_3_r_Q7 = FMUL_Q(fCos_4PI_16,(T2I_Q7 - T2J_Q7)>>7, 7);
			T_10_3_i_Q7 = FMUL_Q(fCos_4PI_16,(T2I_Q7 + T2J_Q7)>>7, 7);
		}
		p[10*FFTW_BLOCK].nRealOut= (T_2_3_r_Q7 - T_10_3_r_Q7) >> FFTW_BLOCK_Q;
		p[10*FFTW_BLOCK].nImageOut = (T_2_3_i_Q7 - T_10_3_i_Q7) >> FFTW_BLOCK_Q;
		p[2*FFTW_BLOCK].nRealOut = (T_2_3_r_Q7 + T_10_3_r_Q7) >> FFTW_BLOCK_Q;
		p[2*FFTW_BLOCK].nImageOut = (T_10_3_i_Q7 + T_2_3_i_Q7) >> FFTW_BLOCK_Q;
		p[14*FFTW_BLOCK].nRealOut = (T_6_3_r_Q7 - T_14_3_r_Q7) >> FFTW_BLOCK_Q;
		p[14*FFTW_BLOCK].nImageOut = (T_6_3_i_Q7 - T_14_3_i_Q7) >> FFTW_BLOCK_Q;
		p[6*FFTW_BLOCK].nRealOut = (T_6_3_r_Q7 + T_14_3_r_Q7) >> FFTW_BLOCK_Q;
		p[6*FFTW_BLOCK].nImageOut = (T_14_3_i_Q7 + T_6_3_i_Q7) >> FFTW_BLOCK_Q;
	}
	{
		ivInt32 T_1_3_r_Q7, T_5_3_r_Q7, T_1_3_i_Q7, T_5_3_i_Q7, T_9_3_r_Q7, T_13_3_i_Q7, T_13_3_r_Q7, T_9_3_i_Q7;
		{
			ivInt32 T_1_2_r_Q6, T_5_2_r_Q6, T_5_2_i_Q6, T_1_2_i_Q6;
			T_1_2_r_Q6 = T_1_1_r_Q5 + T_3_1_i_Q5;
			T_5_2_r_Q6 = FMUL_Q(fCos_4PI_16,((T_5_1_r_w_Q6 + T_7_1_r_w_Q6)>>6),6);
			T_1_3_r_Q7 = T_1_2_r_Q6 + T_5_2_r_Q6;
			T_5_3_r_Q7 = T_1_2_r_Q6 - T_5_2_r_Q6;
			T_5_2_i_Q6 = FMUL_Q(fCos_4PI_16,((T_5_1_i_w_Q6 + T_7_1_i_w_Q6)>>6), 6);
			T_1_2_i_Q6 = T_1_1_i_Q5 - T_3_1_r_Q5;
			T_1_3_i_Q7 = T_5_2_i_Q6 + T_1_2_i_Q6;
			T_5_3_i_Q7 = T_1_2_i_Q6 - T_5_2_i_Q6;
		}
		{
			ivInt32 T2i_Q6, T2l_Q6, T2o_Q6, T2p_Q6;
			T2i_Q6 = FMA_Q(fCos_6PI_16, T_9_2_i_q_Q6 >> 6, fCos_2PI_16 * (T_9_2_r_q_Q6 >> 6), 6);
			T2l_Q6 = FNMS_Q(fCos_6PI_16, T_15_2_r_n_Q6 >> 6, fCos_2PI_16 * (T_13_2_i_Q6 >> 6), 6);
			T_9_3_r_Q7 = T2i_Q6 + T2l_Q6;
			T_13_3_i_Q7 = T2l_Q6 - T2i_Q6;
			T2o_Q6 = FNMS_Q(fCos_6PI_16, T_9_2_r_q_Q6 >> 6, fCos_2PI_16 * (T_9_2_i_q_Q6 >> 6), 6);
			T2p_Q6 = FMA_Q(fCos_2PI_16, T_15_2_r_n_Q6 >> 6, fCos_6PI_16 * (T_13_2_i_Q6 >> 6), 6);
			T_13_3_r_Q7 = T2o_Q6 - T2p_Q6;
			T_9_3_i_Q7 = T2o_Q6 + T2p_Q6;
		}
		p[9*FFTW_BLOCK].nRealOut = (T_1_3_r_Q7 - T_9_3_r_Q7) >> FFTW_BLOCK_Q;
		p[9*FFTW_BLOCK].nImageOut = (T_1_3_i_Q7 - T_9_3_i_Q7) >> FFTW_BLOCK_Q;
		p[1*FFTW_BLOCK].nRealOut = (T_1_3_r_Q7 + T_9_3_r_Q7) >> FFTW_BLOCK_Q;
		p[1*FFTW_BLOCK].nImageOut = (T_9_3_i_Q7 + T_1_3_i_Q7) >> FFTW_BLOCK_Q;
		p[13*FFTW_BLOCK].nRealOut = (T_5_3_r_Q7 - T_13_3_r_Q7) >> FFTW_BLOCK_Q;
		p[13*FFTW_BLOCK].nImageOut = (T_5_3_i_Q7 - T_13_3_i_Q7) >> FFTW_BLOCK_Q;
		p[5*FFTW_BLOCK].nRealOut = (T_5_3_r_Q7 + T_13_3_r_Q7) >> FFTW_BLOCK_Q;
		p[5*FFTW_BLOCK].nImageOut = (T_13_3_i_Q7 + T_5_3_i_Q7) >> FFTW_BLOCK_Q;
	}
	{
		ivInt32 T_0_3_r_Q7, T_4_3_r_Q7, T_0_3_i_Q7, T_4_3_i_Q7, T_8_3_r_Q7, T_12_3_i_Q7, T_12_3_r_Q7, T_8_3_i_Q7;
		{
			ivInt32 T_0_2_r_Q6, T_4_2_r_Q6, T_4_2_i_Q6, T_0_2_i_Q6;
			T_0_2_r_Q6 = T_0_1_r_Q5 + T_2_1_r_Q5;
			T_4_2_r_Q6 = T_4_1_r_Q5 + T_6_1_r_Q5;
			T_0_3_r_Q7 = T_0_2_r_Q6 + T_4_2_r_Q6;
			T_4_3_r_Q7 = T_0_2_r_Q6 - T_4_2_r_Q6;
			T_4_2_i_Q6 = T_4_1_i_Q5 + T_6_1_i_Q5;
			T_0_2_i_Q6 = T_2_1_i_Q5 + T_0_1_i_Q5;
			T_0_3_i_Q7 = T_4_2_i_Q6 + T_0_2_i_Q6;
			T_4_3_i_Q7 = T_0_2_i_Q6 - T_4_2_i_Q6;
		}
		{
			ivInt32 T_8_2_r_Q6, T_12_2_r_Q6, T_8_2_i_Q6, T_12_2_i_Q6;
			T_8_2_r_Q6 = T_8_1_r_Q5 + T_10_1_r_Q5;
			T_12_2_r_Q6 = T_14_1_r_Q5 + T_12_1_r_Q5;
			T_8_3_r_Q7 = T_8_2_r_Q6 + T_12_2_r_Q6;
			T_12_3_i_Q7 = T_12_2_r_Q6 - T_8_2_r_Q6;
			T_8_2_i_Q6 = T_8_1_i_Q5 + T_10_1_i_Q5;
			T_12_2_i_Q6 = T_14_1_i_Q5 + T_12_1_i_Q5;
			T_12_3_r_Q7 = T_8_2_i_Q6 - T_12_2_i_Q6;
			T_8_3_i_Q7 = T_8_2_i_Q6 + T_12_2_i_Q6;
		}
		p[8*FFTW_BLOCK].nRealOut = (T_0_3_r_Q7 - T_8_3_r_Q7) >> FFTW_BLOCK_Q;
		p[8*FFTW_BLOCK].nImageOut = (T_0_3_i_Q7 - T_8_3_i_Q7) >> FFTW_BLOCK_Q;
		p[0].nRealOut = (T_0_3_r_Q7 + T_8_3_r_Q7) >> FFTW_BLOCK_Q;
		p[0].nImageOut = (T_8_3_i_Q7 + T_0_3_i_Q7) >> FFTW_BLOCK_Q;
		p[12*FFTW_BLOCK].nRealOut = (T_4_3_r_Q7 - T_12_3_r_Q7) >> FFTW_BLOCK_Q;
		p[12*FFTW_BLOCK].nImageOut = (T_4_3_i_Q7 - T_12_3_i_Q7) >> FFTW_BLOCK_Q;
		p[4*FFTW_BLOCK].nRealOut = (T_4_3_r_Q7 + T_12_3_r_Q7) >> FFTW_BLOCK_Q;
		p[4*FFTW_BLOCK].nImageOut = (T_12_3_i_Q7 + T_4_3_i_Q7) >> FFTW_BLOCK_Q;
	}
}

void EsRealFFT512Core(PFFTDesc p,ivInt16 nQ)
{
    ivInt32 i;

    for(i = 0; i < 16; ++i) {
        FFTW16_Step_Block(p+FFTW_BLOCK*i, nQ);
    }
    for(i = 0;i < FFTW_BLOCK; ++i) {
        FFTW16_In_Block(p+i,i);
    }
    FFT_Complex2Real(p,256);

}

#endif /* #if (256 == TRANSFORM_FFTNUM_DEF) */

#endif /* !MINI5_USE_FFTW */
