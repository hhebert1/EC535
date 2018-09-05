//
//  main.c
//  hhebert1_HW1
//
//  Created by Hannah on 1/25/17.
//  Copyright © 2017 Hannah. All rights reserved.
//

#include <stdio.h>
#include "bits.h"

int main(int argc, const char * argv[])
{
    
    unsigned int n;//number to be operated on
    
    unsigned int mirror;
    unsigned int count;
    
    //open file for reading
    FILE *inFile;
    inFile = fopen(argv[1], "r");
    
    
    if(inFile==NULL)
        printf("Error: Unable to open file");
    else
    {
        //create output file;
        FILE *outFile;
        outFile = fopen(argv[2], "w+");
        
        //for each line in input file, calculate mirror and count
        //and write those values to output file
        while(fscanf(inFile, "%u", &n) != EOF)
        {
            mirror = BinaryMirror(n);
            count = SequenceCount(n);
            fprintf(outFile, "%u %u\n", mirror, count);
        }
        
        //close both files
        fclose(inFile);
        fclose(outFile);
    }
    
    return 0;
}
