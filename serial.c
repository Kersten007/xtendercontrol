//
//  UNIX Serial Port Interface
//
//  Created by K3A (www.k3a.me) on 13.12.2018.
//  Released under MIT
//


#include "serial.h"

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#define error_message(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

int serial_fd = 0;

static int set_interface_attribs (int fd, int speed, serial_parity_t parity, int stop_bits)
{
  struct termios tio;
  
  bzero(&tio, sizeof(tio)); // clear struct for new port settings

  tio.c_cflag = speed | \
    CS8 /* 8 data bits */ | \
    CLOCAL /* Ignore modem control lines */ | \
    CREAD /* Enable receiver */;

  if (stop_bits == 2) {
    tio.c_cflag |= CSTOPB;
  }

  if (parity > 0) {
    tio.c_cflag |= PARENB;
    if (parity & PARITY_ODD) {
      tio.c_cflag |= PARODD;
    }
  }
 
  // keep input, output and line flags empty
  tio.c_iflag = 0;
  tio.c_oflag = 0;
  tio.c_lflag = 0; // will be noncanonical mode (ICANON not set)

  // set blocking
  tio.c_cc[VMIN]  = 1; // minimum number of characters for noncanonical read
  tio.c_cc[VTIME] = 5; // 1.5 second read timeout
 
  if (tcsetattr(fd, TCSANOW, &tio) != 0)
  {
    error_message ("tcsetattr error %d: %s", errno, strerror(errno));
    return -1;
  }

  return 0;
}

void serial_init(void)
{
      _serial_init("/dev/ttyUSB0", /*B38400*/B115200, PARITY_EVEN, 1);
}

void serial_close(void)
{


}

void serial_clear()
{
    ;
}


int _serial_init(const char* port_path, int speed, serial_parity_t parity, int stop_bits) {
  int ret;

  serial_fd = open(port_path, O_RDWR | O_NOCTTY);
  if (serial_fd < 0) {
    error_message ("error %d opening %s: %s", errno, port_path, strerror(errno));
    return serial_fd;
  }

  // set speed and parity
  if ((ret = set_interface_attribs(serial_fd, speed, parity, stop_bits)) < 0) { 
    return ret;
  }

  return 0;
}

// write to serial port size bytes from ptr
int serial_write(const void* ptr, unsigned size) {
  int ret;

  if ((ret = write(serial_fd, ptr, size)) < 0) {
    error_message ("error writing to serial port: %s", strerror(errno));
  }

  return ret;
}



// read size bytes from serial into ptr buffer
int serial_read(void* ptr, unsigned size) {
  unsigned char* buf = (unsigned char*)ptr;
  unsigned bytesRead = 0;

  while (bytesRead < size) {
    int ret = read(serial_fd, buf+bytesRead, size-bytesRead);

    if (ret < 0) {
      return 0;
    }

    bytesRead += ret;
  }
  return bytesRead;
}

// read and skip bytes until your receive a byte b. Skip this byte as well and return. 
void serial_skip(unsigned char b) {
  unsigned char readByte = 123;
  serial_ret_t ret;

  while ( (ret = serial_read(&readByte, 1) ) == SERIAL_OK ) {
    if (readByte == b) {
      break; // done reading
    }
  }
}