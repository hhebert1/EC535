//
//  bits.c
//  hhebert1_HW1
//
//  Created by Hannah on 1/25/17.
//  Copyright © 2017 Hannah. All rights reserved.
//

#include "bits.h"
#include <math.h>

//function to compute binary mirror of number
unsigned int BinaryMirror(unsigned int n)
{
    unsigned int mirror;
    char n_bin[8*sizeof(unsigned int)] = {0}; //create an array to story binary vals
    
    int mod;
    int i = 8*sizeof(unsigned int) - 1;
    
    while(n != 0) //fill array with mirror of binary data
    {
        mod = n%2;
        n = n/2;
        
        n_bin[i] = n_bin[i] + mod;
        i--;
    }
    
    //convert binary mirror to unsigned int
    mirror = 0;
    int j;
    for(j=0; j < 8*sizeof(unsigned int); j++)
    {
        if(n_bin[j] == 1)
            mirror = mirror + pow(2,j);
    }
    
    return mirror;
}

//funciton to count the number of "10" sequences
unsigned int SequenceCount(unsigned int n)
{
    unsigned int count = 0;
    
    char n_bin[8*sizeof(unsigned int)] = {0}; //create an array to story binary vals
    
    int mod;
    int i = 0;
    
    while(n != 0) //fill array with binary data
    {
        mod = n%2;
        n = n/2;
    
        n_bin[i] = n_bin[i] + mod;
        i++;
    }
    
    //check for "10" pattern in character array storing binary data
    int j;
    for(j=8*sizeof(unsigned int)-1; j > 0 ; j--)
    {
        if((n_bin[j] == 1) && (n_bin[j-1] == 0))
            count++;
    }
    
    return count;
}
