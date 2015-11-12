/*----------------------------------------------------------------------------------------*/
/*                                                                                        */
/* Copyright(c) 2011 Nuvoton Technology Corp. All rights reserved.                        */
/*                                                                                        */
/*----------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------*/
/* Include related headers                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#include "./common/defs.h"
#include "lib/LibSiren7.h"

/*----------------------------------------------------------------------------------------*/
/* Global variables                                                                       */
/*----------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------*/
/* Define functions prototype                                                             */
/*----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------*/
/* Function: LibS7Init      		                                      			   	  */
/*                                                                                        */
/* Parameters:                                                                            */
/*               sCodecCtl: Information of encoder/decoder's control value                */
/*               w32BitRate:Bit Rate, range from 4K~32K			                    	  */
/*               w16Bandwidth: 7K or 14K 						                    	  */
/* Returns:                                                                               */
/*               None											                    	  */
/* Description:                                                                           */
/*               Initialize encoder/decoder's control values      				          */
/*----------------------------------------------------------------------------------------*/
void LibS7Init(sSiren7_CODEC_CTL *sCodecCtl, Word32 w32BitRate, Word16 w16Bandwidth)
{
	sCodecCtl->bit_rate = (signed long)w32BitRate;
	sCodecCtl->bandwidth = (signed short)w16Bandwidth;
	sCodecCtl->number_of_bits_per_frame = (signed short)(sCodecCtl->bit_rate/50);
	if(sCodecCtl->bandwidth == 7000)
	{
		sCodecCtl->number_of_regions = NUMBER_OF_REGIONS;
		sCodecCtl->frame_size = MAX_FRAMESIZE >> 1;
	}
	else if(sCodecCtl->bandwidth == 14000)
	{
		sCodecCtl->number_of_regions = MAX_NUMBER_OF_REGIONS;
		sCodecCtl->frame_size = MAX_FRAMESIZE;
	}
}


/*----------------------------------------------------------------------------------------*/
/* Function: LibS7EnBufReset 		                                      			   	  */
/*                                                                                        */
/* Parameters:                                                                            */
/*               ssFrameSize : Encoder/Decoder's control value frame_size                 */
/*               sS7EncCtx   : Structure of storing encoder's history value	              */
/* Returns:                                                                               */
/*               None											                    	  */
/* Description:                                                                           */
/*               Reset encoder's history buffer values to zero      				      */
/*----------------------------------------------------------------------------------------*/
//void LibS7EnBufReset(signed short ssFrameSize, sSiren7_ENC_CTX *sS7EncCtx)
//{
//	int iCount;
//    for(iCount=0; iCount<ssFrameSize; iCount++)
//    {
//	    sS7EncCtx->history[iCount] = 0;
//	}    
//}

/*----------------------------------------------------------------------------------------*/
/* Function: LibS7Encode    		                                      			   	  */
/*                                                                                        */
/* Parameters:                                                                            */
/*               sCctl: Information of encoder/decoder's control value                    */
/*               sS7EncCtx   : Structure of storing encoder's history value	              */
/*               w16InData: Input encoding data         		                    	  */
/*               w16OutData:Output encoded data         		                    	  */
/* Returns:                                                                               */
/*               None											                    	  */
/* Description:                                                                           */
/*               Encode the raw data as Siren7 format           				          */
/*----------------------------------------------------------------------------------------*/
//void LibS7Encode(const sSiren7_CODEC_CTL *sCctl, sSiren7_ENC_CTX *sS7EncCtx, 
//                Word16 *w16InData, Word16 *w16OutData)
//{
//	Word16 w16Mag_shift;
//
//	/* Convert input samples to rmlt coefs */
//	w16Mag_shift = samples_to_rmlt_coefs(w16InData, sS7EncCtx->history, 
//	                                     sS7EncCtx->mlt_coefs, sCctl->frame_size);
//
//	/* Encode the mlt coefs */
//	encoder(sCctl->number_of_bits_per_frame,
//			sCctl->number_of_regions,
//			sS7EncCtx->mlt_coefs,
//			w16Mag_shift,
//			w16OutData);
//}


/*----------------------------------------------------------------------------------------*/
/* Function: LibS7DeBufReset  		                                      			   	  */
/*                                                                                        */
/* Parameters:                                                                            */
/*               ssFrameSize : Encoder/Decoder's control value frame_size                 */
/*               sS7DecCtx   : Structure of storing decoder's history value	              */
/* Returns:                                                                               */
/*               None											                    	  */
/* Description:                                                                           */
/*               Reset decoder's history buffer values to zero    				          */
/*----------------------------------------------------------------------------------------*/
void LibS7DeBufReset(signed short ssFrameSize, sSiren7_DEC_CTX *sS7DecCtx)
{
	int iCount;

#ifndef NO_FRAME_ERROR_CHECK
	for(iCount=0; iCount<ssFrameSize; iCount++)
	{
		sS7DecCtx->old_decoder_mlt_coefs[iCount] = 0;
	}
#endif

	for(iCount=0; iCount<(ssFrameSize >> 1); iCount++)
	{
		sS7DecCtx->old_samples[iCount] = 0;
	}

	/* initialize the random number generator */
	sS7DecCtx->randobj.seed0 = 1;
	sS7DecCtx->randobj.seed1 = 1;
	sS7DecCtx->randobj.seed2 = 1;
	sS7DecCtx->randobj.seed3 = 1;
}

/*----------------------------------------------------------------------------------------*/
/* Function: LibS7Decode    		                                      			   	  */
/*                                                                                        */
/* Parameters:                                                                            */
/*               sCctl: Information of encoder/decoder's control value                    */
/*               sS7DecCtx   : Structure of storing decoder's history value	              */
/*               w16InData: Input decoding data         		                    	  */
/*               w16OutData:Output decoded data         		                    	  */
/* Returns:                                                                               */
/*               None											                    	  */
/* Description:                                                                           */
/*               Decode the compression data as normal format      				          */
/*----------------------------------------------------------------------------------------*/
void LibS7Decode(const sSiren7_CODEC_CTL *sCctl, sSiren7_DEC_CTX *sS7DecCtx, 
                 Word16 *w16InData, Word16 *w16OutData)
{
	int iCount;
	Word16 w16FrameErrFlag = 0;
	Word16 w16MagShift;
	Bit_Obj sBitobj;

	/* reinit the current word to point to the start of the buffer */
	sBitobj.code_word_ptr = w16InData;
	sBitobj.current_word =  (*w16InData);
	sBitobj.code_bit_count = 0;
	sBitobj.number_of_bits_left = sCctl->number_of_bits_per_frame;

	/* process the out_words into decoder_mlt_coefs */
	decoder(&sBitobj,
			&(sS7DecCtx->randobj),
			sCctl->number_of_regions,
			sS7DecCtx->decoder_mlt_coefs,
			&w16MagShift,
			&(sS7DecCtx->old_mag_shift),
#ifndef NO_FRAME_ERROR_CHECK
			sS7DecCtx->old_decoder_mlt_coefs,
#endif
			&w16FrameErrFlag);

	/* convert the decoder_mlt_coefs to samples */
	rmlt_coefs_to_samples(sS7DecCtx->decoder_mlt_coefs, sS7DecCtx->old_samples,
	                      w16OutData, sCctl->frame_size, w16MagShift);

	/* For ITU testing, off the 2 lsbs. */
	for (iCount=0; iCount<sCctl->frame_size; iCount++)
	{
		w16OutData[iCount] &= 0xfffc;
	}
}

