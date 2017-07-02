// Created by: Ryan Ubinger
// NID: ry629688

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint_fast16_t crc_t;

crc_t XOR(crc_t x, crc_t y)
{
	return x^y;
}

static inline crc_t presetCRC(void)
{
    return 0x0000;
}

static inline crc_t augmentXOR(crc_t crc)
{
    return crc ^ 0x0000;
}

crc_t finalCRC(crc_t crc, const void *data, size_t data_len)
{
	crc_t POLY = 0xA053;
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    int a, b = 0;
    bool bit;
    unsigned char c;
	
	// alters iteration length to allow verify to work
	if ((int) data_len == 512)
		data_len = (int)data_len - 8;
	
	printf("\n\nCRC-15 Calculation Progression:\n\n");
    for (a = 0; a < data_len; a++) 
    {
        c = *d++;
        for (i = 0x80; i > 0; i >>= 1)
        {
            bit = crc & 0x4000;
            if (c & i)
                bit = !bit;

            crc <<= 1;
            if (bit)
                crc = XOR(crc, POLY);
                
        }

        crc &= 0x7fff;
    }
    
    return crc & 0x7fff;
}

crc_t *lineByLineCRC(crc_t crc, const void *data, size_t data_len, crc_t *crcIteration)
{
	crc_t POLY = 0xA053;
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    int a, b = 0;
    bool bit;
    unsigned char c;
	
	// alters iteration length to allow verify function to work
	if ((int) data_len == 512)
		data_len = (int)data_len - 8;
	
    for (a = 0; a < data_len; a++) 
    {
        c = *d++;
        for (i = 0x80; i > 0; i >>= 1)
        {
            bit = crc & 0x4000;
            if (c & i)
                bit = !bit;

            crc <<= 1;
            if (bit)
                crc = XOR(crc, POLY); 
                
        }
        crc &= 0x7fff;
        
        if ((a + 1) % 64 == 0)
        	crcIteration[b++] = augmentXOR(crc);
    }
    
    return crcIteration;
}

void printMessage(char *data, crc_t *crcIteration, char *prefix, crc_t finalCRC)
{
	int i, j = 0, len = strlen(data);
	char c;
	
	for (i = 0; i < len; i++) 
	{
		printf("%c", data[i]); 
			
		if ((i + 1) % 64 == 0 && i != 511) 
		{
			printf("\t-  ");
			printf("0000%04x \n", crcIteration[j]);
			j++;
		}
	}
	
	// only prints if coming from calculateCRC
	if (strcmp(prefix, "0000") == 0)
		printf("%s%04x", prefix, finalCRC);
	
	printf("\t-  0000%04x \n", finalCRC);
	printf("\n");
	printf("\nCRC-15 result: 0000%04x\n", finalCRC);
}

// function to calculate CRC-15
void calculateCRC(char *data, int index)
{
	crc_t crc, crc1;
	crc_t *crcIteration = malloc(sizeof(crc_t) * 8);
	int i, j, len = strlen(data);
	
	// display input text
	for (i = 0; i < len; i++)
	{
		if (i % 64 == 0)
			printf("\n");
			
		printf("%c", data[i]);
	}
	
	for (j = index; j < 504; j++)
		data[j] = '.';
		
	crc = presetCRC();
	crcIteration = lineByLineCRC(crc, (void *)data, strlen(data), crcIteration);
	
	crc1 = presetCRC();
	crc1 = finalCRC(crc1, (void *)data, strlen(data));
	crc1 = augmentXOR(crc1);
	
	printMessage(data, crcIteration, "0000", crc1);
	
	free(crcIteration);
}

// function to verify CRC-15
void verifyCRC(char *data)
{
	crc_t crc, crc1;
	crc_t *crcIteration = malloc(sizeof(crc_t) * 8);
	char receivedCRC[4];
	char *r;
	unsigned int received;
	int i, j;
	
	// display input text
	for (i = 0; i < 512; i++)
	{
		if (i % 64 == 0)
			printf("\n");
			
		printf("%c", data[i]);
	}
	
	for (j = 508; j < 512; j++)
		receivedCRC[j-508] = data[j];

	r = receivedCRC;
	
	received = strtol(r, NULL, 16);
	
	crc = presetCRC();
	crcIteration = lineByLineCRC(crc, (void *)data, strlen(data), crcIteration);
	
	crc1 = presetCRC();
	crc1 = finalCRC(crc1, (void *)data, strlen(data));
	crc1 = augmentXOR(crc1);
	
	printMessage(data, crcIteration, "", crc1);
	
	if (crc1 == received)
	{
		printf("\nCRC-15 verification is confirmed.");
		printf("\nThe file's integrity is intact.\n");
	}
	else
	{
		printf("\nCRC-15 verification failed.");
		printf("\nThe file's integrity is compromised.\n");
	}
	
	free(crcIteration);
}	

int main(int argc, char **argv)
{
	int a = 0;
	char *data = malloc(sizeof(char) * 512);
	char *opCode = argv[1];
	char *opCalc = "c";
	char *opVerify = "v";
	char *fname = argv[2];
	char ch;
	
	FILE *ifp1 = NULL;
	
	ifp1 = fopen(fname, "r");
	
	if (ifp1 == NULL)
		printf("Error: File could not be opened");
	
	printf("CRC-15 Input Text File:\n");
	while ((ch = fgetc(ifp1)) != EOF)
	{
		if (ch == '\n' || ch == '\r')
			continue;
	
		data[a] = ch;
		a++;
	}
	
	fclose(ifp1);
	
	if (strcmp(opCode, opCalc) == 0)
		calculateCRC(data, a);
	else if (strcmp(opCode, opVerify) == 0)
		verifyCRC(data);
	else
	{
		printf("\n\n");
		printf("Please follow valid argument conventions below:\n");
		printf("===============================================\n");
		printf("To calculate CRC -->  ./crcheck c [filename] \n");
		printf("To verify CRC    -->  ./crcheck v [filename] \n");
		printf("===============================================\n");
	}
	
	free(data);
}