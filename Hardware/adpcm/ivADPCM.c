
#include "ivADPCM.h"

static const ivInt8 index_adjust[8] = {-1,-1,-1,-1,2,4,6,8};

static const ivInt16 step_table[89] =
{
	7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
	50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,
	408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,
	2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,
	10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767
};

ivSize ivCall ivADPCM_Encode(ivPADPCMEncoder pEncoder, ivPInt16 pPcmIn, ivSize nSamples, ivPUInt8 pCodeOut)
{
	ivInt32 prev_sample;
	ivInt32 index;
	ivInt32 delta;
	ivInt32 sb;
	ivInt32 step;
	ivInt32 vpdiff;
	ivInt32 code;
	ivInt16 pcm;
	ivInt8 flag;
	ivInt8 adpcm;
	ivSize i;
	
	prev_sample = pEncoder->m_nPrevVal;
	index = pEncoder->m_nIndex;
	
	flag = 0;
	for(i = 0; i < nSamples; i++)
	{
		pcm = *pPcmIn++;
		delta = pcm - prev_sample;
		if(delta < 0)
		{
			delta = -delta;
			sb = 8;
		}
		else
		{
			sb = 0;
		}
		//code = 4 * delta / step_table[index];
		//if (code > 7) {
		//	code = 7;
		//}
		//vpdiff = (code/* + 0.5*/) * step_table[index] / 4;
		//vpdiff = (step_table[index] * yamaha_difflookup[code]) / 8;
		code = 0;
		step = step_table[index];
		vpdiff = (step >> 3);
		if(delta >= step)
		{
			code = 4;
			delta -= step;
			vpdiff += step;
		}
		step >>= 1;
		if(delta >= step)
		{
			code |= 2;
			delta -= step;
			vpdiff += step;
		}
		step >>= 1;
		if(delta >= step)
		{
			code |= 1;
			vpdiff += step;
		}
		index += index_adjust[code];
		if(index < 0) index = 0;
		if(index > 88) index = 88;
		if(sb)
		{
			prev_sample -= vpdiff;
		}
		else
		{
			prev_sample += vpdiff;
		}
		if(prev_sample > 32767)
			prev_sample = 32767;
		else if(prev_sample < -32768)
			prev_sample = -32768;
		if(flag == 0)
		{
			adpcm = (code | sb) << 4;
			flag = 1;
		}
		else
		{
			adpcm |= (code | sb);
			flag = 0;
			*pCodeOut++ = adpcm;
		}
	}
	pEncoder->m_nPrevVal = (ivInt16)prev_sample;
	pEncoder->m_nIndex = (ivInt8)index;
	return (ivSize)nSamples; 
}

ivSize ivCall ivADPCM_Decode(ivPADPCMDecoder pDecoder, ivPUInt8 pCodeIn, ivSize nCodeSize, ivPInt16 pPcmOut)
{
	ivInt32 curr_sample;
	ivInt32 index;
	ivInt32 code;
	ivInt32 sb;
	ivInt32 step;
	ivInt32 samples;
	ivInt16 vpdiff;
	ivInt8 flag;
	ivSize i;
	
	samples = nCodeSize * 2;
	curr_sample = pDecoder->m_nPrevVal;
	index = pDecoder->m_nIndex;
	flag = 1;
	for(i = 0; i < samples; i++)
	{
		if(flag)
		{
			code = (*pCodeIn & 0x0F0) >> 4;
			flag = 0;
		}
		else
		{
			code = (*pCodeIn++ & 0x0F);
			flag = 1;
		}
		if(code & 8)
			sb = 1;
		else
			sb = 0;
			
		code &= 7;
		//vpdiff = (code + 0.5) * step_table[index] / 4;
		step = step_table[index];
		vpdiff = step >> 3;
		if(code & 4) vpdiff += step;
		if(code & 2) vpdiff += step >> 1;
		if(code & 1) vpdiff += step >> 2;
		if(sb)
			curr_sample -= vpdiff;
		else
			curr_sample += vpdiff;
			
		if(curr_sample >= 32767)
			curr_sample = 32767;
		else if(curr_sample <= -32768)
			curr_sample = -32768;
		*pPcmOut++ = curr_sample;
		index += index_adjust[code];
		if(index < 0) index = 0;
		if(index > 88) index = 88;
	}
	pDecoder->m_nPrevVal = (ivInt16)curr_sample;
	pDecoder->m_nIndex = (ivUInt16)index;
	return 	samples;
}
