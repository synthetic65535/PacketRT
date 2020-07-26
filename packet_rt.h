#include "main.h"

/* -------- Settings -------- */

// Packet size, to optimize RAM usage
#define MAX_PACKET_SIZE 1024

/* -------- Definitions -------- */

// Packet receive callback type
typedef void(* packet_rt_callback_t)(uint8_t *data, uint32_t size);

/* -------- Functions -------- */

// Initialization with packet receive callback
void packet_rt_initpacket_rt_init(UART_HandleTypeDef *_huart, packet_rt_callback_t callback);

// Send Packet. Returns 0 on success.
uint8_t packet_rt_send(uint8_t *data, uint32_t size);

