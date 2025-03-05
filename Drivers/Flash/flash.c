
#include "stm32f4xx.h"
#include "flash.h"
#define FLASH_TIMEOUT_INT 50000  // Giới hạn vòng lặp chờ (tránh treo)
#define FLASH_BASE_ADDRESS FLASH_SECTOR1_BASE_ADDRESS  // Địa chỉ lưu firmware



void FLASH_Unlock() {
    FLASH->KEYR = 0x45670123;  // Mở khóa bước 1
    FLASH->KEYR = 0xCDEF89AB;  // Mở khóa bước 2
}

void FLASH_Lock() {
    FLASH->CR |= FLASH_CR_LOCK;  // Khóa lại Flash
}

static FLASH_StatusTypeDef FLASH_WaitForOperation(uint32_t timeout) {
    while (FLASH->SR & FLASH_SR_BSY) {
        if (timeout-- == 0) return FLASH_TIMEOUT;
        HAL_Delay(1);
    }
    return FLASH_OK;
}

// 🔹 Ghi 1 BYTE (8-bit)
FLASH_StatusTypeDef FLASH_Write_Byte(uint32_t address, uint8_t data) {
    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_PSIZE_BYTE;  // Chế độ ghi 1 byte
    FLASH->CR |= FLASH_CR_PG;  // Cho phép ghi

    *(volatile uint8_t*)address = data;

    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_PG;  // Tắt chế độ ghi
    return FLASH_OK;
}

// 🔹 Ghi 2 BYTE (16-bit)
FLASH_StatusTypeDef FLASH_Write_HalfWord(uint32_t address, uint16_t data) {
    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_PSIZE_HALF_WORD;  // Chế độ ghi 2 byte
    FLASH->CR |= FLASH_CR_PG;

    *(volatile uint16_t*)address = data;

    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_PG;
    return FLASH_OK;
}

// 🔹 Ghi 4 BYTE (32-bit)
FLASH_StatusTypeDef FLASH_Write_Word(uint32_t address, uint32_t data) {
    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_PSIZE;
    FLASH->CR |= FLASH_PSIZE_WORD;  // Chế độ ghi 4 byte
    FLASH->CR |= FLASH_CR_PG;

    *(volatile uint32_t*)address = data;

    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_PG;
    return FLASH_OK;
}

FLASH_StatusTypeDef FLASH_Erase_Sectors(uint8_t sector) {

    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_SNB;  // Xóa lựa chọn sector cũ
    FLASH->CR |= FLASH_CR_SER | (sector << FLASH_CR_SNB_Pos);  // Chọn sector cần xóa
    FLASH->CR |= FLASH_CR_STRT;  // Bắt đầu xóa

    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_SER;  // Tắt chế độ xóa sector
    return FLASH_OK;
}

// 🔹 Xóa toàn bộ Flash (tùy chọn không xóa bootloader)
FLASH_StatusTypeDef FLASH_Erase_All(void) {
    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR |= FLASH_CR_MER;  // Chế độ xóa toàn bộ
    FLASH->CR |= FLASH_CR_STRT;  // Bắt đầu xóa

    if (FLASH_WaitForOperation(FLASH_TIMEOUT_INT) != FLASH_OK) return FLASH_TIMEOUT;

    FLASH->CR &= ~FLASH_CR_MER;  // Tắt chế độ xóa toàn bộ
    return FLASH_OK;
}
