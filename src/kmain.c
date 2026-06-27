#include <kernel/printf.h>
#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h>

extern int _hartid[];

static void execute(char *line){
	if (line[0] == '\0') {
		return;
	}
	else if (strcmp(line, "uptime") == 0){
		char out[32];
		snprintf(out, sizeof(out), "%lus\n", (timer_read() / TIMER_FREQ));
		serial_puts(out);
	} 
	else if (strncmp(line, "echo ", 5) == 0){
		serial_puts(line + 5);
		serial_puts("\n");
	} 
	else if (strncmp(line, "alarm ", 6) == 0){
		u64 secs = strtou64(line + 6, 10);
		timer_set_alarm(secs);
	}
	else{
		serial_puts("unknown command :(\n");
	}
}


void kmain()
{
	printk_set_level(LOG_DEBUG);
	info("entered S-mode\n");
	info("booting on hart %d\n", _hartid[0]);
	info("setting up virtual memory...\n");
	vm_init();

	info("enabling traps...\n");
	trap_setup();
	info("enabling timer...\n");
	timer_irq_enable();
	info("enabling serial...\n");
	serial_init();
	serial_irq_enable();
	hart_irq_enable();


	serial_puts("> ");
char line[256];
size_t pos = 0;
char tmp[SERIAL_BUFFER_SIZE];

while (1) {
    size_t n = serial_read(tmp);
    for (size_t i = 0; i < n; i++) {
        char c = tmp[i];
        if (c == '\r' || c == '\n') {
            serial_puts("\n");
            line[pos] = '\0';
            execute(line);
            pos = 0;
            serial_puts("> ");
        } else {
            serial_putc(c);
            if (pos < sizeof(line) - 1)
                line[pos++] = c;
        }
    }
}
}
