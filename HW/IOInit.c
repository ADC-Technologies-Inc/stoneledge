/*
 * IOInit.c
 *
 *  Created on: Apr 5, 2016
 *      Author: zlyzen
 *
 * N. Davidson - 9th May 2018
 * - removed unnecessary global 'private' variables, replaced with stack vars instead
 * - removed unnecessary static declarations in various functions
 * - modified private variables to be declared as const
 * - replaced pin_count calculation with one using % (mod) operator
 * - replaced bit_array with shifts instead
 *
 *
 */

#include "../HW/IOInit.h"
#include "../prog/ntd_debug.h"

// defines
#define CMD_INPUT 0x00
#define CMD_OUTPUT 0x01
#define CMD_INVERT 0x02
#define CMD_CONFIG 0x03


#define NUM_EXT_GPIO_SETS 		0x08
// I2C address of each of the 8 IO expander chips (PCA9557)

#define GPIO100_ADDR 			0x18 		//0b0011000
#define GPIO200_ADDR 			0x1C 		//0b0011100
#define GPIO300_ADDR 			0x1A 		//0b0011010
#define GPIO400_ADDR 			0x1E 		//0b0011110
#define GPIO500_ADDR 			0x19 		//0b0011001
#define GPIO600_ADDR 			0x1D 		//0b0011101
#define GPIO700_ADDR 			0x1B 		//0b0011011
#define GPIO800_ADDR 			0x1F 		//0b0011111

// defines pins as inputs/outputs
#define GPIO_100_INIT_DIR 		0x05 		//0b00000101 // output for status LEDs // 100 and 102 not used
//#define GPIO_200_INIT_DIR 		0xC0 		//0b11000000 // output for status LEDs // 206 and 207 not used;;
#define GPIO_200_INIT_DIR       0x41        //0b01000001 // output for status LEDs // 206 and 200 not used- changed to fix issue, 207 replaced 200
#define GPIO_300_INIT_DIR 		0xFF 		//0b11111111 // input for duty cycle switches
#define GPIO_400_INIT_DIR 	 	0xFF 		//0b11111111 // input for duty cycle switches
//#define GPIO_500_INIT_DIR 	 	0xF0 		//0b11110000 // 500-503 ADC MUX switches (output) 504-507 opto-coupler (input)
#define GPIO_500_INIT_DIR       0x71        //0b01110001 // 500-503 ADC MUX switches (output) 504-507 opto-coupler (input); 507 and 500 switched
#define GPIO_600_INIT_DIR 		0xFF 		//0b00001111 // 600-603 opto-coupler (input) 604-607 not used
#define GPIO_700_INIT_DIR 		0xFF 		//0b00110000 // 700-703 706-707 not used // 704 705 inputs for 10-bit ID
#define GPIO_800_INIT_DIR 		0xFF 		//0b11111111 // 800-807 inputs for 10-bit ID

// sets polarity of pins to non-inverted
#define GPIO_EXT_INIT_POL_INV 	0x00

// sets the outputs to high or low for initial state
#define GPIO_100_INIT_STATE 	0x00 		//0b00000000 // LED outputs all off
#define GPIO_200_INIT_STATE 	0x00 		//0b00000000 // LED outputs all off
#define GPIO_300_INIT_STATE 	0x00 		// GPIO 300 all inputs
#define GPIO_400_INIT_STATE 	0x00 		// GPIO 400 all inputs
#define GPIO_500_INIT_STATE 	0x00 		//0b00000000 // ADC MUX switches low
#define GPIO_600_INIT_STATE 	0x00 		// GPIO 600 all inputs
#define GPIO_700_INIT_STATE 	0x00 		// GPIO 700 all inputs
#define GPIO_800_INIT_STATE 	0x00 		// GPIO 800 all inputs

// private variables
static const uint16_t ext_gpio_addrs[8]    = {GPIO100_ADDR, GPIO200_ADDR, GPIO300_ADDR, GPIO400_ADDR, GPIO500_ADDR, GPIO600_ADDR, GPIO700_ADDR, GPIO800_ADDR};
static const uint16_t ext_gpio_dir[8]   	= {GPIO_100_INIT_DIR, GPIO_200_INIT_DIR, GPIO_300_INIT_DIR, GPIO_400_INIT_DIR, GPIO_500_INIT_DIR, GPIO_600_INIT_DIR, GPIO_700_INIT_DIR, GPIO_800_INIT_DIR};
static const uint16_t ext_gpio_state[8] 	= {GPIO_100_INIT_STATE, GPIO_200_INIT_STATE, GPIO_300_INIT_STATE, GPIO_400_INIT_STATE, GPIO_500_INIT_STATE, GPIO_600_INIT_STATE, GPIO_700_INIT_STATE, GPIO_800_INIT_STATE};

// private function
void ExtGpioReadSet(uint16_t pin_set_, uint16_t* read_buffer_);

// function definitions
void InitGpio_start(void)
{
   EALLOW;

   	   ///////////////////////////////////////////// EPWM1A
	   // Enable an EPWMA1 on GPIO0, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;   // Disable pullup on GPIO0
	   GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // GPIO0 = EPWM1A
	   GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;   // GPIO0 = OUTPUT

	   //GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;


	   ///////////////////////////////////////////// EPWM1B
	   // Enable an EPWM1B on GPIO1, no pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;   // Disable pullup on GPIO1
	   GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;  // GPIO1 = EPWM1B
	   GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;   // GPIO1 = output

	   //GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;

	   ///////////////////////////////////////////// EPWM2A
	   // Enable an EPWM2A on GPIO2, no pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;   // Disable pullup on GPIO2
	   GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;  // GPIO2 = EPWM2A
	   GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;   // GPIO2 = output

	   //GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;

	   ///////////////////////////////////////////// EPWM2B
	   // Enable an EPWM2B on GPIO3, no pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;   // Disable pullup on GPIO3
	   GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;  // GPIO3 = EPWM2B
	   GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;   // GPIO3 = output



	   ///////////////////////////////////////////// EPWM3A
	   // Enable an EPWM3A on GPIO4, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO4 = 1;   // Enable pullup on GPIO4
	   GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;  // GPIO4 = GPIO4
	   GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;   // GPIO4 = output

	   ///////////////////////////////////////////// EPWM3B
	   // Enable an EPWM3B on GPIO5, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO5 = 1;   // disable pullup on GPIO5
	   GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;  // GPIO5 = GPIO5
	   GpioCtrlRegs.GPADIR.bit.GPIO5 = 1;   // GPIO5 = output

	   // Enable an GPIO input on GPIO6, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;   // Enable pullup on GPIO6
	   GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 0;  // GPIO6 = GPIO6
	   GpioCtrlRegs.GPADIR.bit.GPIO6 = 0;   // GPIO6 = input

// GPIO7 - UART_RX
	   /////////////////////////////////////////////
	   //GpioCtrlRegs.GPAPUD.bit.GPIO7 = 0;     // Enable pull-up for GPIO7  (SCIRXDA)
	   //GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 3;   // Asynch input GPIO7 (SCIRXDA)
	   GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;    // Configure GPIO7  for gpio

	   /////////////////////////////////////////////  EPWM5A
	   // Enable an GPIO input on GPIO8, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO8 = 1;   // Enable pullup on GPIO8
	   GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;  // GPIO8 = GPIO8
	   GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;   // GPIO8 = input

	   /////////////////////////////////////////////  EPWM5B
	   // Enable an GPIO input on GPIO9, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO9 = 1;   // Enable pullup on GPIO9
	   GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;  // GPIO9 = GPIO9
	   GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;   // GPIO9 = output


	   /////////////////////////////////////////////  EPWM6B
       GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;   // Enable pullup on GPIO10
       GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;  // GPIO11 = EPMW6A
       GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;   // GPIO11 = output

	   /////////////////////////////////////////////  EPWM6B
	   GpioCtrlRegs.GPAPUD.bit.GPIO11 = 1;   // Enable pullup on GPIO11
	   GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 1;  // GPIO11 = EPMW6B
	   GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;   // GPIO11 = output

	  // GPIO12 - UART_TX
	  /////////////////////////////////////////////
	  // GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0;	   // Enable pull-up for GPIO12 (SCITXDA)
	  GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;   // Configure GPIO12 for SCITXDA operation

	  //
	  GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;
	  GpioCtrlRegs.GPAPUD.bit.GPIO13 = 1;
	  GpioCtrlRegs.GPADIR.bit.GPIO13 = 0;

	  //
	  GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0;
	  GpioCtrlRegs.GPAPUD.bit.GPIO14 = 1;
	  GpioCtrlRegs.GPADIR.bit.GPIO14 = 0;

	  //
	  GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0;
	  GpioCtrlRegs.GPAPUD.bit.GPIO15 = 1;
	  GpioCtrlRegs.GPADIR.bit.GPIO15 = 0;
// GPIO16 - SPI_MOSI

// GPIO17 - SPI_MISO

// GPIO18 - SPI_CLK

// GPIO19 - SPI_CS

	   GpioCtrlRegs.GPAPUD.bit.GPIO16 = 0;   // Enable pull-up on GPIO16 (SPISIMOA)
	   //  GpioCtrlRegs.GPAPUD.bit.GPIO5 = 0;    // Enable pull-up on GPIO5 (SPISIMOA)
	       GpioCtrlRegs.GPAPUD.bit.GPIO17 = 0;   // Enable pull-up on GPIO17 (SPISOMIA)
	   //  GpioCtrlRegs.GPAPUD.bit.GPIO3 = 0;    // Enable pull-up on GPIO3 (SPISOMIA)
	       GpioCtrlRegs.GPAPUD.bit.GPIO18 = 0;   // Enable pull-up on GPIO18 (SPICLKA)
	       GpioCtrlRegs.GPAPUD.bit.GPIO19 = 1;   // Enable pull-up on GPIO19 (SPISTEA)

	   /* Set qualification for selected pins to asynch only */
	   // This will select asynch (no qualification) for the selected pins.
	   // Comment out other unwanted lines.

	       GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = 3; // Asynch input GPIO16 (SPISIMOA)
	   //  GpioCtrlRegs.GPAQSEL1.bit.GPIO5 = 3;  // Asynch input GPIO5 (SPISIMOA)
	       GpioCtrlRegs.GPAQSEL2.bit.GPIO17 = 3; // Asynch input GPIO17 (SPISOMIA)
	   //  GpioCtrlRegs.GPAQSEL1.bit.GPIO3 = 3;  // Asynch input GPIO3 (SPISOMIA)
	       GpioCtrlRegs.GPAQSEL2.bit.GPIO18 = 3; // Asynch input GPIO18 (SPICLKA)
	       //GpioCtrlRegs.GPAQSEL2.bit.GPIO19 = 3; // Asynch input GPIO19 (SPISTEA)

	   /* Configure SPI-A pins using GPIO regs*/
	   // This specifies which of the possible GPIO pins will be SPI functional pins.
	   // Comment out other unwanted lines.

	       GpioCtrlRegs.GPAMUX2.bit.GPIO16 = 1; // Configure GPIO16 as SPISIMOA
	   //  GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 2;  // Configure GPIO5 as SPISIMOA
	       GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 1; // Configure GPIO17 as SPISOMIA
	   //  GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 2;  // Configure GPIO3 as SPISOMIA
	       GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1; // Configure GPIO18 as SPICLKA
	       GpioCtrlRegs.GPAMUX2.bit.GPIO19 = 0; // Configure GPIO19 as GPIO19
	       GpioCtrlRegs.GPADIR.bit.GPIO19 = 0;  // Configure GPIO19 as output



	   // Enable an GPIO input on GPIO20, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO20 = 1;   // Enable pullup on GPIO20
	   GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;  // GPIO20 = GPIO20
	   GpioCtrlRegs.GPADIR.bit.GPIO20 = 0;   // GPIO20 = input

	   // Enable an GPIO input on GPIO21
	   GpioCtrlRegs.GPAPUD.bit.GPIO21 = 1;   // Disable pullup on GPIO21
	   GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;  // GPIO21 = GPIO21
	   GpioCtrlRegs.GPADIR.bit.GPIO21 = 0;   // GPIO21 = input

	   // UART SEL (INPUT, TELLS YOU IF UART IS DIRECTED ON-BOARD OR TO FTDI CHIP) (HIGH FOR USB (FTDI) LOW FOR ON-BOARD)
	   // Enable an GPIO input on GPIO22, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0;   // Enable pullup on GPIO22
	   GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;  // GPIO22 = GPIO22
	   GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;   // GPIO22 = output

	   // Enable an GPIO input on GPIO23, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO23 = 0;   // Enable pullup on GPIO23
	   GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;  // GPIO23 = GPIO23
	   GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;   // GPIO23 = output

	   // Enable an GPIO input on GPIO24, pullup
	   GpioCtrlRegs.GPAPUD.bit.GPIO24 = 1;   // Enable pullup on GPIO24
	   GpioCtrlRegs.GPAMUX2.bit.GPIO24 = 0;  // GPIO24 = GPIO24
	   GpioCtrlRegs.GPADIR.bit.GPIO24 = 0;   // GPIO24 = input

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO25 = 1;   // Enable pullup on GPIO25
	   GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0;  // GPIO25 = GPIO25
	   GpioCtrlRegs.GPADIR.bit.GPIO25 = 0;   // GPIO25 = input

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO26 = 1;
	   GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;
	   GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO27 = 1;
	   GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;
	   GpioCtrlRegs.GPADIR.bit.GPIO27 = 1;

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO28 = 1;
	   GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 0;
	   GpioCtrlRegs.GPADIR.bit.GPIO28 = 1;

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO29 = 1;
	   GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 0;
	   GpioCtrlRegs.GPADIR.bit.GPIO29 = 1;

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO30 = 1;
	   GpioCtrlRegs.GPAMUX2.bit.GPIO30 = 0;
	   GpioCtrlRegs.GPADIR.bit.GPIO30 = 1;

	   //
	   GpioCtrlRegs.GPAPUD.bit.GPIO31 = 1;
	   GpioCtrlRegs.GPAMUX2.bit.GPIO31 = 0;
	   GpioCtrlRegs.GPADIR.bit.GPIO31 = 1;

// GPIO32 - SDA
	   GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;    // Enable pull-up for GPIO28 (SDAA)
	   GpioCtrlRegs.GPBQSEL1.bit.GPIO32 = 3;  // Asynch input GPIO28 (SDAA)
	   GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;   // Configure GPIO28 for SDAA operation

// GPIO33 - SCL
	   GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;	   // Enable pull-up for GPIO29 (SCLA)
	   GpioCtrlRegs.GPBQSEL1.bit.GPIO33 = 3;  // Asynch input GPIO29 (SCLA)
	   GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;   // Configure GPIO29 for SCLA operation

	   // Make GPIO34 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO34 = 1;  // Enable pullup on GPIO34
	   GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0; // GPIO34 = GPIO34
	   GpioCtrlRegs.GPBDIR.bit.GPIO34 = 0;  // GPIO34 = input

	   // TDI
	   // Make GPIO35 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO35 = 1;  // Enable pullup on GPIO35
	   GpioCtrlRegs.GPBMUX1.bit.GPIO35 = 0; // GPIO35 = GPIO35
	   GpioCtrlRegs.GPBDIR.bit.GPIO35 = 0;  // GPIO35 = input

	   // TMS
	   // Make GPIO36 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO36 = 1;  // Enable pullup on GPIO36
	   GpioCtrlRegs.GPBMUX1.bit.GPIO36 = 0; // GPIO36 = GPIO36
	   GpioCtrlRegs.GPBDIR.bit.GPIO36 = 0;  // GPIO36 = input

	   // TDO
	   // Make GPIO37 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO37 = 1;  // Enable pullup on GPIO37
	   GpioCtrlRegs.GPBMUX1.bit.GPIO37 = 0; // GPIO37 = GPIO37
	   GpioCtrlRegs.GPBDIR.bit.GPIO37 = 0;  // GPIO37 = input

// SPI_SD (TCK)
	   // Make GPIO38 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO38 = 1;  // Enable pullup on GPIO38
	   GpioCtrlRegs.GPBMUX1.bit.GPIO38 = 0; // GPIO38 = GPIO38
	   GpioCtrlRegs.GPBDIR.bit.GPIO38 = 0;  // GPIO38 = input

	   // Make GPIO39 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO39 = 1;  // Enable pullup on GPIO38
	   GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0; // GPIO38 = GPIO38
	   GpioCtrlRegs.GPBDIR.bit.GPIO39 = 0;  // GPIO38 = input

	   // Make GPIO40 an output
	   GpioCtrlRegs.GPBPUD.bit.GPIO40 = 1;  // disable pullup on GPIO40
	   GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0; // GPIO40 = GPIO40
	   GpioDataRegs.GPBCLEAR.bit.GPIO40 = 1;
	   GpioCtrlRegs.GPBDIR.bit.GPIO40 = 1;  // GPIO40 = output

	   // Make GPIO41 an input
	   GpioCtrlRegs.GPBPUD.bit.GPIO41 = 1;  // Enable pullup on GPIO38
	   GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0; // GPIO38 = GPIO38
	   GpioCtrlRegs.GPBDIR.bit.GPIO41 = 0;  // GPIO38 = input

	   // Make GPIO42 an output
	   GpioCtrlRegs.GPBPUD.bit.GPIO42 = 1;  // disable pullup on GPIO42
	   GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0; // GPIO42 = GPIO42
	   GpioDataRegs.GPBCLEAR.bit.GPIO42 = 1;
	   GpioCtrlRegs.GPBDIR.bit.GPIO42 = 1;  // GPIO42 = output

	   // Make GPIO43 an output
	   GpioCtrlRegs.GPBPUD.bit.GPIO43 = 1;  // disable pullup on GPIO43
	   GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 0; // GPIO43 = GPIO43
	   GpioDataRegs.GPBCLEAR.bit.GPIO43 = 1;
	   GpioCtrlRegs.GPBDIR.bit.GPIO43 = 1;  // GPIO43 = output
//SPI_CSN
	   // Make GPIO44 an output
	   GpioCtrlRegs.GPBPUD.bit.GPIO44 = 1;  // disable pullup on GPIO44
	   GpioCtrlRegs.GPBMUX1.bit.GPIO44 = 0; // GPIO44 = GPIO44
	   GpioDataRegs.GPBSET.bit.GPIO44 = 1;
	   GpioCtrlRegs.GPBDIR.bit.GPIO44 = 1;  // GPIO44 = output
    EDIS;
}

// initializes GPIO 100-807 to the correct high/low in/out states
void ExtGpioInit(void)
{
    uint16_t ext_gpio_rx_buffer[2];
    uint16_t ext_gpio_buffer[2];
    uint16_t ext_gpio_addr = 0;
	int i = 0;

	while(i < NUM_EXT_GPIO_SETS)
	{

	    //NOTE switched config to be made first before setting output port
		ext_gpio_addr = ext_gpio_addrs[i]; 					// set new address

		//Set port configuration
		ext_gpio_buffer[0] = CMD_CONFIG; 							// command 0x03 in place buffer[0] is configuration input/output read/write
		ext_gpio_buffer[1] = ext_gpio_dir[i]; 				// data of which pins are inputs/outputs
		I2C_Tx(ext_gpio_buffer, 2, ext_gpio_addr);  		// sends pin high/low data
		I2C_Rx(ext_gpio_rx_buffer, 1, ext_gpio_buffer[0], ext_gpio_addr); 	// checks to see if values are correct
		if(ext_gpio_buffer[1] != ext_gpio_rx_buffer[0]) 	// error handling for bad i2c write
		{
			// call error function for bad i2c write to PCA9557 chip
		}

		ext_gpio_buffer[0] = CMD_INVERT; 							// command 0x02 in place buffer[0] is polarity inversion for input pin state data
		ext_gpio_buffer[1] = GPIO_EXT_INIT_POL_INV; 		// data of which pins are inverted (none)
		I2C_Tx(ext_gpio_buffer, 2, ext_gpio_addr); 			// sends polarity inversion data
		I2C_Rx(ext_gpio_rx_buffer, 1, ext_gpio_buffer[0], ext_gpio_addr); 	// checks to see if values are correct
		if(ext_gpio_buffer[1] != ext_gpio_rx_buffer[0]) 	// error handling for bad i2c write
		{
			// call error function for bad i2c write to PCA9557 chip
		}

        ext_gpio_buffer[0] = CMD_OUTPUT;                            // command 0x01 in place buffer[0] is output high/low read/write
        ext_gpio_buffer[1] = ext_gpio_state[i];             // data of which pins are high/low
        I2C_Tx(ext_gpio_buffer, 2, ext_gpio_addr);          // tx's data for gpio output high/low
        I2C_Rx(ext_gpio_rx_buffer, 1, ext_gpio_buffer[0], ext_gpio_addr);   // checks to see if values are correct
        if(ext_gpio_buffer[1] != ext_gpio_rx_buffer[0])     // error handling for bad i2c write
        {
            // call error function for bad i2c write to PCA9557 chip
        }

		i++;
	}
}

void ExtGpioSet(uint16_t pin_, uint16_t set_)
{
    uint16_t ext_gpio_rx_buffer[2];

	uint16_t pin_number;
	uint16_t accm;

#ifdef DEBUG_IOINIT
	uint16_t read1, read2, written;
#endif

	if(pin_ > 807)
		accm = 2;
	accm = 0;
	pin_number = 0;

	accm = pin_/100;
	pin_number = pin_ % 100;

	ExtGpioReadSet(accm, ext_gpio_rx_buffer);
#ifdef DEBUG_IOINIT
	read1 = ext_gpio_rx_buffer[0];
#endif

	if(set_) 													// if request is to set pin high
	{
		ext_gpio_rx_buffer[0] |= (1 << pin_number);/*bit_arrays[pin_number];*/ 		// set pin high
	}
	else 														// if request is to set pin low
	{
		ext_gpio_rx_buffer[0] &= ~(1 << pin_number);/*(bit_arrays[pin_number]);*/ 	// set pin low
	}
#ifdef DEBUG_IOINIT
	written = ext_gpio_rx_buffer[0];
#endif
	GpioSetDirArray(accm, ext_gpio_rx_buffer[0]); 				// write new high/low array to chip


	ExtGpioReadSet(accm, ext_gpio_rx_buffer);
#ifdef DEBUG_IOINIT
	read2 = ext_gpio_rx_buffer[0];
#endif

#ifdef DEBUG_IOINIT
    printf("ExtGpioSet:: Pin: %d; Set %d; accm: "PRINTF_BINSTR8"; buffer: "PRINTF_BINSTR8" read1: "PRINTF_BINSTR8"; read2: "PRINTF_BINSTR8"; written: "PRINTF_BINSTR8"\n", pin_, set_, PRINTF_BINSTR8_ARGS(accm), PRINTF_BINSTR8_ARGS(ext_gpio_rx_buffer[0]),PRINTF_BINSTR8_ARGS(read1), PRINTF_BINSTR8_ARGS(read2), PRINTF_BINSTR8_ARGS(written) );
	if (written != read2){
	    printf( "ExtGpioSet:: ALARM\n");
	}
#endif

}

void GpioSetDirArray(uint16_t pin_array_, uint16_t array_)
{
    uint16_t ext_gpio_addr;
    uint16_t ext_gpio_buffer[2];
    uint16_t ext_gpio_rx_buffer[2];

    //save state of pin_array so we can re-init if necessary
    ext_gpio_state[pin_array_ - 1] = array_;

	ext_gpio_addr = ext_gpio_addrs[pin_array_-1];			// sets address
	ext_gpio_buffer[0] = CMD_OUTPUT; 						// command 0x01 in place buffer[0] is output high/low read/write
	ext_gpio_buffer[1] = array_; 							// new high/low pin data
	I2C_Tx(ext_gpio_buffer, 2, ext_gpio_addr); 				// send new high/low pin data
	I2C_Rx(ext_gpio_rx_buffer, 1, ext_gpio_buffer[0], ext_gpio_addr); 		// read it back to make sure write was successful
	if(ext_gpio_buffer[1] != ext_gpio_rx_buffer[0]) 		// check for same data
	{
		// call error function for bad i2c write to PCA9557 chip
#ifdef DEBUG_IOINIT
	    printf("GpioSetDirArray:: Bad Write\n");
#endif
	}
}

// private function
void ExtGpioReadSet(uint16_t pin_set_, uint16_t* read_buffer_)
{
    uint16_t ext_gpio_addr;

	ext_gpio_addr = ext_gpio_addrs[pin_set_ - 1];
	I2C_Rx(read_buffer_, 1, CMD_INPUT, ext_gpio_addr); 						// reads output port data to read_buffer (we just want to know if we set it right, right?)
}

// send a command such as 2 to get all pin states for GPIO 200-207
uint16_t ExtGpioGetSet(uint16_t pin_set_)
{
    uint16_t ext_gpio_rx_buffer[2];

	ExtGpioReadSet(pin_set_, ext_gpio_rx_buffer );
	return ext_gpio_rx_buffer[0];
}

// send a request such as 206 to read the state of GPIO 206 and it will return a 1 or 0 depending on the state of the pin
uint16_t ExtGpioRead(uint16_t pin_)
{
    uint16_t ext_gpio_rx_buffer[2];
	uint16_t pin_number;
	uint16_t accm;

	accm = pin_/100;
    pin_number = pin_ % 100;

	ExtGpioReadSet(accm, ext_gpio_rx_buffer);
	if( (1 << pin_number) & ext_gpio_rx_buffer[0])
		return 1;
	else
		return 0;

}



