/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <utils.h>
#include <device/map.h>

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

  #include <sys/select.h>
  #include <unistd.h>
  #include <string.h>

  #define TX_OFFSET 0
  #define RX_OFFSET 4

static uint8_t *serial_base = NULL;

static void serial_putc(char ch) {
    MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
  }

static char serial_getc(void) {
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  struct timeval timeout = {
    .tv_sec = 0,
    .tv_usec = 0,
  };

  if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) <= 0) {
    return 0;
  }

  uint8_t ch;
  if (read(STDIN_FILENO, &ch, 1) != 1) {
    return 0;
  }

  return ch;
}

static void serial_io_handler(uint32_t offset, int len, bool is_write) {
  switch (offset) {
    case TX_OFFSET:
      assert(is_write && len == 1);
      serial_putc(serial_base[TX_OFFSET]);
      break;

    case RX_OFFSET: {
      assert(!is_write && len == 4);
      uint32_t data = serial_getc();
      memcpy(serial_base + RX_OFFSET, &data, sizeof(data));
      break;
    }

    default:
      panic("unsupported serial access: offset=%u len=%d write=%d",
            offset, len, is_write);
  }
}

void init_serial(void) {
  serial_base = new_space(8);
  memset(serial_base, 0, 8);

#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("serial", CONFIG_SERIAL_PORT,
              serial_base, 8, serial_io_handler);
#else
  add_mmio_map("serial", CONFIG_SERIAL_MMIO,
                serial_base, 8, serial_io_handler);
#endif
}

