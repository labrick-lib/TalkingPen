/*****************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   © 2004 Polycom, Inc.
**
**   All rights reserved.
**
*****************************************************************************/

/*****************************************************************************
* Filename: rmlt_coefs_to_samples.c
*
* Purpose:  Convert Reversed MLT (Modulated Lapped Transform) 
*           Coefficients to Samples
*
*     The "Reversed MLT" is an overlapped block transform which uses
*     even symmetry * on the left, odd symmetry on the right and a
*     Type IV DCT as the block transform.  * It is thus similar to a
*     MLT which uses odd symmetry on the left, even symmetry * on the
*     right and a Type IV DST as the block transform.  In fact, it is
*     equivalent * to reversing the order of the samples, performing
*     an MLT and then negating all * the even-numbered coefficients.
*
*****************************************************************************/

/***************************************************************************
 Include files                                                           
***************************************************************************/
#include "../common/defs.h"
#include "../common/tables.h"
#include "../common/count.h"

tsRmlt_coefs_to_samples sRmlt_coefs_to_samples __attribute__ ((section("playvars"), zero_init));
/***************************************************************************
 Function:     round_sum
              
 Description:  Takes the lapped sum output and rounds to 16 bits.
               sum is esentially in 19.13 format.
***************************************************************************/
Word16 round_sum (Word32 sum)
{
    register Word32 L_var1;			                                             //kpit change 28/7
    Word16 var_out;

    L_var1 = (sum + (32768L>>3))>>13;

    if (L_var1 > 0X00007fffL)
        var_out = MAX_16;
    else if (L_var1 < (Word32) 0xffff8000L)
        var_out = MIN_16;
    else
        var_out = (Word16) (L_var1);
    return (var_out);
}
/***************************************************************************
 Function:     rmlt_coefs_to_samples 

 Syntax:       void rmlt_coefs_to_samples(Word16 *coefs,       
                                          Word16 *old_samples, 
                                          Word16 *out_samples, 
                                          Word16 dct_length,
                                          Word16 mag_shift)    
            
               inputs:    Word16 *coefs
                          Word16 *old_samples
                          Word16 dct_length
                          Word16 mag_shift
                          
                          
               outputs:   Word16 *out_samples
               
 Description:  Converts the mlt_coefs to samples

 Design Notes:
 
 WMOPS:     7kHz |    24kbit    |    32kbit
          -------|--------------|----------------
            AVG  |     1.91     |    1.91
          -------|--------------|----------------  
            MAX  |     1.91     |    1.91
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |    32kbit      |     48kbit
          -------|--------------|----------------|----------------
            AVG  |     3.97     |    3.97        |     3.97   
          -------|--------------|----------------|----------------
            MAX  |     3.97     |    3.97        |     3.97   
          -------|--------------|----------------|----------------

***************************************************************************/
void rmlt_coefs_to_samples(Word16 *coefs,     
                           Word16 *old_samples,
                           Word16 *out_samples,           
                           Word16 dct_length,
                           Word16 mag_shift)             
{

    
    register Word16	index, vals_left;	                                               //kpit change 27/9
////    Word16	c2s_new_samples[MAX_DCT_LENGTH];
    register Word16	*new_ptr, *old_ptr;	                                              //kpit change 27/9
    const Word16	*win_new, *win_old;
////    Word16	*out_ptr;
    //Word16  half_dct_size;				                                          //kpit change 29/9
    //Word32  sum;							                                         //kpit change 29/9

    //half_dct_size = (dct_length >> 1);		                                    //kpit change 29/9
    
    /* Perform a Type IV (inverse) DCT on the coefficients */
    dct_type_iv_s(coefs, c2s_new_samples, dct_length);
    
    test();
    if (mag_shift > 0) 
    {
        for(index=0;index<dct_length;index++)
        {
            c2s_new_samples[index] = (c2s_new_samples[index] >> mag_shift);
            move16();
        }
    }
    else 
    {
        test();
       // if (mag_shift < 0) 					                                     //kpit chnage 30/9
        //{										                                    //kpit chnage 30/9
            mag_shift = -mag_shift;
            for(index=0;index<dct_length;index++)
            {
                c2s_new_samples[index] = (c2s_new_samples[index] << mag_shift);
                move16();
            }
       // } 										                              //kpit chnage 30/9

    }

    /* Get the first half of the windowed samples */
    
    out_ptr = out_samples;
    move16();
    test();
#ifdef SIREN14    
    if (dct_length==DCT_LENGTH)
    {
        win_new = rmlt_to_samples_window;
        move16();
        win_old = rmlt_to_samples_window + dct_length;
        move16();
    }
    else
    {
        win_new = max_rmlt_to_samples_window;
        move16();
        win_old = max_rmlt_to_samples_window + dct_length;
        move16();
    }
#else
    win_new = rmlt_to_samples_window;
    win_old = rmlt_to_samples_window + dct_length;
#endif
    old_ptr = old_samples;
    move16();
    //new_ptr = new_samples + half_dct_size;
	new_ptr = c2s_new_samples + (dct_length >> 1);		                               //kpit change 29/9
    move16();
    
    //for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
	for (vals_left = (dct_length >> 1);    vals_left > 0;    vals_left--)	      //kpit change 29/9
    {
       // sum = 0L;							                                      //kpit change 28/9
        move32();
        //sum = L_mac(sum,*win_new++, *--new_ptr);
        //sum = L_mac(sum,*--win_old, *old_ptr++);
        //*out_ptr++ = round(L_shl(sum,2));
        //sum += *win_new++ * *--new_ptr;                                         //kpit change 28/9
        //sum += *--win_old * *old_ptr++;                                         //kpit change 28/9
        //*out_ptr++ = round_sum (sum);
		*out_ptr++ = round_sum (((*win_new++ )* (*--new_ptr))+((*--win_old )* (*old_ptr++)));   //kpit change 28/9
        move16();

    }
    
    /* Get the second half of the windowed samples */
    
    //for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
	for (vals_left = (dct_length >> 1);    vals_left > 0;    vals_left--)	             //kpit change 29/9
    {
        //sum = 0L;									                                     //kpit change 28/9
        move32();
        //sum = L_mac(sum,*win_new++, *new_ptr++);
        //sum = L_mac(sum,negate(*--win_old), *--old_ptr);
        //*out_ptr++ = round(L_shl(sum,2));
        //sum += *win_new++ *  *new_ptr++;		                                        //kpit change 28/9
        //sum += -(*--win_old) * *--old_ptr;	                                       //kpit change 28/9
       // *out_ptr++ = round_sum (sum);			                                        //kpit change 28/9
		*out_ptr++ = round_sum (((*win_new++ )*(*new_ptr++))+((-(*--win_old))*(*--old_ptr)));    //kpit change 28/9
        move16();
    }
        
    /* Save the second half of the new samples for   */
    /* next time, when they will be the old samples. */
    
    /* pointer arithmetic */
   // new_ptr = new_samples + half_dct_size;
    new_ptr = c2s_new_samples + (dct_length >> 1);		                                      //kpit change 29/9
    move16();
    old_ptr = old_samples;
    move16();
    //for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
	for (vals_left =(dct_length >> 1);    vals_left > 0;    vals_left--)                 //kpit change 29/9
    {
        *old_ptr++ = *new_ptr++;
        move16();
    }
}
