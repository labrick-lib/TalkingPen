/***************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   ?2004 Polycom, Inc.
**
**   All rights reserved.
**
***************************************************************************/

/***************************************************************************
  Filename:    decoder.c    

  Purpose:     Contains files used to implement the G.722.1 Annex C decoder
		
  Design Notes:

***************************************************************************/

/***************************************************************************
 Include files                                                           
***************************************************************************/
#include "../common/defs.h"
#include "../common/tables.h"
#include "../common/huff_def.h"
#include "../common/count.h"

tsDecoder sDecoder __attribute__ ((section("playvars"), zero_init));
tsDecode_envelope sDecode_envelope __attribute__ ((section("playvars"), zero_init));
tsDecvec sDecvec __attribute__ ((section("playvars"), zero_init));
/***************************************************************************
 Function:    decoder

 Syntax:      void decoder(Bit_Obj *bitobj,                   
                           Rand_Obj *randobj,             
                           Word16 number_of_regions,
                           Word16 *decoder_mlt_coefs,     
                           Word16 *p_mag_shift,           
                           Word16 *p_old_mag_shift,       
                           Word16 *old_decoder_mlt_coefs,    
                           Word16 frame_error_flag)      
              
              inputs:    Bit_Obj *bitobj
                         Rand_Obj *randobj
                         Word16 number_of_regions
                         Word16 *p_old_mag_shift
                         Word16 *old_decoder_mlt_coefs
                         Word16 frame_error_flag
              
              outputs:   Word16 *decoder_mlt_coefs,    
                         Word16 *p_mag_shift,          
                  
                  

 Description: Decodes the out_words into mlt coefs using G.722.1 Annex C

 Design Notes:
 
 WMOPS:     7kHz |   24kbit    |    32kbit
          -------|-------------|----------------
            AVG  |    0.84     |    0.94
          -------|-------------|----------------  
            MAX  |    0.90     |    1.00
          -------|-------------|---------------- 
				
           14kHz |   24kbit    |    32kbit      |     48kbit
          -------|-------------|----------------|----------------
            AVG  |    1.31     |    1.56        |     1.88   
          -------|-------------|----------------|----------------
            MAX  |    1.59     |    1.80        |     1.98   
          -------|-------------|----------------|----------------
				
***************************************************************************/
void decoder(Bit_Obj *bitobj,
             Rand_Obj *randobj,
             Word16 number_of_regions,
	         Word16 *decoder_mlt_coefs,
	         Word16 *p_mag_shift,
             Word16 *p_old_mag_shift,
#ifndef NO_FRAME_ERROR_CHECK
             Word16 *old_decoder_mlt_coefs,
#endif
             Word16 *frame_error_flag)
{


////    Word16  decabsolute_region_power_index[MAX_NUMBER_OF_REGIONS];
////    Word16  decdecoder_power_categories[MAX_NUMBER_OF_REGIONS];
////    Word16  decdecoder_category_balances[MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES-1];
////    UWord16 deccategorization_control;
////    Word16  decdecoder_region_standard_deviation[MAX_NUMBER_OF_REGIONS];
    register Word16  i;													                       //kpit change 28/9

////    Word16  decdecnum_categorization_control_bits;
////    Word16  decnum_categorization_control_possibilities;
////    Word16  decnumber_of_coefs;
////    Word16  decnumber_of_valid_coefs;
    
#ifdef SIREN14  
    if (number_of_regions==MAX_NUMBER_OF_REGIONS)
    {
		decdecnum_categorization_control_bits = MAX_NUM_CATEGORIZATION_CONTROL_BITS;
        decnum_categorization_control_possibilities = MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES;
        decnumber_of_coefs = MAX_DCT_LENGTH;
        decnumber_of_valid_coefs = MAX_NUMBER_OF_VALID_COEFS;
    }
    else
    {
#endif		
		decnum_categorization_control_bits = NUM_CATEGORIZATION_CONTROL_BITS;
        decnum_categorization_control_possibilities = NUM_CATEGORIZATION_CONTROL_POSSIBILITIES;
        decnumber_of_coefs = DCT_LENGTH;
        decnumber_of_valid_coefs = NUMBER_OF_VALID_COEFS;
#ifdef SIREN14  
    }
#endif		

	if (*frame_error_flag == 0) 
    {

        /* convert the bits to absolute region power index and decoder_region_standard_deviation */
        
        decode_envelope(bitobj,
                        number_of_regions,
                        decdecoder_region_standard_deviation,
		                decabsolute_region_power_index,
		                p_mag_shift);

        /* fill the categorization_control with NUM_CATEGORIZATION_CONTROL_BITS */
        deccategorization_control = 0;
        for (i=0; i<decnum_categorization_control_bits; i++) 
        {
        	get_next_bit(bitobj);
        	//categorization_control = categorization_control << 1;						     //kpit change 28/9
        	//categorization_control = categorization_control + bitobj->next_bit;	         //kpit change 28/9
			deccategorization_control = (deccategorization_control<<1) + bitobj->next_bit;         //kpit change 28/9
        }
        
        bitobj->number_of_bits_left = (bitobj->number_of_bits_left - decnum_categorization_control_bits);

        /* obtain decoder power categories and category balances */
        /* based on the absolute region power index              */
        categorize(bitobj->number_of_bits_left,
	               number_of_regions,
	               decnum_categorization_control_possibilities,
	               decabsolute_region_power_index,
	               decdecoder_power_categories,
	               decdecoder_category_balances);

        /* perform adjustmaents to the power categories and category balances based on the cat control */
        rate_adjust_categories(deccategorization_control,
			                   decdecoder_power_categories,
			                   decdecoder_category_balances);

        /* decode the quantized bits into mlt coefs */
        decode_vector_quantized_mlt_indices(bitobj,
                                            randobj,
                                            number_of_regions,
                                            decdecoder_region_standard_deviation,
					                        decdecoder_power_categories,
					                        decoder_mlt_coefs);

#ifndef NO_FRAME_ERROR_CHECK
        /* test for frame errors */
        test_4_frame_errors(bitobj,
                            number_of_regions,
                            decnum_categorization_control_possibilities,
                            frame_error_flag,
                            deccategorization_control,
                            decabsolute_region_power_index);
#endif
    }
#ifndef NO_FRAME_ERROR_CHECK
    /* perform error handling operations */
    error_handling(decnumber_of_coefs,
                   decnumber_of_valid_coefs,
                   frame_error_flag,
                   decoder_mlt_coefs,
                   old_decoder_mlt_coefs,
                   p_mag_shift,
                   p_old_mag_shift);
#endif 
}

/***************************************************************************
 Function:    decode_envelope

 Syntax:      void decode_envelope(Bit_Obj *bitobj,                              
                                   Word16  number_of_regions,
                                   Word16  *decoder_region_standard_deviation,   
                                   Word16  *absolute_region_power_index,         
                                   Word16  *p_mag_shift)                         
              
              inputs:   Bit_Obj *bitobj
                        Word16  number_of_regions
                        
                        
              outputs:  Word16  *decoder_region_standard_deviation
                        Word16  *absolute_region_power_index
                        Word16  *p_mag_shift
              
 
 Description: Recover differential_region_power_index from code bits

 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |     0.04     |    0.04
          -------|--------------|----------------  
            MAX  |     0.05     |    0.05
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |     0.08     |    0.08        |     0.08   
          -------|--------------|----------------|----------------
            MAX  |     0.10     |    0.10        |     0.10   
          -------|--------------|----------------|----------------
				
***************************************************************************/
void decode_envelope(Bit_Obj *bitobj,
                     Word16  number_of_regions,
                     Word16  *decoder_region_standard_deviation,
		             Word16  *absolute_region_power_index,
		             Word16  *p_mag_shift)
     
{
   // register Word16 region;	                                          	//kpit change 27/9
    register Word32 region;		                                            //kpit change 27/9	   30/9
    register Word16 i;			                                            //kpit change 27/9
    register Word16 index;		                                            //kpit change 27/9
////    Word16 differential_region_power_index[MAX_NUMBER_OF_REGIONS];
////    Word16 decenvmax_index;
////    
////   // Word16 temp;		
////    Word32 decenvtemp;					                                         //kpit change 30/9
////    //Word16 temp1;
////	Word32 decenvtemp1;						                                    //kpit change 30/9
////    //Word16 temp2;				                                            //kpit change 29/9
////    Word32 decenvacca;

    index = 0;
    move16();

    /* get 5 bits from the current code word */
    for (i=0; i<5; i++) 
    {
        get_next_bit(bitobj);
        //index = index << 1;		                                       //kpit change 28/9
        //index = (index + bitobj->next_bit);	                           //kpit change 28/9
		index = ((index << 1) + bitobj->next_bit);                         //kpit change 28/9
    }
    bitobj->number_of_bits_left = bitobj->number_of_bits_left - 5;

    /* ESF_ADJUSTMENT_TO_RMS_INDEX compensates for the current (9/30/96)
        IMLT being scaled to high by the ninth power of sqrt(2). */
    differential_region_power_index[0] = (index - ESF_ADJUSTMENT_TO_RMS_INDEX);
    move16();

    /* obtain differential_region_power_index */
    for (region=1; region<number_of_regions; region++) 
    {
        index = 0;
        move16();
        do 
        {
            get_next_bit(bitobj);
            test();
            if (bitobj->next_bit == 0)
            {
	            index = differential_region_power_decoder_tree[region][index][0];
                move16();
            }
            else
            {
	            index = differential_region_power_decoder_tree[region][index][1];
                move16();
            }
            bitobj->number_of_bits_left = (bitobj->number_of_bits_left - 1);
            test();
        } while (index > 0);
        
        differential_region_power_index[region] = -(index);
        move16();
    }

    /* Reconstruct absolute_region_power_index[] from differential_region_power_index[]. */
    absolute_region_power_index[0] = differential_region_power_index[0];
    move16();
    for (region=1; region<number_of_regions; region++) 
    {
        decenvacca = (absolute_region_power_index[region-1] + differential_region_power_index[region]);
        //acca = (acca + DRP_DIFF_MIN);										                    //kpit change 28/9
        //absolute_region_power_index[region] = extract_l(acca); 			                    //kpit change 28/9
		absolute_region_power_index[region] = extract_l(decenvacca + DRP_DIFF_MIN);                   //kpit change 28/9
		    
    }

    /* Reconstruct decoder_region_standard_deviation[] from absolute_region_power_index[]. */
    /* DEBUG!!!! - This integer method jointly computes the mag_shift
       and the standard deviations already mag_shift compensated. It
       relies on REGION_POWER_STEPSIZE_DB being exactly 3.010299957 db
       or a square root of 2 chnage in standard deviation. If
       REGION_POWER_STEPSIZE_DB changes, this software must be
       reworked. */

    decenvtemp = 0;		
    move16();
    decenvmax_index = 0;
    move16();
    for (region=0; region<number_of_regions; region++) 
    {
        decenvacca = (absolute_region_power_index[region] + REGION_POWER_TABLE_NUM_NEGATIVES);
        //i = extract_l(acca);		                                                          //kpit change 29/9				
        
       // temp1 = (i - max_index);			
	    decenvtemp1 = (extract_l(decenvacca) - decenvmax_index);	                                             //kpit change 29/9		
	   
		
        test();
        if (decenvtemp1 > 0) 
        {
             //max_index = i;
			 decenvmax_index = extract_l(decenvacca);	                                                //kpit change 29/9
			  
            move16();
        }
        //temp = (temp + int_region_standard_deviation_table[i]);
		decenvtemp = (decenvtemp + int_region_standard_deviation_table[(extract_l(decenvacca))]);	            //kpit change 29/9
    }
    i = 9;
    move16();

    //temp1 = (temp-8);						                                               //kpit change 28/9
    //temp2 = (max_index-28);				                                               //kpit change 28/9
    test();
    test();
    logic16();
    test();
    logic16();
    while ((i >= 0) && (((decenvtemp-8) >= 0) || ((decenvmax_index-28) > 0))) 
	
    {
        //i = (i-1);
		i--;				                                                                //kpit change 27/9
        decenvtemp = (decenvtemp>>1);
        decenvmax_index = (decenvmax_index-2);
        //temp1 = (temp-8);			                                                       //kpit change 28/9							
        //temp2 = (max_index-28);	                                                       //kpit change 28/9
        test();
        test();
        logic16();
        test();
        logic16();
    }
    
    *p_mag_shift = i;
    move16();
    
    /* pointer arithmetic */
   // temp = (Word16 )(REGION_POWER_TABLE_NUM_NEGATIVES + (*p_mag_shift * 2));
	decenvtemp = (Word16 )(REGION_POWER_TABLE_NUM_NEGATIVES + (*p_mag_shift<<1));	               //kpit change 27/9
    
    for (region=0; region<number_of_regions; region++) 
    {
        decenvacca = (absolute_region_power_index[region]+decenvtemp);
        //i = extract_l(acca);								                              //kpit change 29/9
        //decoder_region_standard_deviation[region] = int_region_standard_deviation_table[i];
		decoder_region_standard_deviation[region] = int_region_standard_deviation_table[(extract_l(decenvacca))];	  //kpit change 29/9
        move16();
    }

}

/***************************************************************************
 Function:     rate_adjust_categories

 Syntax:       void rate_adjust_categories(Word16 categorization_control,            
                                           Word16 *decoder_power_categories,         
                                           Word16 *decoder_category_balances)        
               
               inputs:    Word16 categorization_control,   
                          Word16 *decoder_power_categories,
                          Word16 *decoder_category_balances
                          
               outputs:   Word16 categorization_control,   
                          Word16 *decoder_power_categories,
 
 Description:  Adjust the power categories based on the categorization control

 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |    0.00      |    0.00
          -------|--------------|----------------  
            MAX  |    0.00      |    0.00
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.00      |    0.00        |     0.00   
          -------|--------------|----------------|----------------
            MAX  |    0.01      |    0.01        |     0.01   
          -------|--------------|----------------|----------------
				
***************************************************************************/
void rate_adjust_categories(Word16 categorization_control,
			                Word16 *decoder_power_categories,
			                Word16 *decoder_category_balances)
{
    Word16 i;
    //register Word16 region;	                                                 //kpit change 27/9
	register Word32 region;	                                                    //kpit change 27/9				30/9
    
    i = 0;
    move16();

    test();
    while (categorization_control > 0) 
    {
        region = decoder_category_balances[i++];
        move16();
        decoder_power_categories[region] = (decoder_power_categories[region]+1);
        move16();
        categorization_control = (categorization_control-1);
    }

}

/***************************************************************************
 Function:    decode_vector_quantized_mlt_indices

 Syntax:      void decode_vector_quantized_mlt_indices(Bit_Obj  *bitobj,                                      
                                                       Rand_Obj *randobj,                           
                                                       Word16   number_of_regions,
                                                       Word16   *decoder_region_standard_deviation, 
                                                       Word16   *decoder_power_categories,          
                                                       Word16   *decoder_mlt_coefs)                 
              inputs:    Bit_Obj  *bitobj                           
                         Rand_Obj *randobj
                         Word16   number_of_regions
                         Word16   *decoder_region_standard_deviation
                         Word16   *decoder_power_categories
            
            
              outputs:   Word16   *decoder_mlt_coefs
             

 Description: Decode MLT coefficients

 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |    0.60      |    0.72
          -------|--------------|----------------  
            MAX  |    0.67      |    0.76
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.77      |    0.98        |     1.28   
          -------|--------------|----------------|----------------
            MAX  |    1.05      |    1.18        |     1.36   
          -------|--------------|----------------|----------------
				
***************************************************************************/
void decode_vector_quantized_mlt_indices(Bit_Obj  *bitobj,
                                         Rand_Obj *randobj,
                                         Word16   number_of_regions,
                                         Word16   *decoder_region_standard_deviation,
					                     Word16   *decoder_power_categories,
					                     Word16   *decoder_mlt_coefs)
{
////    Word16 standard_deviation;
////    Word16 *decoder_mlt_ptr;
    //Word16 decoder_mlt_value;
////    Word16 noifillpos;
////    Word16 noifillneg;
    Word16 noise_fill_factor[3] = {5793,8192,23170};
    register Word16 region;					                                       //kpit change 27/9
////    Word16 decveccategory;
    register Word16 j,n;					                                      //kpit change 27/9
////    Word16 decveck[MAX_VECTOR_DIMENSION];
    //Word16 vec_dim;					                                         //kpit change 29/9
    //Word16 num_vecs;						                                     //kpit change 29/9
////    Word16 decvecindex;
////    Word16 signs_index = 0;
////    Word16 decvecbit = 0;
////    Word16 num_sign_bits;
////    Word16 ran_out_of_bits_flag;
////    Word16 *decoder_table_ptr;
////    Word16 decvecrandom_word;
    
    //Word16 temp1;
   // Word16 temp;							                               	//kpit change 29/9
////    Word32 decvecacca;

	signs_index = 0;
	decvecbit = 0;
    ran_out_of_bits_flag = 0;
    move16();

    for (region=0; region<number_of_regions; region++) 
    {
        decveccategory = (Word16)decoder_power_categories[region];
        move16();
        decvecacca = region * REGION_SIZE;
        decvecindex = extract_l(decvecacca);
        decoder_mlt_ptr = &decoder_mlt_coefs[decvecindex];
        move16();
        standard_deviation = decoder_region_standard_deviation[region];
        move16();
        
        //temp = category - 7;	                                                         	kpit change 28/9
        test();
       // if (temp < 0)
	   if ((decveccategory - 7) < 0)	  	                                                       //kpit change 28/9
        {
            /* Get the proper table of decoder tables, vec_dim, and num_vecs for the cat */
            decoder_table_ptr = (Word16 *) table_of_decoder_tables[decveccategory];
            move16();
            //vec_dim = vector_dimension[category];	                                        //kpit change 29/9
            move16();
            //num_vecs = number_of_vectors[category];                                       //kpit change 29/9
            move16();
            
            //for (n=0; n<num_vecs; n++) 
			for (n=0; n<number_of_vectors[decveccategory]; n++) 		                               //kpit change 29/9
            {
                decvecindex = 0;
                move16();
                
                /* get index */
                do 
                {
                    test();
                    if (bitobj->number_of_bits_left <= 0) 
                    {
                        ran_out_of_bits_flag = 1;
                        move16();
    	                break;
    	            }
    
    	            get_next_bit(bitobj);
	                
                    test();
                    if (bitobj->next_bit == 0)
	                {
                        //temp = index << 1;
                        //index = (Word16)*(decoder_table_ptr + temp);
						decvecindex = (Word16)*(decoder_table_ptr + (decvecindex << 1));                //kpit change 29/9
                        move16();
                    }
	                else
	                {
                       // temp = (index << 1);
                        //index = (Word16)*(decoder_table_ptr + temp + 1);
						decvecindex = (Word16)*(decoder_table_ptr +(decvecindex << 1)+ 1);	             //kpit change  29/9
                        move16();
                    }
	                bitobj->number_of_bits_left = (bitobj->number_of_bits_left - 1);
                    test();
	            
                } while (decvecindex > 0);
	  
                test();
                if (ran_out_of_bits_flag != 0)
	                break;
	  
                decvecindex = -(decvecindex);
	            
                /* convert index into array used to access the centroid table */
                /* get the number of sign bits in the index */
                num_sign_bits = index_to_array(decvecindex,decveck,decveccategory);

	            //temp = (bitobj->number_of_bits_left - num_sign_bits);		              //kpit change 29/9
                test();
                //if (temp >= 0) 											             //kpit change 29/9
				if ((bitobj->number_of_bits_left - num_sign_bits)>= 0)	                  //kpit change 29/9
                {
	                test();
                    if (num_sign_bits != 0) 
                    {
	                    signs_index = 0;
	                    move16();
                        for (j=0; j<num_sign_bits; j++) 
                        {
		                    get_next_bit(bitobj);
       		                //signs_index = (signs_index << 1);					         //kpit change 28/9
		                    //signs_index = (signs_index + bitobj->next_bit);	        //kpit change 28/9
							signs_index = ((signs_index<<1) + bitobj->next_bit);         //kpit change 28/9
		                    bitobj->number_of_bits_left = (bitobj->number_of_bits_left - 1);
	                    }
	                    //temp = (num_sign_bits - 1);			                          //kpit change 28/9
                        //bit = (1 << (temp));					                         //kpit change 28/9
						decvecbit = (1 << (num_sign_bits - 1));		                         //kpit change 28/9
	                }
	                
                   // for (j=0; j<vec_dim; j++)			                                 //kpit change 29/9
			       for (j=0; j<vector_dimension[decveccategory];j++) 
                    {
	                    //acca = (standard_deviation * mlt_quant_centroid[category][k[j]]);
                        //acca = (acca >> 12);
						decvecacca = ((standard_deviation * mlt_quant_centroid[decveccategory][decveck[j]])>>12);	  //kpit change 28/9
                        *decoder_mlt_ptr = extract_l(decvecacca);
	                    
                        test();
                        if (*decoder_mlt_ptr != 0) 
                        {
		                    test();
                            if ((signs_index & decvecbit) == 0)
		                        *decoder_mlt_ptr = -(*decoder_mlt_ptr);
		                    decvecbit = (decvecbit >> 1);
	                    }
                        decoder_mlt_ptr++; // = decoder_mlt_value;
                        move16();
	                }
	            }
	            else 
                {
	                ran_out_of_bits_flag = 1;
                    move16();
	                break;
	            }
	        }
            /* If ran out of bits during decoding do noise fill for remaining regions. */
            /* DEBUG!! - For now also redo all of last region with all noise fill. */
        	test();
            if (ran_out_of_bits_flag != 0) 
            {
        	   // temp = (region+1);				                                         //kpit change 29/9
               // for (j=temp; j<number_of_regions; j++)	                                //kpit change 29/9
			    for (j=(region+1); j<number_of_regions; j++)
                {
                    decoder_power_categories[j] = 7;
                    move16();
                }
        	    decveccategory = 7;
                move16();
        	    decoder_mlt_ptr = &decoder_mlt_coefs[region*REGION_SIZE];
                move16();
        	}
        }

        //temp = (category-5);
        //temp1 = (category-6);
        test();
        test();
        logic16();
       // if ((temp == 0) || (temp1 == 0))
	    if (((decveccategory-5) == 0) || ((decveccategory-6) == 0))	                                   //kpit change 29/9
        {
 
	        decoder_mlt_ptr = &decoder_mlt_coefs[region*REGION_SIZE];
	        move16();
            noifillpos = (standard_deviation * noise_fill_factor[decveccategory - 5])>>15;
            noifillneg = -(noifillpos);

	        decvecrandom_word = get_rand(randobj);

	        for (j=0; j<10; j++) 
            {
	            test();
                if (*decoder_mlt_ptr == 0) 
                {
	                logic16();
                    test();
                    if ((decvecrandom_word & 1) == 0) 
                    {
                        *decoder_mlt_ptr = noifillneg;			                           //kpit change 30/9
                        move16();
                    }
	                else
                    {
                        *decoder_mlt_ptr = noifillpos;		                                //kpit change 30/9
                        move16();
                    }
	                //*decoder_mlt_ptr = temp1;
                    move16();
	                decvecrandom_word = (decvecrandom_word>>1);
	            }
	            /* pointer arithmetic */
                decoder_mlt_ptr++;
	        }
	        decvecrandom_word = get_rand(randobj);
	        for (j=0; j<10; j++) 
            {
	            test();
                if (*decoder_mlt_ptr == 0) 
                {
	                logic16();
                    test();
                    if ((decvecrandom_word & 1) == 0) 
                    {
                        *decoder_mlt_ptr = noifillneg;		                              //kpit change 30/9
                        move16();
                    }
	                else
                    {
                        *decoder_mlt_ptr = noifillpos;		                              //kpit change 30/9
                        move16();
                    }
	                //*decoder_mlt_ptr = temp1;
                    move16();
	                decvecrandom_word  = (decvecrandom_word>>1);
	            }
	            /* pointer arithmetic */
                decoder_mlt_ptr++;
	        }
        }

        /* if (category == 7) */
        //temp1 = (category-7);
        test();
        if ((decveccategory-7) == 0)
        {
	        //decvecindex = (category-5);		                                                   //kpit change 29/9
            //noifillpos = (standard_deviation*noise_fill_factor[index])>>15;             //kpit change 29/9
			noifillpos = (standard_deviation*noise_fill_factor[(decveccategory-5)])>>15;	      //kpit change 29/9
	        noifillneg = -(noifillpos);

            decvecrandom_word = get_rand(randobj);
            for (j=0; j<10; j++) 
            {
                logic16();
                test();
                if ((decvecrandom_word & 1) == 0) 
                {
                    *decoder_mlt_ptr++ = noifillneg;		                              //kpit change 30/9
                    move16();
                }
                else
                {
                    *decoder_mlt_ptr++ = noifillpos;			                          //kpit change 30/9
                    move16();
                }
                //*decoder_mlt_ptr++ = temp1;
                move16();
                decvecrandom_word = (decvecrandom_word>>1);
            }
            decvecrandom_word = get_rand(randobj);
            for (j=0; j<10; j++) 
            {
                logic16();
                test();
                if ((decvecrandom_word & 1) == 0) 
                {
                    *decoder_mlt_ptr++ = noifillneg;
                    move16();
                }
                else
                {
                    *decoder_mlt_ptr++ = noifillpos;
                    move16();
                }
                
                //*decoder_mlt_ptr++ = temp1;
                move16();
                decvecrandom_word = (decvecrandom_word>>1);
            }
        }
    }

    test();
    if (ran_out_of_bits_flag)
        bitobj->number_of_bits_left = (bitobj->number_of_bits_left-1);
}
/****************************************************************************************
 Function:    index_to_array 

 Syntax:      number_of_non_zero = index_to_array(Word16 index, 
                                                  Word16 array[MAX_VECTOR_DIMENSION],
                                                  Word16 category)

                inputs:  Word16 index
                         Word16 category                     
                       
                outputs: Word16 array[MAX_VECTOR_DIMENSION] - used in decoder to access
                                                             mlt_quant_centroid table
                        
                         Word16 number_of_non_zero          - number of non zero elements
                                                             in the array
 
 Description: Computes an array of sign bits with the length of the category vector
              Returns the number of sign bits and the array

 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |     0.00     |     0.00
          -------|--------------|----------------  
            MAX  |     0.00     |     0.00
          -------|--------------|---------------- 

           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |     0.00     |     0.00       |      0.00   
          -------|--------------|----------------|----------------
            MAX  |     0.00     |     0.00       |      0.00   
          -------|--------------|----------------|----------------

****************************************************************************************/
Word16 index_to_array(Word16 index,Word16 *array,Word16 category)
{
    register Word16 j;
	Word16 q,p;							                                                //kpit change 27/9
    Word16 number_of_non_zero;
    Word16 max_bin_plus_one;
    Word16 inverse_of_max_bin_plus_one;
    //Word16 temp;									 

    number_of_non_zero = 0;
    move16();

    p = index;
    move16();

    max_bin_plus_one = (max_bin[category]+1);
    inverse_of_max_bin_plus_one = max_bin_plus_one_inverse[category];
    move16();

    //temp = (vector_dimension[category]-1);
    for (j=(vector_dimension[category]-1); j>=0; j--) 
    {
        q = (p*inverse_of_max_bin_plus_one)>>15;
		//temp = extract_l((q*max_bin_plus_one));
        array[j] = (p-(extract_l((q*max_bin_plus_one))));
        move16();

        p = q;
        move16();

        //temp = array[j];
        move16();
        test();
       // if (temp != 0) 
	    if ((array[j])!= 0) 		                                                     //kpit change 29/9
            number_of_non_zero++; // = (number_of_non_zero+1);
    }
    return(number_of_non_zero);
}
/***************************************************************************
 Function:     test_4_frame_errors

 Syntax:       void test_4_frame_errors(Bit_Obj *bitobj,                        
                                        Word16 number_of_regions,
                                        Word16 num_categorization_control_possibilities,
                                        Word16 *frame_error_flag,                
                                        Word16 categorization_control,          
                                        Word16 *absolute_region_power_index)    
        
               inputs:   bit_obj
                         number_of_regions
                         num_categorization_control_possibilities
                         frame_error_flag
                         categorization_control
                         absolute_region_power_index
               
               
               outputs:  frame_error_flag
               
               
               
        
 Description:  Tests for error conditions and sets the frame_error_flag accordingly 

 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |    0.01      |     0.01
          -------|--------------|----------------  
            MAX  |    0.04      |     0.08
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.01      |     0.01       |      0.01   
          -------|--------------|----------------|----------------
            MAX  |    0.02      |     0.06       |      0.08   
          -------|--------------|----------------|----------------
				
***************************************************************************/
void test_4_frame_errors(Bit_Obj *bitobj,
                         Word16 number_of_regions,
                         Word16 num_categorization_control_possibilities,
                         Word16 *frame_error_flag,
                         Word16 categorization_control,
                         Word16 *absolute_region_power_index)
{
    //Word16 region;
    register Word32 region;		                                                     //kpit change 30/9
    register Word16 i;			                                                    //kpit change 27/9
    //Word16 temp;
	Word32 temp;				                                                     //kpit change 30/9
    Word32 acca;
    Word32 accb;			                                                     	//kpit change 29/9
    
    /* Test for bit stream errors. */
    if (bitobj->number_of_bits_left > 0) 
    {
        for (i=0; i<bitobj->number_of_bits_left; i++) 
        {
            get_next_bit(bitobj);
            if (bitobj->next_bit == 0) 
            {
                *frame_error_flag = 1;
					
            }
        }	
    }
    else 
    {
        temp = (categorization_control-(num_categorization_control_possibilities-1));
        if (temp < 0) 
        {
            if (bitobj->number_of_bits_left < 0)
            {
                *frame_error_flag |= 2;
            }
        }
    }

    /* checks to ensure that abs_region_power_index is within range */
    /* the error flag is set if it is out of range */
    for (region=0; region<number_of_regions; region++) 
    {
        /*  the next two lines of comments were modified in release 1.2
	   *  to correct the description of the range of 
	   *  absolute_region_power_index[] to be tested in the next
	   *  9 lines of code.
	   */
	  /*  if ((absolute_region_power_index[region] > 31) ||
            (absolute_region_power_index[region] < -8) */

        acca = (absolute_region_power_index[region]+ESF_ADJUSTMENT_TO_RMS_INDEX);
        accb = (acca-31);
        acca = (acca+8);

        /* the next line was modifed in release 1.2 to
	   * correct miss typed code and error checking.
	   */
       if ((accb > 0) || (acca < 0))
	   
        {
            *frame_error_flag |= 4;
            logic16();
        }
    }

}
/***************************************************************************
 Function:    error_handling

 Syntax:      void error_handling(Word16 number_of_coefs,
                                  Word16 number_of_valid_coefs,
                                  Word16 *frame_error_flag,     
                                  Word16 *decoder_mlt_coefs,    
                                  Word16 *old_decoder_mlt_coefs,
                                  Word16 *p_mag_shift,          
                                  Word16 *p_old_mag_shift)      
              
              inputs:  number_of_coefs
                       number_of_valid_coefs
                       frame_error_flag
                       old_decoder_mlt_coefs
                       p_old_mag_shift
              
              
              outputs: decoder_mlt_coefs
                       old_decoder_mlt_coefs
                       p_mag_shift
                       p_old_mag_shift
 
       

 Description: If both the current and previous frames are errored,             
              set the mlt coefficients to 0. If only the current frame         
              is errored, then repeat the previous frame's mlt coefficients.   
    
 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |    0.02      |     0.02
          -------|--------------|----------------  
            MAX  |    0.03      |     0.03
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.03      |     0.03       |     0.03   
          -------|--------------|----------------|----------------
            MAX  |    0.03      |     0.03       |     0.06   
          -------|--------------|----------------|----------------
				
***************************************************************************/
void error_handling(Word16 number_of_coefs,
                    Word16 number_of_valid_coefs,
                    Word16 *frame_error_flag,
                    Word16 *decoder_mlt_coefs,
                    Word16 *old_decoder_mlt_coefs,
                    Word16 *p_mag_shift,
                    Word16 *p_old_mag_shift)
{
    register Word16 i;			                                                    //kpit change 27/9
    if (*frame_error_flag != 0) 
    {

        for (i = 0; i < number_of_valid_coefs; i++)
        {
            decoder_mlt_coefs[i] = old_decoder_mlt_coefs[i];
			old_decoder_mlt_coefs[i] = 0;
        }

        //for (i = 0; i < number_of_valid_coefs; i++)								//kpit change
        //{
        //    old_decoder_mlt_coefs[i] = 0;
        //}
        
        *p_mag_shift = *p_old_mag_shift;

        *p_old_mag_shift = 0;
    }
    else 
    {
        /* Store in case next frame is errored. */
        for (i = 0; i < number_of_valid_coefs; i++)
        {
            old_decoder_mlt_coefs[i] = decoder_mlt_coefs[i];
        }
  
        *p_old_mag_shift = *p_mag_shift;
    }


    /* Zero out the upper 1/8 of the spectrum. */
    for (i = number_of_valid_coefs; i < number_of_coefs; i++)
    {
        decoder_mlt_coefs[i] = 0;
    }

}
/****************************************************************************************
 Function:    get_next_bit

 Syntax:      void get_next_bit(Bit_Obj *bitobj)
 
 Description: Returns the next bit in the current word inside the bit object
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |    0.00      |    0.00
          -------|--------------|----------------  
            MAX  |    0.00      |    0.00
          -------|--------------|---------------- 

           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.00      |    0.00        |     0.00   
          -------|--------------|----------------|----------------
            MAX  |    0.00      |    0.00        |     0.00   
          -------|--------------|----------------|----------------

****************************************************************************************/
void get_next_bit(Bit_Obj *bitobj)
{
    Word16 temp;

    test();
    if (bitobj->code_bit_count == 0)
    {                        
        bitobj->current_word = *bitobj->code_word_ptr++; 
        move16();
        bitobj->code_bit_count = 16;           
        move16();
    }
    bitobj->code_bit_count = (bitobj->code_bit_count-1);
    temp = (bitobj->current_word>>bitobj->code_bit_count);
    logic16();
    bitobj->next_bit = (Word16 )(temp & 1);

}
/****************************************************************************************
 Function:    get_rand

 Syntax:      Word16 get_rand(Rand_Obj *randobj)
 
 Description: Returns a random Word16 based on the seeds inside the rand object  
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |    0.00      |    0.00
          -------|--------------|----------------  
            MAX  |    0.00      |    0.00
          -------|--------------|---------------- 

           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.00      |    0.00        |     0.00   
          -------|--------------|----------------|----------------
            MAX  |    0.00      |    0.00        |     0.00   
          -------|--------------|----------------|----------------

****************************************************************************************/
Word16 get_rand(Rand_Obj *randobj)
{
   // register Word16 random_word;	                                         //kpit change 27/9
    register Word32 random_word;	                                         //kpit change 27/9   30/9
    Word32 acca;

    acca = (randobj->seed0+randobj->seed3); 
    random_word = extract_l(acca);
    
    logic16();
    test();
	if ((random_word & 32768L) != 0)
        random_word = (random_word+1);
    
    randobj->seed3 = randobj->seed2;
    move16();
    randobj->seed2 = randobj->seed1;
    move16();
    randobj->seed1 = randobj->seed0;
    move16();
    randobj->seed0 = random_word;
    move16();

    return(random_word);
}
