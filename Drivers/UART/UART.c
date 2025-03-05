/*
 * UART2.c
 *
 *  Created on: Nov 16, 2024
 *      Author: thanh
 */

#include "UART.h"

#include <stm32f4xx.h>
#include <stm32f4xx_ll_usart.h>

static const char *const g_pcHex = "0123456789abcdef";

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
usart_meta_t UART1_meta;
usart_meta_t *p_UART1_meta = &UART1_meta;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS

void USART1_IRQ(void) {
	uint8_t data;
	if (LL_USART_IsActiveFlag_TXE(USART1)) {
		if (!rbuffer_empty(&p_UART1_meta->rb_tx)) {
			data = rbuffer_remove(&p_UART1_meta->rb_tx);
			LL_USART_TransmitData9(USART1, (uint16_t) data);
		} else
			LL_USART_DisableIT_TXE(USART1);
	}
	if (LL_USART_IsActiveFlag_RXNE(USART1)) {
		data = LL_USART_ReceiveData8(USART1);
		if (!rbuffer_full(&p_UART1_meta->rb_rx)) {
			rbuffer_insert(data, &p_UART1_meta->rb_rx);
		}
		LL_USART_ClearFlag_RXNE(USART1);
	}
}
void USART1_init(void) {
	rbuffer_init(&p_UART1_meta->rb_tx);                        // Init Rx buffer
	rbuffer_init(&p_UART1_meta->rb_rx);                        // Init Tx buffer
	LL_USART_EnableIT_RXNE(USART1);
}

void USART1_send_char(char c) {
	while (rbuffer_full(&p_UART1_meta->rb_tx))
		;
	rbuffer_insert(c, &p_UART1_meta->rb_tx);
	LL_USART_EnableIT_TXE(USART1);
}

void USART1_send_string(const char *str) {
	while (*str) {
		USART1_send_char(*str++);
	}
}

void USART1_send_array(const char *str, uint8_t len) {
	uint8_t udx;
	for (udx = 0; udx < len; udx++)
		USART1_send_char(*str++);
}

uint8_t USART1_rx_count(void) {
	return rbuffer_count(&p_UART1_meta->rb_rx);
}

uint16_t USART1_read_char(void) {
	return 0;										//fix sau!
	if (!rbuffer_empty(&p_UART1_meta->rb_rx)) {
//		return (((p_UART1_meta->usart_error & USART_RX_ERROR_MASK) << 8)
//				| (uint16_t) rbuffer_remove(&p_UART1_meta->rb_rx));
	} else {
//		return (((p_UART1_meta->usart_error & USART_RX_ERROR_MASK) << 8)
//				| USART_NO_DATA);     // Empty ringbuffer
	}
}

void USART1_close() {
	while (!rbuffer_empty(&p_UART1_meta->rb_tx))
		;                // Wait for Tx to transmit ALL characters in ringbuffer
//	while (!(UCSR0A & (1 << TXC)))
//		;		// Wait for Tx unit to transmit the LAST character of ringbuffer

//	_delay_ms(200);                            // Extra safety for Tx to finish!

//	UCSR0B &= ~((1 << RXEN) | (1 << TXEN) | (1 << RXCIE) | (1 << UDRIE)); //disable TX, RX, RX interrupt
//	UCSR0C &= (1 << UCSZ1) | (1 << UCSZ0);
}

void UASRT1_SetBaudRate(uint32_t Baud) {
	while (!rbuffer_empty(&p_UART1_meta->rb_tx))
		;
	LL_USART_Disable(USART1);
	LL_USART_InitTypeDef USART_InitStruct = { 0 };
	USART_InitStruct.BaudRate = Baud;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART1, &USART_InitStruct);
	LL_USART_ConfigAsyncMode(USART1);
	LL_USART_Enable(USART1);
}

volatile ringbuffer_t* uart_get_USART1_rx_buffer_address(void) {
	return p_UART1_meta;
}
