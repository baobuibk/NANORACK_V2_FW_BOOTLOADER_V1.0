/*****************************************************************************************************************************
 **********************************    Author  : Ehab Magdy Abdullah                      *************************************
 **********************************    Linkedin: https://www.linkedin.com/in/ehabmagdyy/  *************************************
 **********************************    Youtube : https://www.youtube.com/@EhabMagdyy      *************************************
 ******************************************************************************************************************************/

/************************************             Includes               ************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "bootloader.h"
#include <stm32f4xx_ll_bus.h>
#include "UART.h"
#include "flash.h"
extern usart_meta_t UART1_meta;
extern usart_meta_t *p_UART1_meta;

typedef struct _s_firmware_info_ {
	bool is_Available;
	uint32_t address;
	uint32_t length;
	uint32_t crc;
} s_firmware_info;

s_firmware_info Firmware1 = { .is_Available = false, .address =
FIRMWARE1_ADDRESS, .length = NULL, .crc = NULL, };

s_firmware_info Firmware2 = { .is_Available = false, .address =
FIRMWARE2_ADDRESS, .length = NULL, .crc = NULL, };

s_firmware_info Temp_Firmware;

/************************************    Static Functions Decelerations  ************************************/
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer);
static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer);
static void Bootloader_Jump_To_User_App(uint8_t *Host_Buffer);
static void Bootloader_Erase_Flash(uint8_t *Host_Buffer);
static void Bootloader_Memory_Write(uint8_t *Host_Buffer);
static void RESET_CHIP(void);
void Bootloader_CRC_Verify_Seq(uint8_t *pData, uint32_t Data_Len,
		uint32_t *InitVal);
static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len,
		uint32_t Host_CRC);
static void Bootloader_Send_NACK(void);
static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer,
		uint32_t Data_Len);
static uint8_t Host_Address_Verification(uint32_t Jump_Address);
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number,
		uint8_t Number_Of_Sectors);

static uint32_t Firmware_CRC_Verification(s_firmware_info fw);
static uint8_t Flash_Write_Firmware_Info(void);
/************************************    Global Variables Definitions     ************************************/
static uint8_t BL_Host_Buffer[150];
static uint8_t appExists = 0;
uint8_t frame_index = 0;
uint8_t frame_length = 0;
bool receiving_frame = false;
uint16_t frame_timeout = 0;
static uint8_t firmware_sel = 0xFF;

/************************************ Software Interfaces Implementations ************************************/
BL_Status BL_UART_Fetch_Host_Command(void) {
	uint8_t data;
	uint8_t byte_timeout = 200; // Timeout cho từng byte
	BL_Status Status = BL_NACK;

	if(receiving_frame) {
		if(frame_timeout++ > 1000) {
			receiving_frame = false;
			frame_length = 0;
			frame_index = 0;
			frame_timeout = 0;
		}
	}
	while (byte_timeout-- && !rbuffer_empty(&p_UART1_meta->rb_rx)) {
		data = rbuffer_remove(&p_UART1_meta->rb_rx);
		if (!receiving_frame) {
			// Nhận byte đầu tiên (FRAME LENGTH)
			frame_timeout = 0;
			frame_length = data;
			if (frame_length > 0 && frame_length < 255) {
				BL_Host_Buffer[0] = frame_length;
				frame_index = 1;
				receiving_frame = true;
			} else {
				// Nếu frame_length không hợp lệ, reset trạng thái
				frame_index = 0;
				receiving_frame = false;
			}
		} else {
			BL_Host_Buffer[frame_index++] = data;

			if (frame_index >= frame_length + 1) {
				receiving_frame = false;
				frame_length = 0;
				uint16_t Host_CMD_Packet_Len = 0;
				uint32_t Host_CRC32 = 0;
				/* Extract the CRC32 and packet length sent by the HOST */
				Host_CMD_Packet_Len = BL_Host_Buffer[0] + 1;
				Host_CRC32 = *((uint32_t*) ((BL_Host_Buffer
						+ Host_CMD_Packet_Len) - CRC_TYPE_SIZE_BYTE));
				if (CRC_VERIFICATION_FAILED
						== Bootloader_CRC_Verify((uint8_t*) &BL_Host_Buffer[0],
								Host_CMD_Packet_Len - 4, Host_CRC32)) {
					Bootloader_Send_NACK();
					return BL_NACK;
				}
				switch (BL_Host_Buffer[1]) {
				case CBL_GET_CID_CMD:
					Bootloader_Get_Chip_Identification_Number(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_GET_RDP_STATUS_CMD:
					Bootloader_Read_Protection_Level(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_GO_TO_ADDR_CMD:
					Bootloader_Jump_To_User_App(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_FLASH_ERASE_CMD:
					Bootloader_Erase_Flash(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_MEM_WRITE_CMD:
					Bootloader_Memory_Write(BL_Host_Buffer);
					Status = BL_OK;
					break;
				case CBL_RESET_CHIP:
					RESET_CHIP();
					Status = BL_OK;
					break;
				default:
					Status = BL_NACK;
					break;
				}

				return Status;
			}
		}
	}
}

/************************************    Static Functions Implementations  ************************************/
static void Bootloader_Get_Chip_Identification_Number(uint8_t *Host_Buffer) {
	uint16_t MCU_Identification_Number = 0;

	/* Get the MCU chip identification number */
	MCU_Identification_Number = (uint16_t) ((DBGMCU->IDCODE) & 0x00000FFF);
	/* Report chip identification number to HOST */
	Bootloader_Send_Data_To_Host((uint8_t*) &MCU_Identification_Number, 2);

}

static uint8_t CBL_STM32F401_Get_RDP_Level(void) {
	FLASH_OBProgramInitTypeDef FLASH_OBProgram;
	/* Get the Option byte configuration */
	HAL_FLASHEx_OBGetConfig(&FLASH_OBProgram);

	return (uint8_t) (FLASH_OBProgram.RDPLevel);
}

static void Bootloader_Read_Protection_Level(uint8_t *Host_Buffer) {
	uint8_t RDP_Level = 0;

	/* Read Protection Level */
	RDP_Level = CBL_STM32F401_Get_RDP_Level();
	if (0xAA == RDP_Level)
		RDP_Level = 0x00;
	else if (0x55 == RDP_Level)
		RDP_Level = 0x01;
	/* Report Valid Protection Level */
	Bootloader_Send_Data_To_Host((uint8_t*) &RDP_Level, 1);

}

_Bool Jump_To_App(uint32_t address) {
	if (0xFFFFFFFF != *((volatile uint32_t*) address)) {
		appExists = 1;
		Bootloader_Send_Data_To_Host((uint8_t*) &appExists, 1);
		HAL_Delay(1);
		while (!rbuffer_empty(&p_UART1_meta->rb_tx))
			;
		__disable_irq();

		// Tắt tất cả các ngắt và xóa pending interrupts
		for (uint8_t i = 0; i < 8; i++) {
			NVIC->ICER[i] = 0xFFFFFFFF;  // Tắt tất cả các IRQ
			NVIC->ICPR[i] = 0xFFFFFFFF;  // Xóa tất cả pending IRQ
		}

		LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_USART1);
		LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_USART1);
		// Dừng tất cả các ngoại vi đang chạy (tùy vào ứng dụng)

		HAL_RCC_DeInit();
		HAL_DeInit();

		// Xóa bộ nhớ vùng heap & stack (Optional nhưng khuyến khích)
		SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk; // Xóa pending SysTick & PendSV
		SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk; // Xóa pending SysTick

		// Flush bộ nhớ (cần thiết nếu có Prefetch Buffer hoặc DCache/I-Cache)
		__DSB();
		__ISB();

		// Đổi Vector Table sang firmware mới
//		__set_MSP(*((volatile uint32_t*) address));
		SCB->VTOR = address;

		// Đảm bảo mọi thay đổi có hiệu lực trước khi nhảy
		__DSB();
		__ISB();

		SysTick->CTRL = 0;
		SysTick->LOAD = 0;
		SysTick->VAL = 0;  // Đảm bảo bộ đếm cũng reset về 0

		__enable_irq();
		// Tắt SysTick

		// Nhảy vào firmware mới
		uint32_t MainAppAddr = *((volatile uint32_t*) (address + 4));
		void (*reset_handler)(void) = (void(*)(void))MainAppAddr;
		reset_handler();
	}
	return false;
}

static void Bootloader_Jump_To_User_App(uint8_t *Host_Buffer) {
	if (Host_Buffer == NULL) {
		return; // Tránh lỗi truy cập NULL
	}

	uint32_t app_address = ((uint32_t) Host_Buffer[2] << 24)
			| ((uint32_t) Host_Buffer[3] << 16)
			| ((uint32_t) Host_Buffer[4] << 8) | ((uint32_t) Host_Buffer[5]);

	if (!Jump_To_App(app_address)) {
		uint8_t appExists = 0;
		Bootloader_Send_Data_To_Host(&appExists, 1);
	}
}
static uint8_t Perform_Flash_Erase(uint8_t Sector_Number,
		uint8_t Number_Of_Sectors) {
	if (Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER) {
		return INVALID_SECTOR_NUMBER;  // Quá số sector cho phép
	}

	if (Sector_Number
			> (CBL_FLASH_MAX_SECTOR_NUMBER - 1)&& Sector_Number != CBL_FLASH_MASS_ERASE) {
		return UNSUCCESSFUL_ERASE;  // Sector không hợp lệ
	}

	FLASH_Unlock();

	if (Sector_Number == CBL_FLASH_MASS_ERASE) {
		uint8_t result = (FLASH_Erase_All() == FLASH_OK) ?
		SUCCESSFUL_ERASE :
															UNSUCCESSFUL_ERASE;
		FLASH_Lock();
		return result;
	}

	// Đảm bảo không xóa quá giới hạn sector
	Number_Of_Sectors =
			(Sector_Number + Number_Of_Sectors > CBL_FLASH_MAX_SECTOR_NUMBER) ?
					CBL_FLASH_MAX_SECTOR_NUMBER - Sector_Number :
					Number_Of_Sectors;

	for (uint8_t i = 0; i < Number_Of_Sectors; i++) {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
		if (FLASH_Erase_Sectors(Sector_Number + i) != FLASH_OK) {
			FLASH_Lock();
			return UNSUCCESSFUL_ERASE;
		}
	}

	FLASH_Lock();
	return SUCCESSFUL_ERASE;
}

static void Bootloader_Erase_Flash(uint8_t *Host_Buffer) {
	uint8_t Erase_Status = 0;

	/* Perform Mass erase or sector erase of the user flash */
	if (Host_Buffer[2] == 1) {
		Erase_Status = Perform_Flash_Erase(FIRMWARE1_SECTOR,
		FW1_NUM_SECTORS);
		Firmware1.is_Available = false;
		Firmware1.address = 0;
		Firmware1.length = 0;
		Firmware1.crc = 0;
	} else if (Host_Buffer[2] == 2) {
		Erase_Status = Perform_Flash_Erase(FIRMWARE2_SECTOR,
		FW2_NUM_SECTORS);
		Firmware2.is_Available = false;
		Firmware2.address = 0;
		Firmware2.length = 0;
		Firmware2.crc = 0;
	}
	if (SUCCESSFUL_ERASE == Erase_Status) {
		/* Report erase Passed */
		Flash_Write_Firmware_Info();
		Bootloader_Send_Data_To_Host((uint8_t*) &Erase_Status, 1);
	} else {
		/* Report erase failed */
		Bootloader_Send_Data_To_Host((uint8_t*) &Erase_Status, 1);
	}
}

static uint8_t Flash_Memory_Write_Payload(uint8_t *Host_Payload,
		uint32_t Payload_Start_Address, uint16_t Payload_Len) {
	FLASH_StatusTypeDef Flash_Status;

	/* Unlock the FLASH control register access */
	FLASH_Unlock();

	for (uint16_t i = 0; i < Payload_Len; i++) {
		/* Program a byte at a specified address */
		Flash_Status = FLASH_Write_Byte(Payload_Start_Address + i,
				Host_Payload[i]);

		if (Flash_Status != FLASH_OK) {
			FLASH_Lock();  // Khóa Flash trước khi thoát
			return FLASH_PAYLOAD_WRITE_FAILED;
		}
	}

	/* Khóa Flash sau khi hoàn tất ghi */
	FLASH_Lock();
	return FLASH_PAYLOAD_WRITE_PASSED;
}

static void Bootloader_Memory_Write(uint8_t *Host_Buffer) {

	uint32_t HOST_Address = 0;
	uint8_t Payload_Len = 0;
	uint8_t Address_Verification = ADDRESS_IS_INVALID;
	uint8_t Flash_Payload_Write_Status = FLASH_PAYLOAD_WRITE_FAILED;
	uint16_t Frame_Index = 0;
	uint16_t Total_Frame = 0;

	Frame_Index = *((uint16_t*) (&Host_Buffer[7]));
	Total_Frame = *((uint16_t*) (&Host_Buffer[9]));

	/* Extract the CRC32 and packet length sent by the HOST */
	HOST_Address = *((uint32_t*) (&Host_Buffer[2]));
	if ((HOST_Address == FIRMWARE1_ADDRESS)
			|| (HOST_Address == FIRMWARE2_ADDRESS)) {
		Temp_Firmware.address = HOST_Address;
		Temp_Firmware.crc = 0;
		Temp_Firmware.length = 0;
	}

	/* Extract the payload length from the Host packet */
	Payload_Len = Host_Buffer[6];
	/* Verify the Extracted address to be valid address */
	Address_Verification = Host_Address_Verification(HOST_Address);
	if (ADDRESS_IS_VALID == Address_Verification) {
		/* Write the payload to the Flash memory */
		Flash_Payload_Write_Status = Flash_Memory_Write_Payload(
				(uint8_t*) &Host_Buffer[11], HOST_Address, Payload_Len);
		if (FLASH_PAYLOAD_WRITE_PASSED == Flash_Payload_Write_Status) {
			Temp_Firmware.length += Payload_Len;
			if (Frame_Index == (Total_Frame - 1)) {
				Temp_Firmware.crc = Firmware_CRC_Verification(Temp_Firmware);
				if (Temp_Firmware.address == FIRMWARE1_ADDRESS) {
					Firmware1 = Temp_Firmware;
					Firmware1.is_Available = true;
				} else if (Temp_Firmware.address == FIRMWARE2_ADDRESS) {
					Firmware2 = Temp_Firmware;
					Firmware2.is_Available = true;
				}
				Flash_Write_Firmware_Info();
			}
			/* Report payload write passed */
			Bootloader_Send_Data_To_Host((uint8_t*) &Flash_Payload_Write_Status,
					1);
		} else {
			/* Report payload write failed */
			Bootloader_Send_Data_To_Host((uint8_t*) &Flash_Payload_Write_Status,
					1);
		}
	} else {
		/* Report address verification failed */
		Address_Verification = ADDRESS_IS_INVALID;
		Bootloader_Send_Data_To_Host((uint8_t*) &Address_Verification, 1);
	}

}

static uint8_t Host_Address_Verification(uint32_t Jump_Address) {
	uint8_t Address_Verification = ADDRESS_IS_INVALID;

	if ((Jump_Address >= SRAM1_BASE) && (Jump_Address <= STM32F407_SRAM_END)) {
		Address_Verification = ADDRESS_IS_VALID;
	} else if ((Jump_Address >= FLASH_BASE)
			&& (Jump_Address <= STM32F407_FLASH_END)) {
		Address_Verification = ADDRESS_IS_VALID;
	} else {
		Address_Verification = ADDRESS_IS_INVALID;
	}
	return Address_Verification;
}

static uint8_t Bootloader_CRC_Verify(uint8_t *pData, uint32_t Data_Len,
		uint32_t Host_CRC) {
	uint8_t CRC_Status = CRC_VERIFICATION_FAILED;
	uint32_t MCU_CRC_Calculated = 0;
	if (Data_Len == 0xFFFFFFFF)
		return MCU_CRC_Calculated;
	CRC->CR = CRC_CR_RESET;
	for (unsigned int i = 0; i < Data_Len; i++) {
		CRC->DR = (uint32_t) pData[i];
	}
	if (CRC->DR == Host_CRC) {
		CRC_Status = CRC_VERIFICATION_PASSED;
	} else {
		CRC_Status = CRC_VERIFICATION_FAILED;
	}

	return CRC_Status;
}

void Bootloader_CRC_Verify_Seq(uint8_t *pData, uint32_t Data_Len,
		uint32_t *InitVal) {
	CRC->CR = CRC_CR_RESET;
	for (unsigned int i = 0; i < Data_Len; i++) {
		CRC->DR = (uint32_t) pData[i];
	}
	*InitVal = CRC->DR;
}

static void Bootloader_Send_NACK(void) {
	uint8_t Ack_Value = CBL_SEND_NACK;
	USART1_send_array((const char*) &Ack_Value, 1);
}

static void Bootloader_Send_Data_To_Host(uint8_t *Host_Buffer,
		uint32_t Data_Len) {
	USART1_send_array((const char*) Host_Buffer, (uint8_t) Data_Len);
}

static void RESET_CHIP(void) {
	__disable_irq();

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;

	NVIC_SystemReset();
}

static uint32_t Firmware_CRC_Verification(s_firmware_info fw) {
	if (fw.length == 0)
		return 0;

	uint32_t address = fw.address;
	uint32_t end_address = fw.address + fw.length;
	uint8_t data = 0;

	CRC->CR = CRC_CR_RESET;

	while (address < end_address) {
		data = *(uint8_t*) address;
		CRC->DR = (uint32_t) data;
		address++;
	}

	return CRC->DR;
}

static uint8_t Flash_Write_Firmware_Info(void) {
	FLASH_Unlock();

	if (FLASH_Erase_Sectors(FIRMWARE_SAVE_SECTOR) != FLASH_OK) {
		FLASH_Lock();
		return FLASH_PAYLOAD_WRITE_FAILED;
	}

	uint8_t *data_ptr = (uint8_t*) &Firmware1;
	for (uint32_t i = 0; i < sizeof(s_firmware_info); i += 4) {
		if (FLASH_Write_Word(FIRMWARE_SAVE + i, *(uint32_t*) (data_ptr + i))
				!= FLASH_OK) {
			FLASH_Lock();
			return FLASH_PAYLOAD_WRITE_FAILED;
		}
	}

	/* Ghi cả struct Firmware2 */
	data_ptr = (uint8_t*) &Firmware2;
	for (uint32_t i = 0; i < sizeof(s_firmware_info); i += 4) {
		if (FLASH_Write_Word(FIRMWARE_SAVE + sizeof(s_firmware_info) + i,
				*(uint32_t*) (data_ptr + i)) != FLASH_OK) {
			FLASH_Lock();
			return FLASH_PAYLOAD_WRITE_FAILED;
		}
	}

	FLASH_Lock();
	return FLASH_PAYLOAD_WRITE_PASSED;
}

static void Flash_Read_Firmware_Info(void) {
	memcpy(&Temp_Firmware, (void*) FIRMWARE_SAVE, sizeof(s_firmware_info));
	if (Temp_Firmware.address != 0xFFFFFFFF) {
		memcpy(&Firmware1, &Temp_Firmware, sizeof(s_firmware_info));
	}
	memcpy(&Temp_Firmware, (void*) (FIRMWARE_SAVE + sizeof(s_firmware_info)),
			sizeof(s_firmware_info));
	if (Temp_Firmware.address != 0xFFFFFFFF) {
		memcpy(&Firmware2, &Temp_Firmware, sizeof(s_firmware_info));
	}
}

void Firmware_Check_Available() {
	uint32_t CRC_Result = 0;
	Flash_Read_Firmware_Info();

	if (Firmware1.is_Available == true) {
		USART1_send_string("Firmware 1 CRC checking...\n");
		CRC_Result = Firmware_CRC_Verification(Firmware1);
		if (CRC_Result == Firmware1.crc) {
			USART1_send_string("Firmware 1 CRC successfully checked\n");
			USART1_send_string("Chosing Firmware 1 for auto boot\n");
			firmware_sel = 1;
			return;
		}
	}

	if (Firmware2.is_Available == true) {
		USART1_send_string("Firmware 2 CRC checking...\n");
		CRC_Result = Firmware_CRC_Verification(Firmware2);
		if (CRC_Result == Firmware2.crc) {
			USART1_send_string("Firmware 2 CRC successfully checked\n");
			USART1_send_string("Chosing Firmware 2 for auto boot\n");
			firmware_sel = 2;
			return;
		}
	}
	USART1_send_string("No firmware dectected");
}

void Wait_For_Request() {
	uint32_t tick = HAL_GetTick();
	USART1_send_string("Bootloader wait for command\n");
	while (rbuffer_empty(&p_UART1_meta->rb_rx)) {
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_11);
		HAL_Delay(500);
		if (HAL_GetTick() - tick > 10000) {
			if (firmware_sel == 1) {
				USART1_send_string("Timeout, jump to Firmware 1\n");
				Jump_To_App(Firmware1.address);
				return;
			} else if (firmware_sel == 2) {
				USART1_send_string("Timeout, jump to Firmware 2\n");
				Jump_To_App(Firmware2.address);
				return;
			} else {
				USART1_send_string("Timeout, no firmware dectected");
				return;
			}
		}
	}
}
