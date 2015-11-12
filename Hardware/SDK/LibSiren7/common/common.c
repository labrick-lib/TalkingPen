/****************************************************************************************
**
**   ITU-T G.722.1 (2005-05) - Fixed point implementation for main body and Annex C
**   > Software Release 2.1 (2008-06)
**     (Simple repackaging; no change from 2005-05 Release 2.0 code)
**
**   © 2004 Polycom, Inc.
**
**	 All rights reserved.
**
****************************************************************************************/

/****************************************************************************************
  Filename:    common.c    

  Purpose:     Contains the functions used for both G.722.1 Annex C encoder and decoder
		
  Design Notes:

****************************************************************************************/
/****************************************************************************************
 Include files                                                           
****************************************************************************************/
#include "defs.h"
#include "huff_def.h"
#include "huff_tab.h"
#include "tables.h"
#include "count.h"
/****************************************************************************************
 Function:    categorize

 Syntax:      void categorize(Word16 number_of_available_bits,   
                              Word16 number_of_regions,
                              Word16 num_categorization_control_possibilities,
                              Word16 rms_index,                  
                              Word16 power_categories,           
                              Word16 category_balances)          

                  inputs:   number_of_regions
                            num_categorization_control_possibilities
                            number_of_available_bits
                            rms_index[MAX_NUMBER_OF_REGIONS]                              
                  
                  outputs:  power_categories[MAX_NUMBER_OF_REGIONS]                       
                            category_balances[MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES-1]

 Description: Computes a series of categorizations 

 WMOPS:     7kHz |    24kbit    |     32kbit
          -------|--------------|----------------
            AVG  |    0.14      |     0.14
          -------|--------------|----------------  
            MAX  |    0.15      |     0.15
          -------|--------------|---------------- 
   			
           14kHz |    24kbit    |     32kbit     |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.42      |     0.45       |     0.48   
          -------|--------------|----------------|----------------
            MAX  |    0.47      |     0.52       |     0.52   
          -------|--------------|----------------|----------------

****************************************************************************************/


void categorize(Word16 number_of_available_bits,
		        Word16 number_of_regions,
		        Word16 num_categorization_control_possibilities,
		        Word16 *rms_index,
		        Word16 *power_categories,
		        Word16 *category_balances)
{
    
	
   
    Word16 offset;
    //Word16 temp;			 																	//kpit change 28/9
    Word16 frame_size;

    /* At higher bit rates, there is an increase for most categories in average bit
       consumption per region. We compensate for this by pretending we have fewer
       available bits. */
    test();
    if (number_of_regions == MAX_NUMBER_OF_REGIONS)
    {
        frame_size = MAX_DCT_LENGTH;
    }
    else
    {
        frame_size = DCT_LENGTH;
    }

    //temp = number_of_available_bits - frame_size;
    
    test();

    //if (temp > 0)
	if ((number_of_available_bits - frame_size) > 0) 											//kpit change 28/9
    {
        // estimated number_of_available_bits =   ((number_of_available_bits - 320) * 5/8) + 320
		number_of_available_bits = ((number_of_available_bits - frame_size) * 5)/8 + frame_size;
    }

    /* calculate the offset using the original category assignments */

	//offset = calc_offset(rms_index,number_of_regions,number_of_available_bits)
    offset = calc_offset(rms_index,number_of_regions,number_of_available_bits); 



    /* compute the power categories based on the uniform offset */
    compute_raw_pow_categories(power_categories,rms_index,number_of_regions,offset);
    
    
    /* adjust the category assignments */
    /* compute the new power categories and category balances */
    comp_powercat_and_catbalance(power_categories,category_balances,rms_index,number_of_available_bits,number_of_regions,num_categorization_control_possibilities,offset);

}
    
/***************************************************************************
 Function:    comp_powercat_and_catbalance

 Syntax:      void comp_powercat_and_catbalance(Word16 *power_categories,
                                                Word16 *category_balances,
                                                Word16 *rms_index,  
                                                Word16 number_of_available_bits,                                  
                                                Word16 number_of_regions,
                                                Word16 num_categorization_control_possibilities,
                                                Word16 offset)                                                    

                
                inputs:   *rms_index                              
                          number_of_available_bits                 
                          number_of_regions
                          num_categorization_control_possibilities
                          offset
                           
                outputs:  *power_categories  
                          *category_balances 
                
                
 Description: Computes the power_categories and the category balances

 WMOPS:     7kHz |    24kbit    |     32kbit
          -------|--------------|----------------
            AVG  |    0.10      |     0.10
          -------|--------------|----------------  
            MAX  |    0.11      |     0.11
          -------|--------------|---------------- 
				
           14kHz |    24kbit    |     32kbit     |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.32      |     0.35       |     0.38   
          -------|--------------|----------------|----------------
            MAX  |    0.38      |     0.42       |     0.43   
          -------|--------------|----------------|----------------

***************************************************************************/
void comp_powercat_and_catbalance(Word16 *power_categories,
                                  Word16 *category_balances,
                                  Word16 *rms_index,
                                  Word16 number_of_available_bits,
                                  Word16 number_of_regions,
                                  Word16 num_categorization_control_possibilities,
								  Word16 offset)
{
    
    Word16 expected_number_of_code_bits;
    //register Word16 region;
	register Word32 region;				 												//kpit change 30/9
    //Word16 max_region;				  												//kpit change 30/9
    register Word16 j;
    Word16 max_rate_categories[MAX_NUMBER_OF_REGIONS];
    Word16 min_rate_categories[MAX_NUMBER_OF_REGIONS];
    Word16 temp_category_balances[2*MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES];
    Word16 raw_max, raw_min;
	// GBJ added initialization to indexes to avoid crash.
    Word16 raw_max_index=0, raw_min_index=0;
    Word16 max_rate_pointer, min_rate_pointer;
    Word16 max, min;
    Word16 itemp0;
	Word16 i;		   																//kpit change 30/9
    //Word16 itemp1;
    //Word16 min_plus_max;
    //Word16 two_x_number_of_available_bits;									 //kpit change 30/9		   	

    Word16 temp;

    expected_number_of_code_bits = 0;
    move16();
	
#ifdef _CVI_
    for (region=0; region<2*MAX_NUM_CATEGORIZATION_CONTROL_POSSIBILITIES; region++)
		temp_category_balances[region] = 0;
#endif

    for (region=0; region<number_of_regions; region++)
        expected_number_of_code_bits = expected_number_of_code_bits + expected_bits_table[power_categories[region]];


    for (region=0; region<number_of_regions; region++) 
    {
        max_rate_categories[region] = power_categories[region];
        move16();
        
        min_rate_categories[region] = power_categories[region];
        move16();
    }
    
    max = expected_number_of_code_bits;
    move16();
    min = expected_number_of_code_bits;
    move16();
    max_rate_pointer = num_categorization_control_possibilities;
    move16();
    min_rate_pointer = num_categorization_control_possibilities;
    move16();
    i= (num_categorization_control_possibilities-1);								//kpit change 30/9
    //for (j=0; j<num_categorization_control_possibilities-1; j++) 
	for (j=0; j<i; j++)									 							//kpit change 30/9
    {
       // min_plus_max = (max + min);
        //two_x_number_of_available_bits = number_of_available_bits * 2;	   		kpit change 30/9
        
       // temp = min_plus_max - two_x_number_of_available_bits;
        test();
        //if (temp <= 0) 
		//if (((max + min)-two_x_number_of_available_bits) <= 0)	 				//kpit change 28/9
		if (((max + min)-(number_of_available_bits <<1)) <= 0)	 					//kpit change 28/9	 30/9
        {
            raw_min = 99;
            move16();
            /* Search from lowest freq regions to highest for best */
            /* region to reassign to a higher bit rate category.   */
            for (region=0; region<number_of_regions; region++) 
            {
                test();
                if (max_rate_categories[region] > 0) 
                {
                    itemp0 = (max_rate_categories[region]<<1);
                    //itemp1 = offset - rms_index[region];							//kpit change 28/9
                    itemp0 = ((offset - rms_index[region]) - itemp0);
                    
                    //temp = (itemp0 - raw_min);									//kpit change 28/9
                    test();
                    if ((itemp0 - raw_min) < 0) 
                    {
                        raw_min = itemp0;
                        raw_min_index = region;
                    }
                }
            }
            //max_rate_pointer = (max_rate_pointer - 1);
			max_rate_pointer = max_rate_pointer--;	  								//kpit change 30/9
            temp_category_balances[max_rate_pointer] = raw_min_index;
            move16();

        	max = (max - expected_bits_table[max_rate_categories[raw_min_index]]);
        	max_rate_categories[raw_min_index] = (max_rate_categories[raw_min_index] - 1);
        	move16();

            max = (max + expected_bits_table[max_rate_categories[raw_min_index]]);
        }
        else 
        {
            raw_max = -99;
            move16();
            /* Search from highest freq regions to lowest for best region to reassign to
            a lower bit rate category. */
           // max_region = (number_of_regions - 1);
            // for (region= max_region; region >= 0; region--) 
			 for (region=(number_of_regions - 1); region >= 0; region--)   						//kpit change 30/9
            {
                temp = (min_rate_categories[region] - (NUM_CATEGORIES-1));
                test();
				
                if (temp < 0)
                {
                    itemp0 = (min_rate_categories[region]<<1);
                  //  itemp1 = (offset - rms_index[region]);	   				     		//kpit change 28/9
                    itemp0 = ((offset - rms_index[region]) - itemp0);
                    
                    //temp = (itemp0 - raw_max);										 //kpit change 28/9
                    test();
                    if (((itemp0 - raw_max)) > 0) 			 							//kpit change 28/9
                    {
                        raw_max = itemp0;
                        move16();
                        raw_max_index = region;
                        move16();
                    }
                }
            }
		
            temp_category_balances[min_rate_pointer] = raw_max_index;
            move16();
            
            min_rate_pointer = (min_rate_pointer + 1);
            min = (min - expected_bits_table[min_rate_categories[raw_max_index]]);
            
            min_rate_categories[raw_max_index] = (min_rate_categories[raw_max_index] + 1);
            move16();
            // GBJ 4/15/10 Clamp minrate categories to NUM_CATEGORIES-1
			if( min_rate_categories[raw_max_index] > NUM_CATEGORIES-1)
				min_rate_categories[raw_max_index] = NUM_CATEGORIES-1;
			
            min = (min + expected_bits_table[min_rate_categories[raw_max_index]]);
        }
    }
    
    for (region=0; region<number_of_regions; region++)
    {
        power_categories[region] = max_rate_categories[region];
        move16();
    }
    
	i=num_categorization_control_possibilities-1;	   										//kpit change 30/9
   // for (j=0; j<num_categorization_control_possibilities-1; j++)
    for (j=0; j<i; j++)				 														//kpit change 30/9
    {
        category_balances[j] = temp_category_balances[max_rate_pointer++];
        move16();
    }

}
/***************************************************************************
 Function:    calc_offset

 Syntax:      offset=calc_offset(Word16 *rms_index,Word16 number_of_regions,Word16 available_bits)  

                input:  Word16 *rms_index
                        Word16 number_of_regions
                        Word16 available_bits
                        
                output: Word16 offset                        

 Description: Calculates the the category offset.  This is the shift required
              To get the most out of the number of available bits.  A binary 
              type search is used to find the offset.

 WMOPS:     7kHz |    24kbit    |     32kbit
          -------|--------------|----------------
            AVG  |    0.04      |     0.04
          -------|--------------|----------------  
            MAX  |    0.04      |     0.04
          -------|--------------|---------------- 

           14kHz |    24kbit    |     32kbit     |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.08      |     0.08       |     0.08   
          -------|--------------|----------------|----------------
            MAX  |    0.09      |     0.09       |     0.09   
          -------|--------------|----------------|----------------

***************************************************************************/
Word16 calc_offset(Word16 *rms_index,Word16 number_of_regions,Word16 available_bits)  
{
    
    Word16 answer;
    Word16 delta;
    Word16 test_offset;
    register Word16 j;
	register Word32 region; 														//kpit change 30/9
    Word16 power_cats[MAX_NUMBER_OF_REGIONS];	
    Word16 bits;
//    Word16 offset;	   															//kpit change 28/9
 //   Word16 temp;																	//kpit change 28/9

    /* initialize vars */
    answer = -32;
    move16();
    delta = 32;
    move16();
    
    do 
    {
        test_offset =  (answer + delta);
        
        /* obtain a category for each region */
        /* using the test offset             */
        for (region=0; region<number_of_regions; region++) 
        {
            j = (test_offset - rms_index[region])>>1;	  					     //kpit change 28/9
		    // j = j >> 1;								   
            
            /* Ensure j is between 0 and NUM_CAT-1 */
            test();
            if (j < 0) 
            {
                j = 0;
                move16();
            }
            //temp = (j - (NUM_CATEGORIES-1));				 					//kpit change 28/9
            test();
            //if (temp > 0) 
			if ((j - (NUM_CATEGORIES-1)) > 0)				  					//kpit change 28/9
            {
                j = (NUM_CATEGORIES-1);
                move16();
            }

            power_cats[region] = j;
			  
            move16();
        }
        bits = 0;
        move16();

        /* compute the number of bits that will be used given the cat assignments */
        for (region=0; region<number_of_regions; region++)

           // bits = (bits + expected_bits_table[power_cats[region]]);
			  bits = (bits + expected_bits_table[power_cats[region]]);  

        /* if (bits > available_bits - 32) then divide the offset region for the bin search */
        //offset = (available_bits - 32);
        //temp = (bits - offset);
	//	temp = (bits - (available_bits - 32));	 										//kpit change 28/9
        test();
        //if (temp >= 0)
		if ((bits - (available_bits - 32)) >= 0)	 									//kpit change 28/9
        {
            answer = test_offset;
            move16();
        }
        delta = delta >> 1;
        test(); /* for the while loop */
    } while (delta > 0);

    return(answer);
}
/***************************************************************************
 Function:    compute_raw_pow_categories

 Syntax:      void compute_raw_pow_categories(Word16 *power_categories,
                                              Word16 *rms_index,
                                              Word16 number_of_regions,
                                              Word16 offset)
              inputs:  *rms_index
                       number_of_regions
                       offset
                    
              outputs: *power_categories                    



 Description: This function computes the power categories given the offset
              This is kind of redundant since they were already computed
              in calc_offset to determine the offset.

 WMOPS:          |    24kbit    |     32kbit
          -------|--------------|----------------
            AVG  |    0.01      |     0.01
          -------|--------------|----------------  
            MAX  |    0.01      |     0.01
          -------|--------------|---------------- 

           14kHz |    24kbit    |     32kbit     |     48kbit
          -------|--------------|----------------|----------------
            AVG  |    0.01      |     0.01       |     0.01   
          -------|--------------|----------------|----------------
            MAX  |    0.01      |     0.01       |     0.01   
          -------|--------------|----------------|----------------

***************************************************************************/
void compute_raw_pow_categories(Word16 *power_categories,Word16 *rms_index,Word16 number_of_regions,Word16 offset)
{
    //register Word16 region;
	register Word32 region;														//kpit change 30/9
    register Word16 j;															//kpit change 27/9
    //Word16 temp;			  												    //kpit chnage 30/9

    for (region=0; region<number_of_regions; region++) 
    {
        j = (offset - rms_index[region])>>1;	 							   //kpit change 28/9
        //j = j>>1;								 							  //kpit change 28/9
        
        /* make sure j is between 0 and NUM_CAT-1 */
        test();
        if (j < 0) 
        {
            j = 0;
            move16();
        }
       // temp = (j - (NUM_CATEGORIES-1));
        test();
       // if (temp > 0) 
	    if ((j - (NUM_CATEGORIES-1)) > 0) 								    //kpit change 30/9
            j = (NUM_CATEGORIES - 1);
        
        power_categories[region] = j;
        move16();
    }
}

