/******************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   © 2004 Polycom, Inc.
**
**	 All rights reserved.
**
******************************************************************************/

/******************************************************************************
* Filename: samples_to_rmlt_coefs.c
*
* Purpose:  Convert Samples to Reversed MLT (Modulated Lapped Transform) 
*           Coefficients
*
*     The "Reversed MLT" is an overlapped block transform which uses
*     even symmetry * on the left, odd symmetry on the right and a
*     Type IV DCT as the block transform.  * It is thus similar to a
*     MLT which uses odd symmetry on the left, even symmetry * on the
*     right and a Type IV DST as the block transform.  In fact, it is
*     equivalent * to reversing the order of the samples, performing
*     an MLT and then negating all * the even-numbered coefficients.
*
******************************************************************************/

/***************************************************************************
 Include files                                                           
***************************************************************************/
#include "../common/defs.h"
#include "../common/tables.h"
#include "../common/count.h"
#define round15(x) (Word16)((x + (Word32)0x00004000L)>>15)
#define abs_s(x)   (Word16)((x<0) ? (x==0x8000) ? (Word16)0x7fff : -x : x)


/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : norm_s                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 16 bit varia- |
 |   ble var1 for positive values on the interval with minimum of 16384 and  |
 |   maximum of 32767, and for negative values on the interval with minimum  |
 |   of -32768 and maximum of -16384; in order to normalize the result, the  |
 |   following operation must be done :                                      |
 |                    norm_var1 = shl(var1,norm_s(var1)).                    |
 |                                                                           |
 |   Complexity weight : 15                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 000f.                |
 |___________________________________________________________________________|
*/
#if 0
Word16 norm_s (Word16 var1)
{
    register Word16 var_out;

    if (var1 == 0)
    {
        //var_out = 0;
	    return (0);
    }
    else
    {
        if (var1 == (Word16) 0xffff)
        {
            var_out = 15;
        }
        else
        {
            if (var1 < 0)
            {
                var1 = ~var1;
            }
            for (var_out = 0; var1 < 0x4000; var_out++)
            {
                var1 <<= 1;
            }
        }
    }
    return (var_out);
}
#endif
/* ------------------------- End of norm_s() ------------------------- */


/***************************************************************************
 Function:    samples_to_rmlt_coefs 

 Syntax:      Word16 samples_to_rmlt_coefs(new_samples, 
                                           old_samples,
                                           coefs,
                                           dct_length)
                    Word16 *new_samples;           
                    Word16 *old_samples;           
                    Word16 *coefs;                 
                    Word16 dct_length;

 Description: Convert samples to MLT coefficients

 Design Notes:

 WMOPS:     7kHz |    24kbit    |     32kbit
          -------|--------------|----------------
            AVG  |    1.40      |     1.40
          -------|--------------|----------------  
            MAX  |    1.40      |     1.40
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |     32kbit     |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    3.07      |     3.07       |     3.07
          -------|--------------|----------------|----------------
            MAX  |    3.10      |     3.10       |     3.10
          -------|--------------|----------------|----------------
				
***************************************************************************/

Word16 samples_to_rmlt_coefs(register Word16 *new_samples,register Word16 *old_samples,register Word16 *coefs,Word16 dct_length)
{

    Word16	index, vals_left,mag_shift,n;
   // Word16	windowed_data[MAX_DCT_LENGTH]; kpit change
    register Word16	/**new_ptr,*/ *old_ptr, *sam_low, *sam_high;
    const Word16	*win_low, *win_high;
    Word16	*dst_ptr;
    //Word16  neg_win_low;
    //Word16  samp_high;
    //Word16  half_dct_size;			                                     kpit change 30/9
    
    register Word32	acca;
   // Word32	accb;	                                                     kpit change 22/9
    //Word16	temp;
	 Word32	temp;				                                             //kpit change 30/9
   // Word16	temp1;
   	 Word32	temp1;				                                        	//kpit change 30/9
    //Word16	temp2;
    //Word16	temp5;	                                                     kpit change 22/9
   
    //half_dct_size = (dct_length >> 1);	                                //kpit change 30/9
   
    /*++++++++++++++++++++++++++++++++++++++++++++*/
    /* Get the first half of the windowed samples */
    /*++++++++++++++++++++++++++++++++++++++++++++*/
    
   // dst_ptr  = windowed_data;
    dst_ptr = coefs; //kpit change
    //move16();
    
    /* address arithmetic */
#ifdef SIREN14
    if (dct_length==DCT_LENGTH)
    {
        //win_high = samples_to_rmlt_window + half_dct_size;
		win_high = samples_to_rmlt_window + (dct_length >> 1);		      //kpit change 30/9
    }
    else
    {
        //win_high = max_samples_to_rmlt_window + half_dct_size;
		win_high = max_samples_to_rmlt_window + (dct_length >> 1) ;	      //kpit change 30/9
    }
#else
    //win_high = samples_to_rmlt_window + half_dct_size;
	win_high = samples_to_rmlt_window + (dct_length >> 1);	     	      //kpit change 30/9

#endif
    win_low  = win_high;
    //move16();		                                                       //kpit change 22/9
    
    /* address arithmetic */
   // sam_high = old_samples + half_dct_size;
    sam_high = old_samples + (dct_length >> 1);						      //kpit change 30/9
    
    sam_low  = sam_high;
    //move16();						                                       //kpit change 22/9
    
    //for (vals_left = half_dct_size;vals_left > 0;vals_left--)
	for (vals_left = (dct_length >> 1);vals_left > 0;vals_left--)	     //kpit change 30/9
    {
        //acca = 0L;		                                               kpit change 22/9
        //move32();			                                               kpit change 22/9
        
        //acca = L_mac(acca,*--win_low, *--sam_low);
        //acca = L_mac(acca,*win_high++, *sam_high++);
        //temp = round(acca); 
        //acca = *--win_low * *--sam_low;
        //acca += *win_high++ * *sam_high++;
        //acca *= 2;
        //temp = round15(acca);                                            kpit change 22/9
        
        //*dst_ptr++ = round15(acca);	 // kpit change 22/9
        *dst_ptr++ = round15((*--win_low * *--sam_low) + (*win_high++ * *sam_high++));	 // kpit change 22/9

        //move16();	 kpit change 22/9
    }           
    
    /*+++++++++++++++++++++++++++++++++++++++++++++*/
    /* Get the second half of the windowed samples */
    /*+++++++++++++++++++++++++++++++++++++++++++++*/
    
    sam_low  = new_samples;
    //move16();	   kpit change 22/9

    /* address arithmetic */
    sam_high = new_samples + dct_length;
    
    //for (vals_left = half_dct_size;    vals_left > 0;    vals_left--)
	for (vals_left = (dct_length >> 1);    vals_left > 0;    vals_left--)	   //kpit change 30/9
    {
      //  acca = 0L;                                                            kpit change 22/9
      //  move32();	                                                            kpit change 22/9

        //acca = L_mac(acca,*--win_high, *sam_low++);
        //neg_win_low = negate(*win_low++);
        //samp_high = *--sam_high;
        //acca = L_mac(acca, neg_win_low, samp_high);

        //acca = *--win_high *  *sam_low++;
        //neg_win_low = -(*win_low++);
        //samp_high = *--sam_high;
        //acca += neg_win_low * samp_high;
        //temp = round15(acca);                                                    kpit change 22/9
        
        *dst_ptr++=round15((*--win_high *  *sam_low++) + (-(*win_low++) * (*--sam_high)));	 //kpit change 28/9
        //move16();	                                                                kpit change 22/9
    }

       
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Save the new samples for next time, when they will be the old samples */
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    
    dst_ptr = new_samples;
    //move16();	                                                             kpit change 22/9  


    old_ptr = old_samples;
    //move16();		                                                         kpit change 22/9

    
    for (vals_left = dct_length;vals_left > 0;vals_left--)
    {
        *old_ptr++ = *dst_ptr++;
    //  move16();	                                                          kpit change 22/9
    }
    
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Calculate how many bits to shift up the input to the DCT.             */
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    
    temp1=0;
    move16();	 

    for(index=0;index<dct_length;index++)
    {
        temp = abs_s(coefs[index]);                                        //kpit change
        //temp = (temp2 - temp1);
        //test();		                                                 kpit change 22/9
	
        if((temp - temp1) > 0)	 			                            //kpit change 28/9
        {
            move16();
            temp1 = temp;
        }					 
    }
    
    mag_shift=0;
    //move16();				                                            kpit change 22/9

    temp = (temp1 - 14000);
    //test();				                                            kpit change 22/9
    //if (temp >= 0)		                                            kpit change 22/9
    //{
    //    mag_shift = 0;
    //    move16();
    //}						                                            kpit change 22/9
    if( temp < 0)
    {
        //temp = (temp1 - 438);	                                        kpit change 22/9
        test();				  
        if((temp1-438) < 0)
            temp = temp1 + 1;
        else 
        {
            temp = temp1;
            move16();			   
        }
        // accb = (temp * 19174);	                                    // kpit change 22/9
		//accb *= 2;			                                        kpit change 22/9
        acca = ((temp * 19174)>> 20);                                  // kpit change 22/9
        //temp5 = extract_l(acca);                                       kpit change 22/9
        temp = norm_s(extract_l(acca));
        if (temp == 0)			   
        {
            mag_shift = 9;
        }
        else
          mag_shift = (temp - 6);	  
       
    }

    acca = 0L;
    for(index=0; index<dct_length; index++)
    {
        //temp = abs_s(coefs[index]);			                    	//kpit change
        acca = (acca + abs_s(coefs[index]));
    }
    
    acca = (acca >> 7);
    if (temp1 < acca)
    {
       // mag_shift = (mag_shift-1);
	   mag_shift = mag_shift--;					                      //kpit change 30/9

    }
   if (mag_shift > 0) 
    {
        for(index=0;index<dct_length;index++)
        {
            coefs[index] = (coefs[index] << mag_shift);		           //kpit change
        }
    }
    else 
    {
        if (mag_shift < 0) 
        {
            n = -mag_shift;
            for(index=0;index<dct_length;index++)
            {
                coefs[index] = (coefs[index] >> n);	                   //kpit change
            }
        }
    }

    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Perform a Type IV DCT on the windowed data to get the coefficients */
    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    dct_type_iv_a(coefs, dct_length);                                  //kpit change

    return(mag_shift);
}
