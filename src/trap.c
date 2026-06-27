#include <kernel/trap.h>
#include <kernel/panic.h>
#include <arch/csr.h>
#include <arch/timer.h>
#include <kernel/serial.h>
#include <arch/plic.h>

/* defined in src/trap_entry.S */
extern void trap_entry();

void handle_irq()
{
	u64 scause = csr_read(CSR_SCAUSE);
	switch (scause) {
		case TRAP_TIMER_IRQ:
			timer_irq();
			break;
		case TRAP_EXTERNAL_IRQ: {
			u32 irq = plic_hart_claim_irq(0);
			if (irq != 0){
				if (irq == IRQ_SERIAL) {
					serial_irq();
				}
				plic_hart_complete_irq(0, irq);
			}
			break;
		}
		default:
			panic("unknown interrupt %lx :(", scause);
			break;
	}
}

void handle_exception()
{
	u64 scause = csr_read(CSR_SCAUSE);
	u64 stval = csr_read(CSR_STVAL);
	u64 sepc = csr_read(CSR_SEPC);
	// map all exceptions included in trap.h (n sei se igual ou dif do lab01... checar)
	switch (scause) {
		case EXCEPTION_INST_ACCESS_FAULT:
			panic("instruction access fault at 0x%lx :(", stval);
			break;
		case EXCEPTION_LOAD_ACCESS_FAULT:
			panic("load access fault at 0x%lx :(", stval);
			break;
		case EXCEPTION_STORE_ACCESS_FAULT:
			panic("store access fault at 0x%lx :(", stval);
			break;
		case EXCEPTION_INST_PAGE_FAULT:
			panic("instruction page fault at 0x%lx :(", stval);
			break;
		case EXCEPTION_LOAD_PAGE_FAULT:
			panic("load page fault at 0x%lx :(", stval);
			break;
		case EXCEPTION_STORE_PAGE_FAULT:
			panic("store page fault at 0x%lx :(", stval);
			break;
		default:
			panic("unknown exception %lx at 0x%lx :(", scause, sepc);
			break;
	}
}

void trap_setup()
{
	csr_write(CSR_STVEC, (u64)trap_entry);
	csr_clear(CSR_SSTATUS, 	CSR_SSTATUS_SIE); // interrupts disabled until ready !
}

void handle_trap()
{
	u64 scause = csr_read(CSR_SCAUSE);
	if (scause & TRAP_IRQ_BIT) {
		handle_irq();
	} 
	else {
		handle_exception();
	}
}

void hart_irq_enable()
{
	csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE); // enable interrupts
}

u64 hart_irq_save()
{
	u64 old = csr_read_clear(CSR_SSTATUS, CSR_SSTATUS_SIE); // atomic !
	return old & CSR_SSTATUS_SIE; // return just the SIE bit
}

void hart_irq_restore(u64 flags)
{
	csr_set(CSR_SSTATUS, flags);
}

void hart_irq_disable()
{
	csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE); // disable interrupts
}
