#include <kernel/serial.h>
#include <kernel/panic.h>
#include <arch/io.h>
#include <kernel/mm.h>
#include <arch/spinlock.h>
#include <kernel/string.h>
#include <arch/plic.h>
#include <arch/csr.h>

static struct {
	char buffer[SERIAL_BUFFER_SIZE];
	size_t len;
	struct spinlock lock;
} dev;

// cute itty bitty helper function to have the serial direct mapped 
// offset will be defined by SERIAL_THR, SERIAL_LSR, etc. at io.h
static void *serial_reg(u64 off)
{
    return (void *)((u64)SERIAL_BASE + KERNEL_DIRECT_MAP_START + off);
}

void serial_init()
{
	// enablind and clearing the fifos
	iowrite8(SERIAL_FCR_FIFO_ENABLE | SERIAL_FCR_RX_FIFO_CLEAR | SERIAL_FCR_TX_FIFO_CLEAR, serial_reg(SERIAL_FCR));
	// DONE: enable rx interrupt
	spin_init(&dev.lock);
	dev.len = 0;

}

void serial_irq_enable()
{
	// OPEN THE GATES !
	iowrite8(SERIAL_IER_ERBFI, serial_reg(SERIAL_IER)); 
	plic_irq_set_priority(IRQ_SERIAL, 1); // upgrade priority
	plic_hart_set_threshold(0, 0); // hart 0 accepts all > 0
	plic_hart_enable_irq(0, IRQ_SERIAL); // enable the irq for hart 0
	csr_set(CSR_SIE, CSR_SIE_SEIE);
}

void serial_irq_disable()
{
	iowrite8(0, serial_reg(SERIAL_IER)); // disable all serial interrupts
	plic_hart_enable_irq(0, IRQ_SERIAL); // disable the irq for hart 0
}

void serial_irq()
{
	// read RBR until clears, so irq doesnt storm 
	spin_lock(&dev.lock);
	while (ioread8(serial_reg(SERIAL_LSR)) & SERIAL_LSR_DTR) { // DTR = data ready
		char c = (char)ioread8(serial_reg(SERIAL_RBR));
		if (dev.len < SERIAL_BUFFER_SIZE) {
			dev.buffer[dev.len++] = c; // else: buffer full
		}
	}
	spin_unlock(&dev.lock);
}

size_t serial_read(char *buf)
{
	u64 flags = spin_lock_irqsave(&dev.lock);
	size_t len = dev.len;
	memcpy(buf, dev.buffer, len);
	dev.len = 0; // reset buffer
	spin_unlock_irqrestore(&dev.lock, flags); // needs irqsave bc runs with interrupts #on
	return len;
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
