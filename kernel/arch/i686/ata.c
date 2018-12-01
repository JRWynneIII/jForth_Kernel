#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/system.h>
#include <kernel/keyboard.h>
#include <kernel/ata.h>

//Ports
#define PIO_IO_BASE_PORT 0x1F0
#define PIO_CONTROL_BASE_PORT 0x3F6
#define PIO_ALT_IO_BASE_PORT 0x170
#define PIO_ALT_CONTROL_BASE_PORT 0x376

//Registers
#define PIO_DATA_REGISTER PIO_IO_BASE_PORT + 0
#define PIO_ERROR_REGISTER PIO_IO_BASE_PORT + 1
#define PIO_FEATURES_REGISTER PIO_IO_BASE_PORT + 1
#define PIO_SECTOR_COUNT_REGISTER PIO_IO_BASE_PORT + 2
#define PIO_SECTOR_NUMBER_REGISTER PIO_IO_BASE_PORT + 3
#define PIO_LBA_LO PIO_IO_BASE_PORT + 3
#define PIO_CYLINDER_LOW_REGISTER PIO_IO_BASE_PORT + 4
#define PIO_LBA_MID PIO_IO_BASE_PORT + 4
#define PIO_CYLINDER_HIGH_REGISTER PIO_IO_BASE_PORT + 5
#define PIO_LBA_HI PIO_IO_BASE_PORT + 5
#define PIO_DRIVE_HEAD_REGISTER PIO_IO_BASE_PORT + 6
#define PIO_STATUS_REGISTER PIO_IO_BASE_PORT + 7
#define PIO_COMMAND_REGISTER PIO_IO_BASE_PORT + 7
#define PIO_ALT_STATUS_REGISTER PIO_CONTROL_BASE_PORT + 0
#define PIO_DEVICE_CONTROL_REGISTER PIO_CONTROL_BASE_PORT + 0
#define PIO_DRIVE_ADDRESS_REGISTER PIO_CONTROL_BASE_PORT + 1

// Secondary registers
#define PIO_SECONDARY_DATA_REGISTER PIO_ALT_IO_BASE_PORT + 0
#define PIO_SECONDARY_ERROR_REGISTER PIO_ALT_IO_BASE_PORT + 1
#define PIO_SECONDARY_FEATURES_REGISTER PIO_ALT_IO_BASE_PORT + 1
#define PIO_SECONDARY_SECTOR_COUNT_REGISTER PIO_ALT_IO_BASE_PORT + 2
#define PIO_SECONDARY_SECTOR_NUMBER_REGISTER PIO_ALT_IO_BASE_PORT + 3
#define PIO_SECONDARY_LBA_LO PIO_ALT_IO_BASE_PORT + 3
#define PIO_SECONDARY_CYLINDER_LOW_REGISTER PIO_ALT_IO_BASE_PORT + 4
#define PIO_SECONDARY_LBA_MID PIO_ALT_IO_BASE_PORT + 4
#define PIO_SECONDARY_CYLINDER_HIGH_REGISTER PIO_ALT_IO_BASE_PORT + 5
#define PIO_SECONDARY_LBA_HI PIO_ALT_IO_BASE_PORT + 5
#define PIO_SECONDARY_DRIVE_HEAD_REGISTER PIO_ALT_IO_BASE_PORT + 6
#define PIO_SECONDARY_STATUS_REGISTER PIO_ALT_IO_BASE_PORT + 7
#define PIO_SECONDARY_COMMAND_REGISTER PIO_ALT_IO_BASE_PORT + 7
#define PIO_SECONDARY_ALT_STATUS_REGISTER PIO_ALT_CONTROL_BASE_PORT + 0
#define PIO_SECONDARY_DEVICE_CONTROL_REGISTER PIO_ALT_CONTROL_BASE_PORT + 0
#define PIO_SECONDARY_DRIVE_ADDRESS_REGISTER PIO_ALT_CONTROL_BASE_PORT + 1

//Hacky/bad. Fix this TODO
uint16_t DRIVE_DATA[256];


// If the bus is floating then no disks exist or forgot to flush the cache
bool ata_floating_bus_check(char reg) {
	uint16_t status = inb(reg);
	if (status == 0xFF)
		return true;
	else
		return false;
}

char ata_read_status() {
	char output = NULL;
	// NOTE: Must read from the status port 5 times to introduce a 400ns
	// delay. The values before then will be possibly incorrect
	for (int i = 0; i < 5; i++)
		output = inb(PIO_STATUS_REGISTER);
	return output;
}

char ata_identify() {
	outb(0xA0, PIO_DRIVE_ADDRESS_REGISTER);
	outb(0xB0, PIO_DRIVE_ADDRESS_REGISTER); //If slave on this port

	outb(0x00, PIO_SECTOR_COUNT_REGISTER);
	outb(0x00, PIO_LBA_LO);
	outb(0x00, PIO_LBA_MID);
	outb(0x00, PIO_LBA_HI);
	
	outb(0xEC, PIO_COMMAND_REGISTER);
	
	char status = ata_read_status();

	if (status != 0) {
		char cur = inb(PIO_STATUS_REGISTER);
		while((1 << 7) & cur) { //Poll the status port until BSY (7th bit) is unset
			cur = inb(PIO_STATUS_REGISTER);
		}

		char lba_mid = inb(PIO_LBA_MID);
		char lba_hi = inb(PIO_LBA_HI);

		// If drive is an ATA drive, then both lba_mid and lba_hi are 0
		if (lba_mid == 0 && lba_hi == 0) {
			while(!((1 << 3) & cur)) { 
				cur = inb(PIO_STATUS_REGISTER);
				if ((1 << 0) & cur) //If ERR bit is set
					return; //TODO: DO SOMETHING HERE
			}
			
			// Read 256 16-bit values from the data port
			for(int i = 0; i < 256; i++)
				DRIVE_DATA[i] = inb(PIO_DATA_REGISTER);
		}

	}

}

char ATA_Init() {
	char exists_floating_drives = ata_floating_bus_check(PIO_STATUS_REGISTER);
	char exists_floating_secondary_drives = ata_floating_bus_check(PIO_SECONDARY_STATUS_REGISTER);

	ata_identify();
}
