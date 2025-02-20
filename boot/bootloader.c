extern void setup_vbar();

void boot_main() {
    setup_vbar();  // Set the interrupt vector table
}