/***********************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   ?2004 Polycom, Inc.
**
**   All rights reserved.
**
***********************************************************************/
#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "typedef.h"
#include "basop32.h"

#ifndef PI
#define  PI             3.141592653589793238462
#endif

#ifdef SIREN14
#define MAX_DCT_LENGTH      640
#else
#define MAX_DCT_LENGTH  DCT_LENGTH
#endif

#define DCT_LENGTH          320
#define DCT_LENGTH_DIV_2    160
#define DCT_LENGTH_DIV_4     80
#define DCT_LENGTH_DIV_8     40
#define DCT_LENGTH_DIV_16    20
#define DCT_LENGTH_DIV_32    10
#define DCT_LENGTH_DIV_64     5


#define MAX(a,b) (a > b ? a : b)
#define MIN(a,b) (a < b ? a : b)

#define NUM_CATEGORIES                  8
#define NUM_CATEGORIZATION_CONTROL_BITS           4
#define NUM_CATEGORIZATION_CONTROL_POSSIBILITIES  16
#define CORE_SIZE       10
#define DCT_LENGTH_LOG  6
#define MAX_DCT_LENGTH_LOG  7

/*  region_size = (BLOCK_SIZE * 0.875)/NUM_REGIONS; */
#define NUMBER_OF_REGIONS       14
//#define NUMBER_OF_REGIONS       16		 // To expand BW
#ifdef SIREN14
#define MAX_NUMBER_OF_REGIONS   28
#else
#define MAX_NUMBER_OF_REGIONS NUMBER_OF_REGIONS
#endif

#define REGION_SIZE             20
#define NUMBER_OF_VALID_COEFS   (NUMBER_OF_REGIONS * REGION_SIZE)
#ifdef SIREN14
#define MAX_NUMBER_OF_VALID_COEFS   (MAX_NUMBER_OF_REGIONS * REGION_SIZE)
#else
#define MAX_NUMBER_OF_VALID_COEFS NUMBER_OF_VALID_COEFS
#endif

#define REGION_POWER_TABLE_SIZE 64
#define REGION_POWER_TABLE_NUM_NEGATIVES 24

#ifdef SIREN14
#define MAX_NUM_CATEGORIZATION_CONTROL_BITS 5
#define MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES 32
#else
#define MAX_NUM_CATEGORIZATION_CONTROL_BITS NUM_CATEGORIZATION_CONTROL_BITS
#define MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES NUM_CATEGORIZATION_CONTROL_POSSIBILITIES
#endif

#define ENCODER_SCALE_FACTOR 18318.0

/* The MLT output is incorrectly scaled by the factor
   product of ENCODER_SCALE_FACTOR and sqrt(160.)
   This is now (9/30/96) 1.0/2^(4.5) or 1/22.627.
   In the current implementation this  
   must be an integer power of sqrt(2). The
   integer power is ESF_ADJUSTMENT_TO_RMS_INDEX.
   The -2 is to conform with the range defined in the spec. */

 
#define ESF_ADJUSTMENT_TO_RMS_INDEX (9-2)
 

#define INTERMEDIATE_FILES_FLAG 0

#ifdef SIREN14
/* Max bit rate is 48000 bits/sec. */
#define MAX_BITS_PER_FRAME 960
#else
/* Max bit rate is 32000 bits/sec. */
#define MAX_BITS_PER_FRAME 640
#endif

#ifdef SIREN14
#define MAX_SAMPLE_RATE 32000
#else
#define MAX_SAMPLE_RATE 16000
#endif
#define MAX_FRAMESIZE   (MAX_SAMPLE_RATE/50)

/***************************************************************************/
/* Type definitions                                                        */
/***************************************************************************/

#pragma pack(4)

typedef struct
{
    Word16 code_bit_count;      /* bit count of the current word */
    Word16 current_word;        /* current word in the bitstream being processed */
    Word16 *code_word_ptr;      /* pointer to the bitstream */
    Word16 number_of_bits_left; /* number of bits left in the current word */
    Word16 next_bit;            /* next bit in the current word */
}Bit_Obj;

#ifndef Rand_Obj_defined
#define Rand_Obj_defined
typedef struct
{
    Word16 seed0;
    Word16 seed1;
    Word16 seed2;
    Word16 seed3;
}Rand_Obj;
#endif // Rand_Obj_defined

#pragma pack()

//------dct4_s.c
typedef struct	
{
	Word16   buffer_a[MAX_DCT_LENGTH];
    Word16   *in_ptr;
    Word16   *in_ptr_low;
    Word16   *in_ptr_high;
    Word16   *next_in_base;
    Word16   *temp_ptr;
    Word16   *out_ptr_low;
    Word16   *out_ptr_high;
    Word16   *next_out_base;
    Word16   *out_buffer;
    Word16   *in_buffer;
    Word16   *buffer_swap;
	Word16   out_val_low;
	Word16   out_val_high;
    Word16   *pair_ptr;       
	Word16   set_span;
	Word16   pairs_left;
	Word16   sets_left;
	Word32   dct4s_index;
	Word32   dct4s_dummy;
	Word32 	 dct4s_sum;
	Word32   dct_length_log;
}tsDct_type_iv_s;

extern tsDct_type_iv_s sDct_type_iv_s;

#define buffer_a 		sDct_type_iv_s.buffer_a
#define in_ptr 			sDct_type_iv_s.in_ptr
#define in_ptr_low 		sDct_type_iv_s.in_ptr_low
#define in_ptr_high 	sDct_type_iv_s.in_ptr_high
#define next_in_base 	sDct_type_iv_s.next_in_base
#define temp_ptr 		sDct_type_iv_s.temp_ptr
#define out_ptr_low 	sDct_type_iv_s.out_ptr_low
#define out_ptr_high 	sDct_type_iv_s.out_ptr_high
#define next_out_base 	sDct_type_iv_s.next_out_base
#define out_buffer 		sDct_type_iv_s.out_buffer
#define in_buffer 		sDct_type_iv_s.in_buffer
#define buffer_swap 	sDct_type_iv_s.buffer_swap
#define out_val_low 	sDct_type_iv_s.out_val_low
#define out_val_high 	sDct_type_iv_s.out_val_high
#define pair_ptr 		sDct_type_iv_s.pair_ptr
#define set_span 		sDct_type_iv_s.set_span
#define pairs_left 		sDct_type_iv_s.pairs_left
#define sets_left 		sDct_type_iv_s.sets_left
#define dct4s_index		sDct_type_iv_s.dct4s_index
#define dct4s_dummy		sDct_type_iv_s.dct4s_dummy
#define dct4s_sum 		sDct_type_iv_s.dct4s_sum
#define dct_length_log 	sDct_type_iv_s.dct_length_log

//-------coef2sam.c
typedef struct	
{
	Word16	c2s_new_samples[MAX_DCT_LENGTH];
    Word16	*out_ptr;
}tsRmlt_coefs_to_samples;

extern tsRmlt_coefs_to_samples sRmlt_coefs_to_samples;
#define c2s_new_samples 		sRmlt_coefs_to_samples.c2s_new_samples
#define out_ptr 				sRmlt_coefs_to_samples.out_ptr

//--------decoder.c
typedef struct
{
    Word16  decabsolute_region_power_index[MAX_NUMBER_OF_REGIONS];
    Word16  decdecoder_power_categories[MAX_NUMBER_OF_REGIONS];
    Word16  decdecoder_category_balances[MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES-1];
    UWord16 deccategorization_control;
    Word16  decdecoder_region_standard_deviation[MAX_NUMBER_OF_REGIONS];
    Word16  decnum_categorization_control_bits;
    Word16  decnum_categorization_control_possibilities;
    Word16  decnumber_of_coefs;
    Word16  decnumber_of_valid_coefs;
}tsDecoder;

extern tsDecoder sDecoder;

#define decabsolute_region_power_index 		sDecoder.decabsolute_region_power_index
#define decdecoder_power_categories 		sDecoder.decdecoder_power_categories
#define decdecoder_category_balances 		sDecoder.decdecoder_category_balances
#define deccategorization_control 			sDecoder.deccategorization_control
#define decdecoder_region_standard_deviation 	sDecoder.decdecoder_region_standard_deviation
#define decnum_categorization_control_bits 	sDecoder.decnum_categorization_control_bits
#define decnum_categorization_control_possibilities 			sDecoder.decnum_categorization_control_possibilities
#define decnumber_of_coefs 					sDecoder.decnumber_of_coefs
#define decnumber_of_valid_coefs 			sDecoder.decnumber_of_valid_coefs


typedef struct
{
    Word16 differential_region_power_index[MAX_NUMBER_OF_REGIONS];
    Word16 decenvmax_index;
    Word32 decenvtemp;					                 
	Word32 decenvtemp1;						                
    Word32 decenvacca;
}tsDecode_envelope;
extern tsDecode_envelope sDecode_envelope;

#define differential_region_power_index 	sDecode_envelope.differential_region_power_index
#define decenvmax_index 	sDecode_envelope.decenvmax_index
#define decenvtemp 			sDecode_envelope.decenvtemp
#define decenvtemp1 		sDecode_envelope.decenvtemp1
#define decenvacca 			sDecode_envelope.decenvacca


typedef struct
{
    Word16 standard_deviation;
    Word16 *decoder_mlt_ptr;
    Word16 noifillpos;
    Word16 noifillneg;
    Word16 decveccategory;
    Word16 decveck[5];//[MAX_VECTOR_DIMENSION];
    Word16 decvecindex;
    Word16 signs_index;
    Word16 decvecbit;
    Word16 num_sign_bits;
    Word16 ran_out_of_bits_flag;
    Word16 *decoder_table_ptr;
    Word16 decvecrandom_word;
    Word32 decvecacca;
}tsDecvec;
extern 	tsDecvec sDecvec;

#define standard_deviation 	sDecvec.standard_deviation
#define decoder_mlt_ptr 	sDecvec.decoder_mlt_ptr
#define noifillpos 			sDecvec.noifillpos
#define noifillneg 			sDecvec.noifillneg
#define decveccategory 		sDecvec.decveccategory
#define decveck 			sDecvec.decveck
#define decvecindex 		sDecvec.decvecindex
#define signs_index 		sDecvec.signs_index
#define decvecbit 			sDecvec.decvecbit
#define num_sign_bits 		sDecvec.num_sign_bits
#define ran_out_of_bits_flag 	sDecvec.ran_out_of_bits_flag
#define decoder_table_ptr 	sDecvec.decoder_table_ptr
#define decvecrandom_word 	sDecvec.decvecrandom_word
#define decvecacca 			sDecvec.decvecacca

/***************************************************************************/
/* Function definitions                                                    */
/***************************************************************************/
extern Word16  compute_region_powers(Word16  *mlt_coefs,
                             Word16  mag_shift,
                             Word16  *drp_num_bits,
                             UWord16 *drp_code_bits,
                             Word16  *absolute_region_power_index,
                             Word16  number_of_regions); 

void    vector_quantize_mlts(Word16 number_of_available_bits,
                          Word16 number_of_regions,
                          Word16 num_categorization_control_possibilities,
                          Word16 *mlt_coefs,
                          Word16 *absolute_region_power_index,
                          Word16 *power_categories,
                          Word16 *category_balances,
                          Word16 *p_categorization_control,
                          Word16 *region_mlt_bit_counts,
                          UWord32 *region_mlt_bits);  

Word16  vector_huffman(Word16 category,
                      Word16 power_index,
                      Word16 *raw_mlt_ptr,
                      UWord32 *word_ptr);


void    adjust_abs_region_power_index(Word16 *absolute_region_power_index,Word16 *mlt_coefs,Word16 number_of_regions); 

void    bits_to_words(UWord32 *region_mlt_bits,Word16 *region_mlt_bit_counts,
                      Word16 *drp_num_bits,UWord16 *drp_code_bits,Word16 *out_words,
                      Word16 categorization_control, Word16  number_of_regions,
                      Word16  num_categorization_control_bits, Word16 number_of_bits_per_frame);

void    encoder(Word16  number_of_available_bits,
                Word16  number_of_regions,
                Word16  *mlt_coefs,
                Word16  mag_shift,
                Word16  *out_words);

void decoder(Bit_Obj *bitobj,
    	     Rand_Obj *randobj,
             Word16 number_of_regions,
             Word16 *decoder_mlt_coefs,
	         Word16 *p_mag_shift,
	         Word16 *p_old_mag_shift,
#ifndef NO_FRAME_ERROR_CHECK
	         Word16 *old_decoder_mlt_coefs,
#endif
	         Word16 *frame_error_flag);

Word16  samples_to_rmlt_coefs(Word16 *new_samples,Word16 *history,Word16 *coefs,Word16 dct_length);
void rmlt_coefs_to_samples(Word16 *coefs,     
                           Word16 *old_samples,           
                           Word16 *out_samples,           
                           Word16 dct_length,           
                           Word16 mag_shift);

Word16  index_to_array(Word16 index,Word16 *array,Word16 category);
void    categorize(Word16 number_of_available_bits,
                   Word16 number_of_regions,
				   Word16 num_categorization_control_possibilities,
		           Word16 *rms_index,
		           Word16 *power_categories,
		           Word16 *category_balances);

Word16 calc_offset(Word16 *rms_index,Word16 number_of_regions,Word16 available_bits);  
void   compute_raw_pow_categories(Word16 *power_categories,Word16 *rms_index,Word16 number_of_regions,Word16 offset); 
void   comp_powercat_and_catbalance(Word16 *power_categories,
                                    Word16 *category_balances,
                                    Word16 *rms_index,
                                    Word16 number_of_available_bits,
                                    Word16 number_of_regions,
                                    Word16 num_categorization_control_possibilities,
                                    Word16 offset);
 
void dct_type_iv_a (Word16 *output,Word16 dct_length);		 //kpit change
void dct_type_iv_s(Word16 *input,Word16 *output,Word16 dct_length);
void decode_envelope(Bit_Obj *bitobj,
                     Word16  number_of_regions,
                     Word16  *decoder_region_standard_deviation,
		             Word16  *absolute_region_power_index,
		             Word16  *p_mag_shift);

void decode_vector_quantized_mlt_indices(Bit_Obj  *bitobj,
                                         Rand_Obj *randobj,
                                         Word16   number_of_regions,
                                         Word16   *decoder_region_standard_deviation,
					                     Word16   *dedecoder_power_categories,
					                     Word16   *dedecoder_mlt_coefs);

void rate_adjust_categories(Word16 categorization_control,
			                Word16 *decoder_power_categories,
			                Word16 *decoder_category_balances);

void get_next_bit(Bit_Obj *bitobj);
Word16 get_rand(Rand_Obj *randobj);

void test_4_frame_errors(Bit_Obj *bitobj,
                         Word16 number_of_regions,
                         Word16 num_categorization_control_possibilities,
                         Word16 *frame_error_flag,
                         Word16 categorization_control,
                         Word16 *absolute_region_power_index);

void error_handling(Word16 number_of_coefs,
                    Word16 number_of_valid_coefs,
                    Word16 *frame_error_flag,
                    Word16 *decoder_mlt_coefs,
                    Word16 *old_decoder_mlt_coefs,
                    Word16 *p_mag_shift,
                    Word16 *p_old_mag_shift);

#endif //__DEFS_H__
