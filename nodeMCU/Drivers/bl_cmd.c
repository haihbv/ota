/**
 * @file    bl_cmd.c
 * @brief   Bộ xử lý lệnh bootloader giao tiếp với PC qua UART.
 *
 * @details Giao thức khung dữ liệu:
 *          [CMD][LEN][PAYLOAD][CHECKSUM]
 *          - CMD (1 byte)     : Mã lệnh (ví dụ: CMD_ERASE/CMD_WRITE/CMD_VERIFY/CMD_JUMP).
 *          - LEN (1 byte)     : Độ dài phần PAYLOAD tính theo byte.
 *          - PAYLOAD (LEN B)  : Dữ liệu kèm theo tuỳ theo từng lệnh.
 *          - CHECKSUM (1 byte): Tổng 8-bit (mod 256) của các byte từ CMD đến hết PAYLOAD.
 *
 *          Ý nghĩa lệnh:
 *          - CMD_ERASE : Xoá một vùng flash bắt đầu tại APP_ADDRESS với kích thước chỉ định.
 *          - CMD_WRITE : Ghi dữ liệu vào flash tại địa chỉ chỉ định.
 *          - CMD_VERIFY: Tính checksum image ứng dụng (bắt đầu tại APP_ADDRESS, kích thước size_verify) và so với PC.
 *          - CMD_JUMP  : Nhảy thực thi Application (Reset_Handler) sau khi gửi ACK.
 *
 * @note    - Tất cả multi-byte trong PAYLOAD (uint16_t/uint32_t) được copy trực tiếp bằng memcpy.
 *            Nếu PC gửi theo Little-Endian (thường gặp), hai bên sẽ khớp tự nhiên trên STM32 (LE).
 *          - Cần đảm bảo các hàm Flash_* và Jump_To_Application() đã được hiện thực an toàn.
 *          - UART1.SendChar() là primitive truyền byte đơn qua UART.
 */

#include "bl_cmd.h"
#include <string.h>

/**
 * @brief  Tính checksum 8-bit (mod 256) cho dải dữ liệu.
 *
 * @param  data Con trỏ tới vùng dữ liệu đầu vào.
 * @param  len  Độ dài dữ liệu (byte).
 * @retval uint8_t Checksum 1 byte (tổng dồn & 0xFF).
 *
 * @details Thuật toán:
 *          sum = Σ data[i] với i ∈ [0, len-1], sau đó lấy 8-bit thấp.
 *          Đây là checksum đơn giản để phát hiện lỗi truyền cơ bản.
 *
 * @warning Không phát hiện được mọi lỗi đảo bit phức tạp như CRC. Dùng cho giao thức nhẹ.
 */
static uint8_t calc_checksum(uint8_t *data, uint16_t len)
{
    uint16_t sum = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

/**
 * @brief  Gửi ACK (0x79) về PC để xác nhận thao tác thành công.
 *
 * @retval None
 *
 * @note   ACK/NACK chỉ xác nhận ở tầng giao thức. Thành công thực tế phụ thuộc vào
 *         Flash_* và kiểm tra hợp lệ địa chỉ/size ở nơi khác.
 */
void BL_SendACK(void)
{
    UART1.SendChar(CMD_ACK);
}

/**
 * @brief  Gửi NACK (0x1F) về PC để báo gói sai hoặc thao tác thất bại.
 *
 * @retval None
 */
void BL_SendNACK(void)
{
    UART1.SendChar(CMD_NACK);
}

/**
 * @brief  Phân tích và xử lý gói lệnh từ PC theo giao thức bootloader.
 *
 * @param  rx_buf  Con trỏ đến buffer chứa toàn bộ gói (bao gồm CMD..CHECKSUM).
 * @param  len     Tổng độ dài buffer @p rx_buf (tính theo byte).
 * @retval None
 *
 * @details Định dạng gói:
 *          [CMD][LEN][PAYLOAD][CHECKSUM]
 *          - CMD: mã lệnh
 *          - LEN: số byte của PAYLOAD
 *          - CHECKSUM = calc_checksum(&CMD, 2 + LEN) (tức CMD..PAYLOAD)
 *
 *          Kiểm tra cơ bản:
 *          - Gói tối thiểu phải có 3 byte (CMD + LEN + CHECKSUM).
 *          - Chỉ chấp nhận gói khi checksum đúng.
 *          - Với mỗi lệnh, kiểm tra độ dài PAYLOAD như mô tả dưới đây.
 *
 * @par    Lệnh CMD_ERASE
 *         - PAYLOAD: 4 byte @c size (uint32_t), kích thước cần xoá tính từ @ref APP_ADDRESS.
 *         - Hành động: gọi Flash_Erase(APP_ADDRESS, size). Gửi ACK.
 *         - Lỗi: nếu payload_len != 4 → NACK.
 *
 * @par    Lệnh CMD_WRITE
 *         - PAYLOAD: 4 byte @c addr (uint32_t) + N byte @c data.
 *         - Hành động: gọi Flash_Write(addr, data, data_len).
 *         - Thành công: nếu Flash_Write trả về FLASH_OP_OK → ACK, ngược lại NACK.
 *         - Lỗi: nếu payload_len < 4 → NACK.
 *         - @warning Cần đảm bảo @c addr thuộc vùng flash hợp lệ và alignment theo phần cứng.
 *
 * @par    Lệnh CMD_VERIFY
 *         - PAYLOAD: 2 byte @c checksum_pc (uint16_t) + 4 byte @c size_verify (uint32_t).
 *         - Hành động:
 *              + Tính tổng dồn byte image tại [APP_ADDRESS .. APP_ADDRESS + size_verify - 1].
 *              + Mask 8-bit: checksum_stm &= 0xFF.
 *              + So sánh @c checksum_stm với @c checksum_pc.
 *           - Kết quả: bằng nhau → ACK; khác → NACK.
 *         - Lỗi: nếu payload_len != 6 → NACK.
 *         - @note  Hiện quy về 8-bit, dù @c checksum_pc là 16-bit. Cần thống nhất phía PC.
 *
 * @par    Lệnh CMD_JUMP
 *         - PAYLOAD: rỗng (LEN = 0).
 *         - Hành động: gửi ACK, DelayMs(10) để xả UART, gọi Jump_To_Application().
 *
 * @note   Endianness: vì memcpy trực tiếp, giả định PC đóng gói Little-Endian (phổ biến).
 *         Nếu PC là Big-Endian, cần chuyển đổi byte-order trước khi memcpy.
 *
 * @warning Độ dài biên: Hàm có kiểm tra tối thiểu và checksum; tuy nhiên bạn nên
 *         bảo đảm @c len >= 3 + payload_len trước khi truy cập rx_buf[2 + payload_len].
 *         Ở đây logic đã gián tiếp đảm bảo, nhưng nếu tích hợp parser mới, hãy kiểm tra chặt chẽ.
 */
void BL_ProcessCommand(uint8_t *rx_buf, uint16_t len)
{
    /* Tối thiểu phải có CMD + LEN + CHECKSUM */
    if (len < 3)
    {
        BL_SendNACK();
        return;
    }

    uint8_t cmd = rx_buf[0];
    uint8_t payload_len = rx_buf[1];
    uint8_t *payload = &rx_buf[2];
    if ((uint16_t)(2 + payload_len) >= len)
    {
        BL_SendNACK();
        return;
    }

    /* Xác thực checksum trên vùng [CMD..PAYLOAD] */
    uint8_t checksum = rx_buf[2 + payload_len];
    uint8_t calc = calc_checksum(rx_buf, (uint16_t)(2 + payload_len));
    if (calc != checksum)
    {
        BL_SendNACK();
        return;
    }

    switch (cmd)
    {
    case CMD_ERASE:
    {
        /* PAYLOAD = size (4 byte) */
        if (payload_len != 4)
        {
            BL_SendNACK();
            break;
        }
        uint32_t size = 0;
        memcpy(&size, payload, sizeof(uint32_t));

        if (Flash_Erase(APP_ADDRESS, size) == FLASH_OP_OK)
        {
            BL_SendACK();
        }
        else
        {
            BL_SendNACK();
        }
        break;
    }

    case CMD_WRITE:
    {
        /* PAYLOAD = addr (4 byte) + data (N byte) */
        if (payload_len < 4)
        {
            BL_SendNACK();
            break;
        }
        uint32_t addr = 0;
        memcpy(&addr, payload, sizeof(uint32_t));

        uint8_t *data = &payload[4];
        uint16_t data_len = (uint16_t)(payload_len - 4);

        if (Flash_Write(addr, data, data_len) == FLASH_OP_OK)
        {
            BL_SendACK();
        }
        else
        {
            BL_SendNACK();
        }
        break;
    }

    case CMD_VERIFY:
    {
        /* PAYLOAD = checksum_pc (2 byte) + size_verify (4 byte) */
        if (payload_len != 6)
        {
            BL_SendNACK();
            break;
        }

        uint16_t checksum_pc = 0;
        uint32_t size_verify = 0;

        memcpy(&checksum_pc, &payload[0], sizeof(uint16_t));
        memcpy(&size_verify, &payload[2], sizeof(uint32_t));

        /* Tính tổng dồn image ứng dụng (mask 8-bit) */
        uint16_t checksum_stm = 0;
        for (uint32_t i = 0; i < size_verify; i++)
        {
            checksum_stm += *(uint8_t *)(APP_ADDRESS + i);
        }
        checksum_stm &= 0xFF;

        if (checksum_stm == checksum_pc)
        {
            BL_SendACK();
        }
        else
        {
            BL_SendNACK();
        }
        break;
    }

    case CMD_JUMP:
    {
        /* PAYLOAD rỗng */
        if (payload_len != 0)
        {
            BL_SendNACK();
            break;
        }
        BL_SendACK();
        DelayMs(10); /* Đợi một chút trước khi nhảy để đảm bảo byte UART đã ra khỏi FIFO */
        Jump_To_Application();
        break;
    }

    default:
        /* Lệnh không hỗ trợ */
        BL_SendNACK();
        break;
    }
}
