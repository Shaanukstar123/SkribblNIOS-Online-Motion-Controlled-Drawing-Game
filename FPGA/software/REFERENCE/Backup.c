//NIOS Imports
#include "system.h"
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"

//Standard Imports
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Time Import
#include "sys/alt_timestamp.h"
#define SAMPLING_TIME 3
#define INTERVALSECOND 100000000

//Variable Declerations
//UART
FILE* fp;

//LED
alt_u8 led;
int level;

//Timer
alt_u32 startTime, stopTime;

void led_write(alt_u8 led_pattern) {
    IOWR(LED_BASE, 0, led_pattern);
}

int main () {
	//Accelerometer setup
	alt_32 x_read;
	alt_32 y_read;
	alt_32 z_read;
	alt_up_accelerometer_spi_dev * acc_dev;
	acc_dev = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi");
	if (acc_dev == NULL) {
		// if return 1, check if the spi ip name is "accelerometer_spi"
		return 1;
	}
	//Open file for reading and writing
	char dataIn = 0;
	fp = fopen ("/dev/jtag_uart", "r+");

	if (fp)	{
		//Begin
		led_write(0x7);
		alt_timestamp_start();
		startTime = alt_timestamp();
		// Loop until KILL command
		//Comparing strings in the usual way compares the pointers.
		while (dataIn != 'k') {
			// Get data/command from Python interface
			led_write(0x60);
			dataIn = getc(fp);
			printf(dataIn);
			// Print a message if character is 't'.
			/*
			if (dataIn == 't') {
				fwrite (msg, strlen (msg), 1, fp);
				alt_printf("Got t\n");
			}
			*/
			//If file write fails
			if (ferror(fp)) {// Check if an error occurred with the file
				clearerr(fp);// If so, clear it.
			}
			//Obtain values at a certain frequency
			stopTime = alt_timestamp();
			//Frequency of accelerometer is 2^SAMPLING_TIME Hz, with 6, 64Hz
			if ((stopTime-startTime) > (INTERVALSECOND >> SAMPLING_TIME-1)) {
				alt_up_accelerometer_spi_read_x_axis(acc_dev, & x_read);
				alt_up_accelerometer_spi_read_y_axis(acc_dev, & y_read);
				alt_up_accelerometer_spi_read_z_axis(acc_dev, & z_read);
				printf("%d %d %d\n", x_read, y_read, z_read);
				startTime = alt_timestamp();
			}
		}
		printf("Killed.\n");
		led_write(0x1);
		fclose (fp);
	}
	return 0;
}
