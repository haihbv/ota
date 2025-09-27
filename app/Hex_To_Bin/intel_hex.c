#include "intel_hex.h"
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 256

static char cBufferData[MAX_LINE_LENGTH];
static uint8_t cInternalBufferData[50];
static Intel_HexTypeDef IntelHexData;

static uint32_t Read_Line(FILE *fp, char *pBufferData)
{
    uint32_t count = 0;
    int data_line;

    while ((data_line = fgetc(fp)) != EOF && (data_line != '\n') && (count < MAX_LINE_LENGTH - 1))
    {
        *pBufferData++ = (char)data_line;
        count++;
    }

    *pBufferData = '\0';
    return count;
}

static uint8_t Hex_Char_To_Value(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return 0;
}

static uint8_t Hex_String_To_Byte(const char *pStr)
{
    return (Hex_Char_To_Value(pStr[0]) << 4) | Hex_Char_To_Value(pStr[1]);
}

static uint16_t Hex_String_To_Word(const char *pStr)
{
    return (Hex_String_To_Byte(pStr) << 8) | Hex_String_To_Byte(pStr + 2);
}

void IntelHex_Convert_Str_To_Hex(char *pBuff, int length)
{
    int j = 0;
    for (int i = 0; i < length; i += 2)
    {
        if (j < 50) // Đảm bảo không vượt quá kích thước buffer
        {
            cInternalBufferData[j++] = Hex_String_To_Byte(&pBuff[i]);
        }
    }
}

Intel_StatusTypeDef IntelHex_Read_AllLine(FILE *fp)
{
    uint8_t check_sum;
    Intel_StructureType Intel_StatusInit = START_CODE;

    uint32_t count, data_count;
    uint32_t line_count = 0;

    while (Read_Line(fp, cBufferData) > 0)
    {
        // Reset trạng thái cho mỗi dòng
        Intel_StatusInit = START_CODE;
        memset(IntelHexData.data, 0, sizeof(IntelHexData.data));
        memset(cInternalBufferData, 0, sizeof(cInternalBufferData));

        line_count++;

        for (count = 0; count < strlen(cBufferData); count++)
        {
            switch (Intel_StatusInit)
            {
            case START_CODE:
                if (cBufferData[count] == ':')
                {
                    Intel_StatusInit = BYTE_COUNT;
                }
                else
                {
                    return FAIL;
                }
                break;

            case BYTE_COUNT:
                IntelHex_Convert_Str_To_Hex(&cBufferData[count], 2);
                IntelHexData.byte_count = cInternalBufferData[0];
                count += 1; // Đã xử lý 2 ký tự
                Intel_StatusInit = ADDRESS;
                break;

            case ADDRESS:
                IntelHex_Convert_Str_To_Hex(&cBufferData[count], 4);
                IntelHexData.address[0] = cInternalBufferData[0];
                IntelHexData.address[1] = cInternalBufferData[1];
                count += 3; // Đã xử lý 4 ký tự
                Intel_StatusInit = RECORD_TYPE;
                break;

            case RECORD_TYPE:
                IntelHex_Convert_Str_To_Hex(&cBufferData[count], 2);
                IntelHexData.record_type = cInternalBufferData[0];
                count += 1; // Đã xử lý 2 ký tự
                Intel_StatusInit = DATA;
                break;

            case DATA:
                IntelHex_Convert_Str_To_Hex(&cBufferData[count], IntelHexData.byte_count * 2);
                for (data_count = 0; data_count < IntelHexData.byte_count; data_count++)
                {
                    IntelHexData.data[data_count] = cInternalBufferData[data_count];
                }
                count += (IntelHexData.byte_count * 2) - 1; // Di chuyển count
                Intel_StatusInit = CHECKSUM;
                break;

            case CHECKSUM:
                IntelHex_Convert_Str_To_Hex(&cBufferData[count], 2);
                IntelHexData.checksum = cInternalBufferData[0];
                Intel_StatusInit = DONE;
                break;

            case DONE:
                check_sum = IntelHexData.byte_count +
                            IntelHexData.address[0] +
                            IntelHexData.address[1] +
                            IntelHexData.record_type;

                for (data_count = 0; data_count < IntelHexData.byte_count; data_count++)
                {
                    check_sum += IntelHexData.data[data_count];
                }

                check_sum = (~check_sum) + 1;

                if (check_sum == IntelHexData.checksum)
                {
                    if (IntelHexData.record_type == 0x01)
                    {
                        return SUCCESS;
                    }
                }
                else
                {
                    return FAIL;
                }
                break;

            default:
                return FAIL;
            }
        }
    }
    return SUCCESS;
}

void IntelHex_Convert_Hex_To_Bin(const char *pHexFile, const char *pBinFile)
{
    FILE *hex_fp = fopen(pHexFile, "r");
    FILE *bin_fp = fopen(pBinFile, "wb");
    if (!hex_fp || !bin_fp)
    {
        perror("Error opening files");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), hex_fp))
    {
        if (line[0] != ':')
        {
            continue;
        }

        uint8_t byte_count = Hex_String_To_Byte(&line[1]);
        uint16_t address = Hex_String_To_Word(&line[3]);
        uint8_t record_type = Hex_String_To_Byte(&line[7]);

        if (record_type == 0x00)
        {
            for (int i = 0; i < byte_count; i++)
            {
                uint8_t data_byte = Hex_String_To_Byte(&line[9 + i * 2]);
                fwrite(&data_byte, 1, 1, bin_fp);
            }
        }
        else if (record_type == 0x01)
        {
            break;
        }
    }

    fclose(hex_fp);
    fclose(bin_fp);
    printf("Conversion complete: %s -> %s\n", pHexFile, pBinFile);
}