#include "packet_rt.h"
#include "cmsis_os.h"
#include "base64.h"
#include "crc32.h"
#include "stm32f1xx_hal_uart.h"

/* -------- Constants -------- */

#define PACKET_DELIMITER '\n'
#define DELIMITER_SIZE 1
#define CRC32_SIZE 4
#define MAX_ENCODED_PACKET_SIZE (((MAX_PACKET_SIZE + CRC32_SIZE) / 3 + 1) * 4)
#define RX_BUFF_SIZE (MAX_ENCODED_PACKET_SIZE + DELIMITER_SIZE + 1)
#define TX_BUFF_SIZE (MAX_ENCODED_PACKET_SIZE + DELIMITER_SIZE + 1)

/* -------- Variables -------- */

// Packet Receive Callback
volatile packet_rt_callback_t packet_receive_callback;
// Receive Ring Buffer
volatile uint8_t rx_buff[RX_BUFF_SIZE];
volatile uint32_t rx_head=0, rx_tail=0;
// Transmit Ring Buffer
volatile uint8_t tx_buff[TX_BUFF_SIZE];
volatile uint32_t tx_head=0, tx_tail=0;
// Temp bytes
volatile uint8_t rx_byte, tx_byte;
// Received Packed
volatile uint8_t rx_packet[MAX_PACKET_SIZE + CRC32_SIZE];
// Packed to transmit
volatile uint8_t tx_packet[MAX_PACKET_SIZE + CRC32_SIZE];
// Transmit Mutex
volatile uint8_t tx_mutex=0;
// UART Handle
UART_HandleTypeDef *huart;

/* -------- Functions -------- */


// Initialization
void packet_rt_init(UART_HandleTypeDef *_huart, packet_rt_callback_t callback)
{
	packet_receive_callback = callback;
	tx_mutex = 0;
	rx_head = rx_tail = 0;
	tx_head = tx_tail = 0;
	huart = _huart;
	HAL_UART_Receive_IT(huart, (uint8_t*)&rx_byte, 1);
}


// Send Byte
static void putchar(uint8_t data)
{
	uint32_t tx_head_next = (tx_head + 1) % TX_BUFF_SIZE;
	if (tx_head_next != tx_tail) // Check if ring buffer is not full
	{
		if (tx_head == tx_tail) // If ring buffer was empty
		{
			if (huart->gState == HAL_UART_STATE_BUSY_TX) // If transmission is in progress
			{
				// Just add a new byte to buffer
				tx_buff[tx_head++] = data;
			} else { // If transmitter is free
				// Try to start transmission
				tx_byte = data;
				HAL_UART_Transmit_IT(huart, (uint8_t*)&tx_byte, 1);
			}
		} else { // If buffer was not empty
			// Just add a new byte to buffer
			tx_buff[tx_head++] = data;
		}
	} else { // If buffer is full
		// Try to start transmission
		HAL_UART_Transmit_IT(huart, (uint8_t*)&tx_byte, 1);
	}
}


// Send Packet
uint8_t packet_rt_send(uint8_t *data, uint32_t size)
{
	if ((tx_head != tx_tail) || tx_mutex) return 1; // If transmission buffer is not empty
	tx_mutex = 1;
	for (uint32_t i=0; i<size; i++) tx_packet[i] = data[i]; // Copy data
	*(uint32_t*)(&tx_packet[size]) = calc_crc32((uint8_t *)&tx_packet, size); // Add CRC32
	base64_encode((char *)&tx_buff, (char *)&tx_packet, size + 4); // Encode
	for (uint32_t i=0; i<size+4; i++) putchar(tx_packet[i]); // Transmit
	putchar(PACKET_DELIMITER); // Send packet delimiter
	tx_mutex = 0;
	return 0;
}


// Packet decoding procedure
static void try_decode_packet(uint32_t length)
{
	uint32_t result = base64_decode((char *)&rx_packet, (char *)&rx_buff, length); // Decode
	if (result >= 4)
	{
		uint32_t crc32 = calc_crc32((uint8_t *)&rx_packet, result - 4); // Calc CRC32
		if (crc32 == ((uint32_t)(&rx_packet[result - 4]))) // Check CRC32
		{
			packet_receive_callback((uint8_t *)&rx_packet, result - 4); // Callback
		}
	}
}


// USART Receive Complete
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *_huart)
{
	if(huart == _huart)
	{
		uint32_t rx_head_next = (rx_head + 1) % RX_BUFF_SIZE;
		if (rx_head_next != rx_tail) // Check if ring buffer is not full
		{
			// Add new byte
			rx_buff[rx_head] = rx_byte;
			if (rx_byte == PACKET_DELIMITER) // If packet received
			{
				try_decode_packet(rx_head);
				rx_head = rx_tail = 0; // Reset buffer
			} else {
				rx_head = rx_head_next;
			}
		}
		// Continue receiving
		HAL_UART_Receive_IT(huart, (uint8_t*)&rx_byte, 1);
	}
}


// USART Transmit Complete
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *_huart)
{
	if(huart == _huart)
	{
		if (tx_head == tx_tail) // If transmission buffer is empty
		{
			// Nothing left to do
		} else { // If it is something in transmission buffer
			// Send next byte
			HAL_UART_Transmit_IT(huart, (uint8_t*)&(tx_buff[tx_tail]), 1);
			tx_tail = (tx_tail + 1) % TX_BUFF_SIZE;
		}
	}
}


