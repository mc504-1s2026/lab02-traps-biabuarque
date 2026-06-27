#include <arch/timer.h>
#include <kernel/panic.h>
#include <arch/csr.h>
#include <kernel/serial.h>

u64 timer_read()
{
	return csr_read(CSR_TIME);
}

void timer_irq_enable()
{
	csr_set(CSR_SIE, CSR_SIE_STIE);
}

void timer_irq_disable()
{
	csr_clear(CSR_SIE, CSR_SIE_STIE);
}

void timer_set_alarm(u64 secs)
{
	csr_write(CSR_STIMECMP, timer_read() + secs * (u64)TIMER_FREQ);
}

void timer_irq()
{
	// disarms the alarm. we cant directly clear the timer checker so
	csr_write(CSR_STIMECMP, ~0UL);
	// print the alarm message
	serial_puts("alarm\r\n");
}
