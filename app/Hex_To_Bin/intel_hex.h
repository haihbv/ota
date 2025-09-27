/**
 * @file intel_hex.h
 * @brief API phân tích và chuyển đổi Intel HEX (header).
 *
 * Header này cung cấp một API nhỏ để kiểm tra tính hợp lệ của file Intel
 * HEX và chuyển đổi chúng sang dạng nhị phân thô. Phần cài đặt sử dụng
 * bộ phân tích theo dòng đơn giản và một bộ đệm nội bộ nhỏ.
 */

#ifndef INTEL_HEX_H
#define INTEL_HEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * @enum Intel_StructureType
 * @brief Các trạng thái của máy trạng thái (state machine) khi xử lý một dòng Intel HEX.
 *
 * Bộ phân tích (parser) sẽ chuyển qua các trạng thái này khi giải mã từng
 * dòng bản ghi ASCII HEX.
 */
typedef enum
{
    START_CODE = 0, /**< Waiting for ':' start code */
    BYTE_COUNT,     /**< Next field: byte count (1 byte) */
    ADDRESS,        /**< Next field: address (2 bytes) */
    RECORD_TYPE,    /**< Next field: record type (1 byte) */
    DATA,           /**< Next field: data bytes (N bytes) */
    CHECKSUM,       /**< Next field: checksum (1 byte) */
    DONE            /**< Finished processing this record */
} Intel_StructureType;

/**
 * @enum Intel_StatusTypeDef
 * @brief Mã trạng thái trả về cho các thao tác liên quan đến Intel HEX.
 */
typedef enum
{
    SUCCESS = 0, /**< Operation succeeded */
    FAIL         /**< Operation failed (parse error, checksum, I/O, etc.) */
} Intel_StatusTypeDef;

/**
 * @struct Intel_HexTypeDef
 * @brief Biểu diễn trong bộ nhớ của một bản ghi Intel HEX đã được phân tích.
 *
 * Các trường tương ứng trực tiếp với định dạng chuẩn của bản ghi Intel HEX.
 * Mảng `data` hiện có thể chứa tối đa 16 byte dữ liệu (tùy chỉnh nếu file
 * HEX của bạn có bản ghi dài hơn).
 */
typedef struct
{
    uint8_t byte_count;   /**< Number of data bytes in this record */
    uint8_t address[2];   /**< 16-bit address (big-endian as two bytes) */
    uint8_t record_type;  /**< Record type (0x00=data, 0x01=EOF, etc.) */
    uint8_t data[16];     /**< Data bytes (up to 16 by current implementation) */
    uint8_t checksum;     /**< Record checksum */
} Intel_HexTypeDef;

/**
 * @brief Phân tích và xác thực tất cả các dòng từ một file Intel HEX đang mở.
 *
 * Hàm này đọc từng dòng từ con trỏ FILE được cung cấp và xác thực các bản ghi
 * cùng checksum cho tới khi gặp bản ghi End Of File (EOF).
 *
 * @param fp Con trỏ FILE* đã mở để đọc (phải được đặt ở đầu file HEX).
 * @return SUCCESS nếu phân tích hoàn tất và tìm thấy bản ghi EOF với checksum hợp lệ,
 *         ngược lại trả về FAIL.
 */
Intel_StatusTypeDef IntelHex_Read_AllLine(FILE *fp);

/**
 * @brief Chuyển một chuỗi con ASCII hex thành các byte nhị phân lưu trong bộ đệm nội bộ.
 *
 * Hàm trợ giúp này chuyển `length` ký tự ASCII hex (nên là số chẵn) bắt đầu tại
 * `pBuff` thành các byte và lưu vào bộ đệm nội bộ của module.
 *
 * @param pBuff Con trỏ tới chuỗi ký tự ASCII hex (ví dụ: "0A1B...").
 * @param length Số ký tự ASCII cần chuyển (mong đợi là số chẵn).
 * @note Hàm này ghi vào một bộ đệm tĩnh ở cấp module được sử dụng bởi bộ phân tích.
 */
void IntelHex_Convert_Str_To_Hex(char *pBuff, int length);

/**
 * @brief Chuyển file Intel HEX sang file nhị phân thô.
 *
 * Đọc `pHexFile` và ghi các byte nhị phân thô vào `pBinFile`.
 * Cài đặt hiện tại ghi nối tiếp các dữ liệu từ các bản ghi và không sinh
 * các khoảng đệm (gaps) cho các địa chỉ không liên tiếp. Nếu bạn cần
 * xuất theo địa chỉ (sparse binary), hãy sửa phần cài đặt để seek và
 * điền (pad) file đầu ra tương ứng.
 *
 * @param pHexFile Đường dẫn tới file Intel HEX đầu vào.
 * @param pBinFile Đường dẫn tới file nhị phân đầu ra sẽ được tạo/ghi đè.
 */
void IntelHex_Convert_Hex_To_Bin(const char *pHexFile, const char *pBinFile);

#ifdef __cplusplus
}
#endif


#endif // INTEL_HEX_H
