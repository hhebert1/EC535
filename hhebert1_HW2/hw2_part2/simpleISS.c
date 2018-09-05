#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct instruction
{
	int lineNum;
	char instr[10];
	char op1[10];
	char op2[10];
};

int main(int argv, char *argc[])
{
	struct instruction line[100];
	int instrCount = 0;
	char lineRead[100];

	FILE *inFile;
	inFile = fopen(argc[1], "r");

	//Read in file into structure to store all instructions (line)
	if(inFile == NULL)
	{
		printf("Error: Unable to open file");
	}
	else
	{
		while(fgets(lineRead,100, inFile) != NULL)
		{
			sscanf(lineRead,"%d %s %[^','], %s",&line[instrCount].lineNum, line[instrCount].instr, line[instrCount].op1, line[instrCount].op2);
			instrCount++;
		}
	}

	//begin decoding instructions
	int8_t REG[6];
	int8_t MEM[256];

	int exCount=0; //number of executed instructions
	int cycleCount=0; //number of clock cycles
	int memCount=0; //Number of hits to local memory
	int lsCount=0; //number of executed LD/ST instructions

	int instrIndex = instrCount;

	int srcR;
	int destR;

	int JEflag;
	int jumpVal;

	int i;
	int j;
	int k;

	int check=0;
	for(i=0; i< instrIndex; i++)
	{
		if(strcmp(line[i].instr, "MOV") == 0)
		{
			srcR = atoi(line[i].op2);
			destR = atoi(&line[i].op1[1]);
			REG[destR-1] = srcR;

			exCount++;
			cycleCount++;
			printf("%d -", line[i].lineNum);
			printf("MOV\n");
		}
		else if(strcmp(line[i].instr, "ADD") == 0)
		{
			//add reg
			if(line[i].op2[0]== 'R')
			{
				srcR = atoi(&line[i].op2[1]);
				destR = atoi(&line[i].op1[1]);

				REG[destR -1] = REG[destR-1] + REG[srcR-1];
				printf("%d -", line[i].lineNum);
				printf("ADD reg\n");
			}

			//add immediate
			else
			{
				srcR = atoi(line[i].op2);
				destR = atoi(&line[i].op1[1]);

				REG[destR -1] = REG[destR-1] + srcR;
				printf("%d -", line[i].lineNum);
				printf("ADD imm\n");
			}

			exCount++;
			cycleCount++;
		}
		else if(strcmp(line[i].instr, "CMP") == 0)
		{

			srcR = atoi(&line[i].op2[1]);
			destR = atoi(&line[i].op1[1]);
			if(REG[srcR-1] == REG[destR-1])
			{
				JEflag = 1;
				printf("set jump flag: ");
			}
			else
			{
				JEflag = 0;
				printf("did not set jump flag: ");
			}

			exCount++;
			cycleCount = cycleCount + 2;
			printf("%d -", line[i].lineNum);
			printf("CMP\n");
		}
		else if(strcmp(line[i].instr, "JE") == 0)
		{
			jumpVal = atoi(line[i].op1);
			if(JEflag == 1)
			{
				for(j=0; j<instrIndex; j++)
				{
					if(jumpVal == line[j].lineNum)
					{
						i=j-1;
						break;
					}
				}
			}
			exCount++;
			cycleCount++;
			printf("%d -", line[i].lineNum);
			printf("JE\n");
			continue;
		
		}
		else if(strcmp(line[i].instr, "JMP") == 0)
		{
			jumpVal = atoi(line[i].op1);
			//i = jumpVal;

			for(j=0; j<instrIndex; j++)
			{
				if(jumpVal == line[j].lineNum)
				{
					i=j-1;
					break;
				}
			}
	
			exCount++;
			cycleCount++;
			printf("%d -", line[i].lineNum);
			printf("JMP\n");
			continue;
		}
		else if(strcmp(line[i].instr, "LD") == 0)
		{
			srcR= atoi(&line[i].op2[1]);
			destR=atoi(&line[i].op1[1]);

				if(MEM[REG[srcR-1]] == REG[destR-1]) //if found in local mem
				{
				cycleCount = cycleCount + 2;
				memCount++;
				printf("%d -", line[i].lineNum);
				printf("LD-- access local mem\n");
				}
				else //if not found in local mem
				{
					REG[destR-1] = MEM[REG[srcR-1]];
									cycleCount = cycleCount + 40;
				printf("%d -", line[i].lineNum);
				printf("LD--main mem\n");
				}

			exCount++;
			lsCount++;
		}
		else if(strcmp(line[i].instr, "ST") == 0)
		{
			srcR= atoi(&line[i].op2[1]); 
			destR=atoi(&line[i].op1[1]); 

				if(MEM[REG[destR-1]] == REG[srcR-1]) //if found in local mem
				{
					MEM[REG[destR-1]] = REG[srcR-1];
					memCount++;
									cycleCount = cycleCount + 2;
				printf("%d -", line[i].lineNum);
				printf("ST-- access local mem\n");
				}
				else //if not found in local mem
				{
					MEM[REG[destR-1]] = REG[srcR-1];
									cycleCount = cycleCount + 40;
				printf("%d -", line[i].lineNum);
				printf("ST--access main mem\n");
				}

			exCount++;
			lsCount++;
		}  
	}

	//final printing
	printf("Total number of instructions in the code: %d\n", instrCount);
	printf("Total number of executed instructions: %d\n", exCount);
	printf("Total number of clock cycles: %d\n", cycleCount);
	printf("Number of hits to local memory: %d\n", memCount);
	printf("Total number of executed LD/ST instructions: %d\n", lsCount);
}