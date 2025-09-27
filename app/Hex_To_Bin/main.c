#include <stdio.h>
#include "intel_hex.h"

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("Usage: %s <input.hex> <output.bin>\n", argv[0]);
		return 1;
	}
	
	const char *inputFile = argv[1];
	const char *outputFile = argv[2];
	
    FILE *fp = fopen(inputFile, "r");
    if (!fp)
    {
        perror("Error opening input file");
        return 1;
    }

    Intel_StatusTypeDef status = IntelHex_Read_AllLine(fp);
    fclose(fp);

    if (status != SUCCESS)
    {
        fprintf(stderr, "Error: Intel HEX file '%s' is invalid. Cannot convert.\n", inputFile);
        return 1;
    }

    printf("Intel HEX file is valid. Converting to binary...\n");
    IntelHex_Convert_Hex_To_Bin(inputFile, outputFile);
    return 0;
}
