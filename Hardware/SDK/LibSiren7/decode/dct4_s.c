/********************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   ?2004 Polycom, Inc.
**
**   All rights reserved.
**
********************************************************************************/

/********************************************************************************
* Filename: dct_type_iv_s.c
*
* Purpose:  Discrete Cosine Transform, Type IV used for inverse MLT
*
* The basis functions are
*
*	 cos(PI*(t+0.5)*(k+0.5)/block_length)
*
* for time t and basis function number k.  Due to the symmetry of the expression
* in t and k, it is clear that the forward and inverse transforms are the same.
*
*********************************************************************************/

/***************************************************************************
 Include files                                                           
***************************************************************************/
#include "../common/defs.h"
#include "../common/count.h"
#include "dct4_s.h"

/***************************************************************************
 External variable declarations                                          
***************************************************************************/
extern const Word8    syn_bias_7khz[DCT_LENGTH];
extern const Word8    dither[DCT_LENGTH];
#ifdef SIREN14
extern const Word8    max_dither[MAX_DCT_LENGTH];
#endif
extern const Word16     dct_core_s[DCT_LENGTH_DIV_32][DCT_LENGTH_DIV_32];
extern const cos_msin_t	s_cos_msin_2[DCT_LENGTH_DIV_32];
extern const cos_msin_t	s_cos_msin_4[DCT_LENGTH_DIV_16];
extern const cos_msin_t	s_cos_msin_8[DCT_LENGTH_DIV_8];
extern const cos_msin_t	s_cos_msin_16[DCT_LENGTH_DIV_4];
extern const cos_msin_t	s_cos_msin_32[DCT_LENGTH_DIV_2];
extern const cos_msin_t	s_cos_msin_64[DCT_LENGTH];
extern const cos_msin_t* const s_cos_msin_table[];

#define round16(x) (Word16)((x + (Word32)0x00008000L)>>16)
#define round15(x) (Word16)((x + (Word32)0x00004000L)>>15)
#define round14(x) (Word16)((x + (Word32)0x00002000L)>>14)

tsDct_type_iv_s sDct_type_iv_s __attribute__ ((section("playvars"), zero_init));

/********************************************************************************
 Function:    dct_type_iv_s

 Syntax:      void dct_type_iv_s (Word16 *input,Word16 *output,Word16 dct_length)
              

 Description: Discrete Cosine Transform, Type IV used for inverse MLT

 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |     1.74     |     1.74
          -------|--------------|----------------  
            MAX  |     1.74     |     1.74
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |     3.62     |     3.62       |      3.62   
          -------|--------------|----------------|----------------
            MAX  |     3.62     |     3.62       |      3.62   
          -------|--------------|----------------|----------------

********************************************************************************/

void dct_type_iv_s (Word16 *input,Word16 *output,Word16 dct_length)
{
////    Word16   buffer_a[MAX_DCT_LENGTH];//kpit change buffer_b[MAX_DCT_LENGTH];KPIT-, buffer_c[MAX_DCT_LENGTH] removed;
////    Word16   *in_ptr, *in_ptr_low, *in_ptr_high, *next_in_base ,*temp_ptr;
////    Word16   *out_ptr_low, *out_ptr_high, *next_out_base;
////    Word16   *out_buffer, *in_buffer, *buffer_swap;
   // Word16   in_val_low, in_val_high;
////    Word16   out_val_low, out_val_high;
   // Word16   in_low_even, in_low_odd;
   // Word16   in_high_even, in_high_odd;
    //Word16   out_low_even, out_low_odd;
    //Word16   out_high_even, out_high_odd;
////    Word16   *pair_ptr;
  //  Word16   cos_even, cos_odd, msin_even, msin_odd;
////    Word16   set_span,/* set_count*//* set_count_log,*/ pairs_left, sets_left;	             //kpit change 30/9
	register Word16   set_count_log	;		                                                 //kpit change 27/9
    //register Word16   i,k;					                                             //kpit change 27/9
	register Word32   i,k;					                                                 //kpit change 27/9
    //Word16   index;
////	Word32   dct4s_index;							                                                   //kpit change 30/9
    //Word16   dummy;
////	Word32   dct4s_dummy;						                                                      //kpit change 30/9
////    Word32 	 dct4s_sum;
    const cos_msin_t* const	*table_ptr_ptr;
	const cos_msin_t     *cos_msin_ptr;

   // register Word32 acca;					                                                     //kpit change 27/9
    //register Word16 temp;					                                                   //kpit change 27/9

    //Word16   dct_length_log;
////	Word32   dct_length_log;		                                                         //kpit change 30/9
    const Word8   *dither_ptr;
    
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Do the sum/difference butterflies, the first part of */
    /* converting one N-point transform into 32 - 10 point transforms  */
    /* transforms, where N = 1 << DCT_LENGTH_LOG.           */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    test();
#ifdef SIREN14
    if (dct_length==DCT_LENGTH)
    {
#endif  
        dct_length_log = DCT_LENGTH_LOG;
        dither_ptr = dither;
#ifdef SIREN14
    }
    else
    {
        dct_length_log = MAX_DCT_LENGTH_LOG;
        dither_ptr = max_dither;
    }
#endif    
    in_buffer  = input;
    out_buffer = buffer_a;
    dct4s_index=0;
    i=0;
   
    for (set_count_log = 0;    set_count_log <= dct_length_log - 2;    set_count_log++) 
    {

        /*===========================================================*/
        /* Initialization for the loop over sets at the current size */
        /*===========================================================*/
        
        /*    set_span      = 1 << (DCT_LENGTH_LOG - set_count_log); */
        set_span = dct_length >> set_count_log;
           
        //set_count     = 1 << set_count_log;
        in_ptr        = in_buffer;
        next_out_base = out_buffer;
        
        /*=====================================*/
        /* Loop over all the sets of this size */
        /*=====================================*/
        //temp = (dct4s_index - 1);
        //if(temp < 0)
		if((dct4s_index - 1) < 0)	 //kpit change 28/9
        {
           // for (sets_left = set_count;sets_left > 0;sets_left--) 
		    for (sets_left = ( 1 << set_count_log);sets_left > 0;sets_left--) 	      //kpit change 30/9
            {
    
                /*||||||||||||||||||||||||||||||||||||||||||||*/
                /* Set up output pointers for the current set */
                /*||||||||||||||||||||||||||||||||||||||||||||*/
                /* pointer arithmetic */
                out_ptr_low    = next_out_base;
                next_out_base += set_span;
                out_ptr_high   = next_out_base;

                /*||||||||||||||||||||||||||||||||||||||||||||||||||*/
                /* Loop over all the butterflies in the current set */
                /*||||||||||||||||||||||||||||||||||||||||||||||||||*/
                    
                do 
                {
                    //in_val_low      = *in_ptr++;
                    //in_val_high     = *in_ptr++;

                    /* BEST METHOD OF GETTING RID OF BIAS, BUT COMPUTATIONALLY UNPLEASANT */
                    /* ALTERNATIVE METHOD, SMEARS BIAS OVER THE ENTIRE FRAME, COMPUTATIONALLY SIMPLEST. */
                    /* IF THIS WORKS, IT'S PREFERABLE */
                        
                    //dummy = add(in_val_low,dither_ptr[i++]);
                    //acca = L_add(dummy,in_val_high);
                    //out_val_low = extract_l(L_shr(acca,1));
                    dct4s_dummy = (*in_ptr + dither_ptr[i++]);	                                   //kpit change 29/9
														
                    //acca = (dummy + in_val_high);			                                  //kpit change 28/9
                    //out_val_low = extract_l(acca>>1);		                                 //kpit change 28/9
					out_val_low = extract_l(((dct4s_dummy + (*(in_ptr+1)) >>1)));	                 //kpit change 28/9
					
				
                    //acca = in_val_low + dither_ptr[i++] + in_val_high ;
                    //out_val_low = extract_l(acca/2);
					

                    
                    //dummy = add(in_val_low,dither_ptr[i++]);
                    //acca = L_add(dummy,-in_val_high);
                    //out_val_high = extract_l(L_shr(acca,1));
                    dct4s_dummy = ((*in_ptr) + dither_ptr[i++]);
					
				
                   // acca = (dummy - in_val_high);
                    //out_val_high = extract_l(acca>>1);
				    out_val_high = extract_l((dct4s_dummy - (*(in_ptr+1)))>>1);                  //kpit change 28/9
				
																		
                    //acca = in_val_low + dither_ptr[i++] + -in_val_high;
                    //out_val_high = extract_l(acca/2);
                    
                    *out_ptr_low++  = out_val_low;
                    *--out_ptr_high = out_val_high;
					
					in_ptr += 2;
                    
                    /* this involves comparison of pointers */
                    /* pointer arithmetic */

                } while (out_ptr_low < out_ptr_high);
    
            } /* End of loop over sets of the current size */
        }
        else
        {
            //for (sets_left = set_count;    sets_left > 0;    sets_left--) 
			for (sets_left = (1 << set_count_log);    sets_left > 0;    sets_left--)       //kpit change 30/9
            {
                /*||||||||||||||||||||||||||||||||||||||||||||*/
                /* Set up output pointers for the current set */
                /*||||||||||||||||||||||||||||||||||||||||||||*/
                
                out_ptr_low    = next_out_base;
                next_out_base += set_span;
                out_ptr_high   = next_out_base;

            	/*||||||||||||||||||||||||||||||||||||||||||||||||||*/
            	/* Loop over all the butterflies in the current set */
            	/*||||||||||||||||||||||||||||||||||||||||||||||||||*/
                
                do 
                {
                    //in_val_low      = *in_ptr++;
                    //in_val_high     = *in_ptr++;

                    //out_val_low     = add(in_val_low,in_val_high);
                    //out_val_high    = add(in_val_low,negate(in_val_high));
                    out_val_low     = (*in_ptr) + (*(in_ptr+1));
                    out_val_high    = (*in_ptr) - (*(in_ptr+1));
                    
                    *out_ptr_low++  = out_val_low;
                    *--out_ptr_high = out_val_high;
					in_ptr += 2;
               } while (out_ptr_low < out_ptr_high);
    
            } /* End of loop over sets of the current size */
        }

        /*============================================================*/
        /* Decide which buffers to use as input and output next time. */
        /* Except for the first time (when the input buffer is the    */
        /* subroutine input) we just alternate the local buffers.     */
        /*============================================================*/
        
        in_buffer = out_buffer;
        if (out_buffer == buffer_a)
        {
            out_buffer = output;		                                       //kpit change removed buffer_b
        }
        else
        {
            out_buffer = buffer_a;
        }
        
        //index = index + 1;
		  dct4s_index++;							                                  //kpit change 27/9
    } /* End of loop over set sizes */


    /*++++++++++++++++++++++++++++++++*/
    /* Do 32 - 10 point transforms */
    /*++++++++++++++++++++++++++++++++*/
    
    pair_ptr = in_buffer;
    //KPIT- buffer_swap = buffer_c;
	if(pair_ptr == buffer_a)
     buffer_swap = output;	                                                 //kpit change
	 else
	  buffer_swap = buffer_a;

	  temp_ptr = buffer_swap;

    for (pairs_left = 1 << (dct_length_log - 1);    pairs_left > 0;    pairs_left--) 
    {
        for ( k=0; k<CORE_SIZE; k++ )
        {
            dct4s_sum=0L;
            for ( i=0; i<CORE_SIZE; i++ )
            {
                dct4s_sum += pair_ptr[i] * dct_core_s[i][k];
            }
            buffer_swap[k] = round15(dct4s_sum);
        }
        
        pair_ptr   += CORE_SIZE;
        buffer_swap += CORE_SIZE;
    }
    
    for (i=0;i<dct_length;i++)
    {
        //KPIT- in_buffer[i] = buffer_c[i] removed;
		in_buffer[i] = temp_ptr[i];
    }

    table_ptr_ptr = s_cos_msin_table;
    /*++++++++++++++++++++++++++++++*/
    /* Perform rotation butterflies */
    /*++++++++++++++++++++++++++++++*/
    dct4s_index=0;
    
    for (set_count_log = dct_length_log - 2 ;    set_count_log >= 0;    set_count_log--) 
    {

        /*===========================================================*/
        /* Initialization for the loop over sets at the current size */
        /*===========================================================*/
        
        /*    set_span      = 1 << (DCT_LENGTH_LOG - set_count_log); */
        set_span = dct_length >> set_count_log;
        
        //set_count     = 1 << set_count_log;		                                 //kpit change 30/9
        next_in_base  = in_buffer;
        if (set_count_log == 0)
        {
            next_out_base = output;		
         }
        else
        {
            next_out_base = out_buffer;
        }
        
        /*=====================================*/
        /* Loop over all the sets of this size */
        /*=====================================*/

        //for (sets_left = set_count;    sets_left > 0;    sets_left--) 
		for (sets_left =(1 << set_count_log) ;    sets_left > 0;    sets_left--) 	  //kpit change 30/9
        {

            /*|||||||||||||||||||||||||||||||||||||||||*/
            /* Set up the pointers for the current set */
            /*|||||||||||||||||||||||||||||||||||||||||*/
            
            in_ptr_low     = next_in_base;
            
            //temp = set_span >> 1;
            //in_ptr_high    = in_ptr_low + temp;
			in_ptr_high    = in_ptr_low + (set_span >> 1);	                         //kpit change 28/9
            
            next_in_base  += set_span;
           
            out_ptr_low    = next_out_base;
            
            next_out_base += set_span;
            out_ptr_high   = next_out_base;
            
            cos_msin_ptr   = *table_ptr_ptr;

            /*||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
            /* Loop over all the butterfly pairs in the current set */
            /*||||||||||||||||||||||||||||||||||||||||||||||||||||||*/

	        do 
            {
                /* address arithmetic */
                //in_low_even     = *in_ptr_low++;
                //in_low_odd      = *in_ptr_low++;
                //in_high_even    = *in_ptr_high++;
                //in_high_odd     = *in_ptr_high++;
                //cos_even        = cos_msin_ptr[0].cosine;
                //msin_even       = cos_msin_ptr[0].minus_sine;
                //cos_odd         = cos_msin_ptr[1].cosine;
                //msin_odd        = cos_msin_ptr[1].minus_sine;
                //cos_msin_ptr   += 2;

                //sum = 0L;
                //sum+=cos_even*in_low_even;
				dct4s_sum=(cos_msin_ptr[0].cosine)*(*in_ptr_low);	                              //kpit change 28/9
                //neg_msin_even = -(cos_msin_ptr[0].minus_sine);
                dct4s_sum+=(-(cos_msin_ptr[0].minus_sine))*(*in_ptr_high);
				//sum *= 2;
                //out_low_even = round15(sum);
                //out_low_even = round15(sum);	                                         //29/9
				*out_ptr_low++  =  round14(dct4s_sum);


                //sum = 0L;				                                                kpit change 27/9
				//sum+=msin_even*in_low_even;
                dct4s_sum=(cos_msin_ptr[0].minus_sine)*(*in_ptr_low);	                        // kpit change 27/9
                dct4s_sum+=(cos_msin_ptr[0].cosine)*(*in_ptr_high);
                //out_high_even= round15(sum);		                                   //kpit change 29/9
				*--out_ptr_high =  round14(dct4s_sum);
				in_ptr_low++;
				in_ptr_high++;

                //sum = 0L;						                                        kpit change 27/9
                //sum+=cos_odd*in_low_odd;
				dct4s_sum=(cos_msin_ptr[1].cosine)*(*in_ptr_low);			                   //kpit change 27/9
                dct4s_sum+=(cos_msin_ptr[1].minus_sine)*(*in_ptr_high);
                //out_low_odd=round15(sum);	                                        	kpit change  29/9
				*out_ptr_low++  =  round14(dct4s_sum);	                                  //kpit change 29/9

                //sum = 0L;						                                         //kpit change 27/9
                //sum+=msin_odd*in_low_odd;
				dct4s_sum=(cos_msin_ptr[1].minus_sine)*(*in_ptr_low);		                    //kpit change 27/9
                //neg_cos_odd = -cos_odd;
                dct4s_sum+=(-(cos_msin_ptr[1].cosine))*(*in_ptr_high);
				//out_high_odd= round15(sum);
				*--out_ptr_high = 	round14(dct4s_sum);	                                	//kpit change 29/9

               // *out_ptr_low++  = out_low_even;
			  //  *out_ptr_low++  =  round15(sum);		                               //kpit change 29/9
                //*--out_ptr_high = out_high_even;
				//*--out_ptr_high =  round15(sum);		                              //kpit change 29/9
                //*out_ptr_low++  = out_low_odd;
				//*out_ptr_low++  =  round15(sum);	                                 //kpit change 29/9
                //*--out_ptr_high = out_high_odd;
				//*--out_ptr_high = 	round15(sum);		                         //kpit change 29/9
				cos_msin_ptr   += 2;
				in_ptr_low++;
				in_ptr_high++;
            } while (out_ptr_low < out_ptr_high);

	    } /* End of loop over sets of the current size */

        /*=============================================*/
        /* Swap input and output buffers for next time */
        /*=============================================*/
        
        buffer_swap = in_buffer;
        in_buffer   = out_buffer;
        out_buffer  = buffer_swap;
        
        //dct4s_index = dct4s_index + 1;   														//kpit change 27/9
		dct4s_index++;			   														//kpit change 27/9
        table_ptr_ptr++;
    }
    /*------------------------------------
    
         ADD IN BIAS FOR OUTPUT
         
    -----------------------------------*/
    if (dct_length==DCT_LENGTH)
    {
        for(i=0;i<320;i++) 
        {
           dct4s_sum = output[i] + syn_bias_7khz[i];
           //acca = sum - 32767;		   											//kpit change 28/9

           //if (acca > 0)															//kpit change 28/9
		   if ((dct4s_sum - 32767) > 0)
		    {
               dct4s_sum = 32767L;
           }
           //acca = sum + 32768L;													//kpit change 28/9
           //if (acca < 0)			  												//kpit change 28/9
		   if ((dct4s_sum + 32768L) < 0)
		    
           {
               dct4s_sum = -32768L;
           }
           output[i] = extract_l(dct4s_sum);
        }
    }
}

