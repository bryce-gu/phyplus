#ifndef __UART_ROM_H__
#define __UART_ROM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "bus_dev.h"
#include "gpio.h"

#define UART_TX_FIFO_SIZE    16
#define UART_RX_FIFO_SIZE    16

#define FCR_RX_TRIGGER_00 0x00
#define FCR_RX_TRIGGER_01 0x40
#define FCR_RX_TRIGGER_10 0x80
#define FCR_RX_TRIGGER_11 0xc0
#define FCR_TX_TRIGGER_00 0x00
#define FCR_TX_TRIGGER_01 0x10
#define FCR_TX_TRIGGER_10 0x20
#define FCR_TX_TRIGGER_11 0x30
#define FCR_TX_FIFO_RESET 0x04
#define FCR_RX_FIFO_RESET 0x02
#define FCR_FIFO_ENABLE   0x01

#define UART_FIFO_RX_TRIGGER	FCR_RX_TRIGGER_01//FCR_RX_TRIGGER_10//FCR_RX_TRIGGER_11
#define UART_FIFO_TX_TRIGGER	FCR_TX_TRIGGER_00//FCR_TX_TRIGGER_00//FCR_TX_TRIGGER_01

#define IER_PTIME   0x80
#define IER_EDSSI   0x08
#define IER_ELSI    0x04
#define IER_ETBEI   0x02
#define IER_ERBFI   0x01

/*LSR 0x14*/
#define LSR_RFE		0x80
#define LSR_TEMT	0x40
#define LSR_THRE	0x20
#define LSR_BI		0x10
#define LSR_FE		0x08
#define LSR_PE		0x04
#define LSR_OE		0x02
#define LSR_DR		0x01

/*USR 0x7c*/
#define USR_RFF   0x10
#define USR_RFNE  0x08
#define USR_TFE   0x04
#define USR_TFNF  0x02
#define USR_BUSY  0x01

#define DUMMY_UART_RX_FORCE_HIGH_ENABLE          {AP_COM->new_add_reg1 |= BIT(16);}

#define DUMMY_UART_RX_FORCE_HIGH_DISABLE         {AP_COM->new_add_reg1 &= ~BIT(16);}


#define SWU_TX() swu_tx_mode(0)
#define SWU_RX() swu_rx_mode(0)

#define SWU_TX_B(baud) swu_tx_mode(baud)
#define SWU_RX_B(baud) swu_rx_mode(baud)

//#define SWU_DUMMY_PIN   P16
//#define SWU_TX_PIN      P9
//#define SWU_RX_PIN      P10
//#define SWU_RX_PIN_2ND  P17

#define SWU_DUMMY_PIN   GPIO_DUMMY
#define SWU_TX_PIN      P4
#define SWU_RX_PIN      P5


enum UARTIRQID{
	NONE_IRQ = 0,
	NO_IRQ_PENDING_IRQ = 1,
	THR_EMPTY = 2,
	RDA_IRQ = 4,
	RLS_IRQ = 6,
	BUSY_IRQ = 7,
	TIMEOUT_IRQ = 12,
};

typedef enum{
  UART_EVT_TYPE_RX_DATA = 1,
  UART_EVT_TYPE_RX_DATA_TO, //case rx data of uart RX timeout
  UART_EVT_TYPE_TX_COMPLETED,
} uart_Evt_Type_t;

/*
 * rom use:
 * SWU_*
 * swu_*
*/
void SWU_UART0_IRQHandler(void);
void swu_uart_init(int baud, gpio_pin_e tx_pin, gpio_pin_e rx_pin, comm_cb_t cb);
void swu_uart_deinit(void);
	
void swu_uart_tx(const char *data);
void swu_uart_send_buff(unsigned char *data,int len);
void swu_uart_set_baudrate(uint32_t baud);
void swu_tx_mode(int baud);
void swu_rx_mode(int baud);
void swu_uart_rxtrigger(uint32_t rxfifo_num);
void swu_uart_txtrigger(uint32_t txfifo_num);

/*
 * sdk use
 * */

enum
{
    TX_STATE_UNINIT = 0,
    TX_STATE_IDLE,
    TX_STATE_TX,
    TX_STATE_ERR
};

typedef struct _uart_Evt_t
{
    uint8_t   type;
    uint8_t*  data;
    uint8_t   len;
} uart_Evt_t;

//uart interrupt function
typedef void (*uart_int_t)(void);

//uart data process,uart int callback
typedef void (*uart_Hdl_t)(uart_Evt_t* pev);

typedef struct _uart_Cfg_t
{
    gpio_pin_e  tx_pin;
    gpio_pin_e  rx_pin;
    uint32_t    baudrate;
    bool        use_fifo;
    bool        use_tx_buf;
    bool        parity;
	uint8_t     fcr;	//FCR_TX_FIFO_RESET|FCR_RX_FIFO_RESET|FCR_FIFO_ENABLE|UART_FIFO_RX_TRIGGER|UART_FIFO_TX_TRIGGER; 
    uart_Hdl_t  evt_handler;
} uart_Cfg_t;

typedef struct _uart_Tx_Buf_t
{
    uint8_t   tx_state;
    uint16_t  tx_data_offset;
    uint16_t  tx_data_size;
    uint16_t  tx_buf_size;
    uint8_t*  tx_buf;
} uart_Tx_Buf_t;

void hal_uart_init(uart_Cfg_t cfg);
void hal_uart_deinit(void);
int hal_uart_set_tx_buf(uint8_t* buf, uint16_t size);
int hal_uart_get_tx_ready(void);
int hal_uart_send_buff(uint8_t* buff,uint16_t len);
int hal_uart_send_byte(unsigned char data);
void __attribute__((weak)) hal_UART0_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif
