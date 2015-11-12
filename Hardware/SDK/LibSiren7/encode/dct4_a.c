/*********************************************************************************
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   © 2004 Polycom, Inc.
**
**	 All rights reserved.
**
*********************************************************************************/

/*********************************************************************************
* Filename: dct_type_iv_a.c
*
* Purpose:  Discrete Cosine Transform, Type IV used for MLT
*
* The basis functions are
*
*	 cos(PI*(t+0.5)*(k+0.5)/block_length)
*
* for time t and basis function number k.  Due to the symmetry of the expression
* in t and k, it is clear that the forward and inverse transforms are the same.
*
*********************************************************************************/

/*********************************************************************************
 Include files                                                           
*********************************************************************************/
#include "../common/defs.h"
#include "../common/count.h"
#include "dct4_a.h"

/*********************************************************************************
 External variable declarations                                          
*********************************************************************************/
extern const Word8       anal_bias[DCT_LENGTH];
extern const Word16       dct_core_a[DCT_LENGTH_DIV_32][DCT_LENGTH_DIV_32];
extern const cos_msin_t   a_cos_msin_2 [DCT_LENGTH_DIV_32];
extern const cos_msin_t   a_cos_msin_4 [DCT_LENGTH_DIV_16];
extern const cos_msin_t   a_cos_msin_8 [DCT_LENGTH_DIV_8];
extern const cos_msin_t   a_cos_msin_16[DCT_LENGTH_DIV_4];
extern const cos_msin_t   a_cos_msin_32[DCT_LENGTH_DIV_2];
extern const cos_msin_t   a_cos_msin_64[DCT_LENGTH];
//extern cos_msin_t* const   a_cos_msin_table[];
extern const cos_msin_t* const a_cos_msin_table[];
#define round15(x) (Word16)((x + (Word32)0x00004000L)>>15)

/*********************************************************************************
 Function:    dct_type_iv_a

 Syntax:      void dct_type_iv_a (input, output, dct_length) 
                        Word16   input[], output[], dct_length;              

 Description: Discrete Cosine Transform, Type IV used for MLT

 Design Notes:
                
 WMOPS:          |    24kbit    |     32kbit
          -------|--------------|----------------
            AVG  |    1.14      |     1.14
          -------|--------------|----------------  
            MAX  |    1.14      |     1.14
          -------|--------------|---------------- 
                
           14kHz |    24kbit    |     32kbit     |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    2.57      |     2.57       |     2.57
          -------|--------------|----------------|----------------
            MAX  |    2.57      |     2.57       |     2.57
          -------|--------------|----------------|----------------

*********************************************************************************/

    void dct_type_iv_a (Word16 *output,Word16 dct_length)	                        //kpit change
{
    Word16   buffer_a[MAX_DCT_LENGTH];    //KPIT-buffer_b[MAX_DCT_LENGTH];           KPIT- buffer_c[MAX_DCT_LENGTH] removed;
    Word16   *in_ptr, *in_ptr_low, *in_ptr_high, *next_in_base, *temp_ptr;	       //*temp_ptr added by kpit
    Word16   *out_ptr_low, *out_ptr_high, *next_out_base;
    Word16   *out_buffer, *in_buffer, *buffer_swap;
   // Word16   in_val_low, in_val_high;
    Word16   out_val_low, out_val_high;
    //Word16   in_low_even, in_low_odd;
    //Word16   in_high_even, in_high_odd;
    //Word16   out_low_even, out_low_odd;
    //Word16   out_high_even, out_high_odd;
    Word16   *pair_ptr;
    //Word16   cos_even, cos_odd, msin_even, msin_odd;
    //Word16   neg_cos_odd;
    //Word16   neg_msin_even;
    Word32   sum;
  //  Word16   set_span, /*set_count,*//* set_count_log,*/ pairs_left/*, sets_left*/;    //kpit change 30/9
	Word16   set_span,pairs_left;
	register Word16   set_count_log,sets_left;		                                     //kpit change 24/9
	register Word16   i;
	Word16 k;		
	
   // Word16   index;
    const cos_msin_t* const *table_ptr_ptr;
	const cos_msin_t   *cos_msin_ptr;
    
    //register Word16   temp;
    //register Word32   acca;

    Word16   dct_length_log;



    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Do the sum/difference butterflies, the first part of */
    /* converting one N-point transform into N/2 two-point  */
    /* transforms, where N = 1 << DCT_LENGTH_LOG. = 64/128  */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    test();
#ifdef SIREN14
    if (dct_length==DCT_LENGTH)
    {
#endif
        dct_length_log = DCT_LENGTH_LOG;

        /* Add bias offsets */
        for (i=0;i<dct_length;i++)
        {
            output[i] = (output[i] + anal_bias[i]);
        }
#ifdef SIREN14
    }
    else
        dct_length_log = MAX_DCT_LENGTH_LOG;
#endif

    //index = 0L;
	in_buffer  = output;				                                              //kpit change

    out_buffer = buffer_a;

    //temp = dct_length_log-2;
   i = dct_length_log-2;

    for (set_count_log=0;set_count_log<=i;set_count_log++)
    {

        /*===========================================================*/
        /* Initialization for the loop over sets at the current size */
        /*===========================================================*/

        /*    set_span      = 1 << (DCT_LENGTH_LOG - set_count_log); */
        set_span = dct_length>>set_count_log;					
        //set_count     = 1<<set_count_log;						                   //kpit change 30/9

        in_ptr        = in_buffer;
        next_out_base = out_buffer;

        /*=====================================*/
        /* Loop over all the sets of this size */
        /*=====================================*/

        for (sets_left=(1<<set_count_log);sets_left>0;sets_left--)		            //kpit change 30/9
        {

            /*||||||||||||||||||||||||||||||||||||||||||||*/
            /* Set up output pointers for the current set */
            /*||||||||||||||||||||||||||||||||||||||||||||*/

            out_ptr_low    = next_out_base;
            next_out_base  = next_out_base + set_span;
			 out_ptr_high   = next_out_base;

            /*||||||||||||||||||||||||||||||||||||||||||||||||||*/
            /* Loop over all the butterflies in the current set */
            /*||||||||||||||||||||||||||||||||||||||||||||||||||*/

            do 
            {
                //in_val_low      = *in_ptr++;
                //in_val_high     = *in_ptr++;
                //acca            = in_val_low+in_val_high;
				//sum            = (in_val_low+in_val_high)>>1;	                        //kpit change 28/9
				sum            = ((*in_ptr) + (*(in_ptr+1)))>>1;	                    //kpit change 28/9	29/9
                //acca            = acca>>1;
                out_val_low     = extract_l(sum);

                //acca            = in_val_low-in_val_high;
                //acca            = acca>>1;
				//sum            = (in_val_low-in_val_high)>>1;	                        //kpit change 28/9  29/9
				sum            =   ((*in_ptr) -(*(in_ptr+1)))>>1;	                     //kpit change 29/9
                out_val_high    = extract_l(sum);

                *out_ptr_low++  = out_val_low;
                *--out_ptr_high = out_val_high;
				in_ptr += 2;								                             //kpit change 29/9
            } while (out_ptr_low < out_ptr_high);

        } /* End of loop over sets of the current size */

        /*============================================================*/
        /* Decide which buffers to use as input and output next time. */
        /* Except for the first time (when the input buffer is the    */
        /* subroutine input) we just alternate the local buffers.     */
        /*============================================================*/

        in_buffer = out_buffer;		
        if (out_buffer == buffer_a)
			  out_buffer = output;  
            //out_buffer = buffer_b;  KPIT CHANGE BUFFER_B removed
        else
            out_buffer = buffer_a;
        //index = index+1;
		//index= index++;											                      //kpit change 28/9

    } /* End of loop over set sizes */


    /*++++++++++++++++++++++++++++++++*/
    /* Do N/2 two-point transforms,   */
    /* where N =  1 << DCT_LENGTH_LOG */
    /*++++++++++++++++++++++++++++++++*/

    pair_ptr = in_buffer;
    //KPIT- buffer_swap = buffer_c;
	if( pair_ptr == buffer_a)	                                                             //kpit change
		buffer_swap = output;	                                                             //kpit change
	else
		buffer_swap = buffer_a;			
	temp_ptr = buffer_swap;	                                                                  //kpit change

    //temp = dct_length_log-1;											                     //kpit change 28/9
   // temp = 1<<temp;													                     //kpit change 28/9

    //for (pairs_left=temp; pairs_left > 0; pairs_left--)
	for (pairs_left=(1<<(dct_length_log-1)); pairs_left > 0; pairs_left--)                   //kpit change 28/9
    {
        for ( k=0; k<CORE_SIZE; k++ )
        {
            sum=0L;
            for ( i=0; i<CORE_SIZE; i++ )
            {
                //sum = L_mac(sum, pair_ptr[i],dct_core_a[i][k]);
                sum += pair_ptr[i]*dct_core_a[i][k];
            }
            buffer_swap[k] = round15(sum);
        }
        /* address arithmetic */
        pair_ptr   += CORE_SIZE;
        buffer_swap += CORE_SIZE;
    }

    for (i=0;i<dct_length;i++)
    {
        //KPIT- in_buffer[i] = buffer_c[i];
		in_buffer[i] = temp_ptr[i];	                                                          //kpit -change
    }
    
    table_ptr_ptr = a_cos_msin_table;

    /*++++++++++++++++++++++++++++++*/
    /* Perform rotation butterflies */
    /*++++++++++++++++++++++++++++++*/
    //temp = sub(dct_length_log,2);
    //temp = dct_length_log-2;				  //kpit change 28/9
    //for (set_count_log = temp; set_count_log >= 0;    set_count_log--)
	for (set_count_log = (dct_length_log-2); set_count_log >= 0;    set_count_log--)         //kpit change 28/9
    {
        /*===========================================================*/
        /* Initialization for the loop over sets at the current size */
        /*===========================================================*/
        /*    set_span      = 1 << (DCT_LENGTH_LOG - set_count_log); */
        //set_span = shr(dct_length,set_count_log);
		set_span = dct_length >> set_count_log;

        //set_count     = 1<<set_count_log;			                                          //kpit change 30/9
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
        //for (sets_left = set_count; sets_left > 0;sets_left--)
		for (sets_left = (1<<set_count_log); sets_left > 0;sets_left--)	                      //kpit change 30/9
        {
            /*|||||||||||||||||||||||||||||||||||||||||*/
            /* Set up the pointers for the current set */
            /*|||||||||||||||||||||||||||||||||||||||||*/
            in_ptr_low     = next_in_base;
           // temp           = set_span>>1;				                                       //kpit change 28/9

            /* address arithmetic */
           // in_ptr_high    = in_ptr_low + temp;		                                        //kpit change 28/9
		    in_ptr_high    = in_ptr_low + (set_span>>1);                                       //kpit change 28/9
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
				sum=(cos_msin_ptr[0].cosine)*(*in_ptr_low);	                                //kpit change 28/9
                //neg_msin_even = -(cos_msin_ptr[0].minus_sine);
                sum+=(-(cos_msin_ptr[0].minus_sine))*(*in_ptr_high);
				//sum *= 2;
                //out_low_even = round15(sum);
                //out_low_even = round15(sum);	                                           //kpit change 29/9
				*out_ptr_low++  =  round15(sum);


                //sum = 0L;				                                                    kpit change 27/9
				//sum+=msin_even*in_low_even;
                sum=(cos_msin_ptr[0].minus_sine)*(*in_ptr_low);	                           // kpit change 27/9
                sum+=(cos_msin_ptr[0].cosine)*(*in_ptr_high);
                //out_high_even= round15(sum);		                                       //kpit change 29/9
				*--out_ptr_high =  round15(sum);
				in_ptr_low++;
				in_ptr_high++;

                //sum = 0L;						                                              kpit change 27/9
                //sum+=cos_odd*in_low_odd;
				sum=(cos_msin_ptr[1].cosine)*(*in_ptr_low);			                        //kpit change 27/9
                sum+=(cos_msin_ptr[1].minus_sine)*(*in_ptr_high);
                //out_low_odd=round15(sum);		                                             kpit change 29/9
				*out_ptr_low++  =  round15(sum);	                                        //kpit change 29/9

                //sum = 0L;					                                            	//kpit change 27/9
                //sum+=msin_odd*in_low_odd;
				sum=(cos_msin_ptr[1].minus_sine)*(*in_ptr_low);		                         //kpit change 27/9
                //neg_cos_odd = -cos_odd;
                sum+=(-(cos_msin_ptr[1].cosine))*(*in_ptr_high);
				//out_high_odd= round15(sum);
				*--out_ptr_high = 	round15(sum);		                                     //kpit change 29/9

               // *out_ptr_low++  = out_low_even;
			  //  *out_ptr_low++  =  round15(sum);		                                     //kpit change 29/9
                //*--out_ptr_high = out_high_even;
				//*--out_ptr_high =  round15(sum);		                                     //kpit change 29/9
                //*out_ptr_low++  = out_low_odd;
				//*out_ptr_low++  =  round15(sum);	                                         //kpit change 29/9
                //*--out_ptr_high = out_high_odd;
				//*--out_ptr_high = 	round15(sum);		                                 //kpit change 29/9
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
        table_ptr_ptr++;
    }
}

