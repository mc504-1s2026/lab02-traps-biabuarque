#include <kernel/serial.h>
#include <kernel/panic.h>
#include <arch/io.h>
#include <kernel/mm.h>

// cute itty bitty helper function to have the serial direct mapped 
// offset will be defined by SERIAL_THR, SERIAL_LSR, etc. at io.h
static volatile u8 *serial_reg(u64 off)
{
    return (volatile u8 *)((u64)SERIAL_BASE + KERNEL_DIRECT_MAP_START + off);
}

void serial_init()
{
	// enablind and clearing the fifos
	iowrite8(SERIAL_FCR_FIFO_ENABLE | SERIAL_FCR_RX_FIFO_CLEAR | SERIAL_FCR_TX_FIFO_CLEAR, serial_reg(SERIAL_FCR));
	// TODO: enable rx interrupt
}

void serial_irq_enable()
{
	/* not implemented */
	BUG();
}

void serial_irq_disable()
{
	/* not implemented */
	BUG();
}

void serial_irq()
{
	/* not implemented */
	BUG();
}

size_t serial_read(char *buf)
{
	/* not implemented */
	BUG();
}

void serial_puts(char *str)
{
	// putc for the whole string
	// girl so clever not having the loops first args
	for (; *str != '\0'; str++) {
		if (*str == '\n') {
			serial_putc('\r');
		}
		serial_putc(*str);
	}
}

void serial_putc(char c)
{
	// wait for THRE, write THR
	while(!(ioread8(serial_reg(SERIAL_LSR)) & SERIAL_LSR_THRE));
	iowrite8((u8)c, serial_reg(SERIAL_THR));
}
