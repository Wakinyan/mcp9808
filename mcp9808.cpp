/**************************************************************************/
/*! 
    @file     mcp9808.cpp
    @author   GALLER Florian
	@license  CC0
	@doc_lang German
	
	I2C Driver for Microchip's MCP9808 I2C Temp sensor für den RPi2 
	http://www.microchip.com/MCP9808 (21.12.2015)
	http://ww1.microchip.com/downloads/en/DeviceDoc/25095A.pdf (21.12.2015)
	
	Output with printf

    alpha 1.0 - startup
*/
/**************************************************************************/

/***********************************TODO***********************************/
/*! 
	MCP9808 Einstellungen
	--> Temperaturbegrenzung
	--> Manufacure ID
	--> Device ID
	--> Sonstiges: siehe Datenblatt
	--> Resolution setting
*/
/***********************************TODO***********************************/

#include <unistd.h> //muss nicht extra miteinbezogen werden
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h> //SMBus - Protokoll | Packet Error Code (PEC) not implemented? p27 smbus v2.0
#include <sys/ioctl.h>
#include <sys/types.h> //muss nicht extra miteinbezogen werden
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

/**************************************************************************/
/*! 
	Default Werte *evtl in Config auslagern / eigenen Header*
*/
/**************************************************************************/
#define I2C_SMBUS_PATH				   "/dev/i2c-1" //I2C Bus Datei (DEBIAN Wheezy - RPi2)

#define MCP9808_I2CADDR_DEFAULT        0x18 //Standard I2C Adresse
#define MCP9808_REG_CONFIG             0x01 //Konfiguration Register-Adresse (Achtung Reihenfolge (MSB-LSB) 7-0 & 15-8)
	#define MCP9808_REG_CONFIG_HYST_10	   0x0004 //bit 10	 Hysterese
	#define MCP9808_REG_CONFIG_HYST_9	   0x0002 //bit 9	 Hysterese
	#define MCP9808_REG_CONFIG_SHUTDOWN    0x0001 //bit8 Shutdown Mode - Standy - low power consumation ~0,1 µA
	#define MCP9808_REG_CONFIG_CRITLOCKED  0x8000 //bit7 kritische Temperatur sperre
	#define MCP9808_REG_CONFIG_WINLOCKED   0x4000 //bit6 obere und untere Temperatur sperre
	#define MCP9808_REG_CONFIG_INTCLR      0x2000 //bit5 "Clear interrupt output"
	#define MCP9808_REG_CONFIG_ALERTSTAT   0x1000 //bit4 "Alert Output Status Bit"
	#define MCP9808_REG_CONFIG_ALERTCTRL   0x0800 //bit3 "Alert Output control"
	#define MCP9808_REG_CONFIG_ALERTSEL    0x0400 //bit2 "Alert Output set" 1=critical Temp only
	#define MCP9808_REG_CONFIG_ALERTPOL    0x0200 //bit1 "Alert Output Polarity"
	#define MCP9808_REG_CONFIG_ALERTMODE   0x0100 //bit0 "Alert Output Mode"
#define MCP9808_REG_UPPER_TEMP         0x02 //obere Temperaturbegrenzung - upper Temperature Limit
#define MCP9808_REG_LOWER_TEMP         0x03 //untere Temperaturbegrenzung - lower Temperature Limit
#define MCP9808_REG_CRIT_TEMP          0x04 //kritische Abgrenzung - critical Temperature Limit
#define MCP9808_REG_AMBIENT_TEMP       0x05 //Temperatur auslese Register (Read-Only)
#define MCP9808_REG_MANUF_ID           0x06 //Hersteller ID Register
#define MCP9808_REG_DEVICE_ID          0x07 //Geräte ID Register
#define MCP9808_REG_RESOLUTION		   0x08 //Auflösungs Register
	#define MCP9808_REG_RESOLUTION_0       0x00 // 0,5K    - ADC time 30ms
	#define MCP9808_REG_RESOLUTION_1       0x01 // 0,25K   - ADC time 65ms
	#define MCP9808_REG_RESOLUTION_2       0x02 // 0,125K  - ADC time 130ms
	#define MCP9808_REG_RESOLUTION_3       0x03 // 0,0625K - ADC time 250ms
	
void functions(int device) {
	unsigned long funcs; //als 0 festgelegt - 32bit 0x00

//	printf("\nChecking for MCP9808 required I2C functions:\n");
	
	if (ioctl(device,I2C_FUNCS,&funcs) < 0) // überprüfen der I2C (SMBus) Standard Funktionen
    {
		perror("ioctl() I2C_FUNCS failed");
		exit (1);
    }
		
//	if (funcs & I2C_FUNC_I2C) { // 0 wahr //Wenn I2C Bus Spezifikationen vorhanden
//		printf(" - I2C\n");
//	}
//	if (funcs & (I2C_FUNC_SMBUS_BYTE)) { //0 wahr //Wenn SMBUS Byte Spezifikation vorhanden
//		printf(" - I2C_FUNC_SMBUS_BYTE\n");
//	}
//	if (funcs & (I2C_FUNC_SMBUS_WORD_DATA)) {
//		printf(" - I2C_FUNC_SMBUS_WORD_DATA\n"); 
//	}
//	if (funcs & (I2C_FUNC_SMBUS_BYTE_DATA)) {//0 wahr //Wenn SMBUS Byte Spezifikation vorhanden
//		printf(" - I2C_FUNC_SMBUS_BYTE_DATA\n");
//	}
}

/* Scanne alle I2C SMBUS Adressen */
void scan_i2c_bus(int device) {
	int address, reg0;
	
//	printf("\nChecking SMBus for Slaves\n");
	for (address = 0; address < 127; address++) { //7bit Adressen!
		if (ioctl(device, I2C_SLAVE, address) < 0) {
			perror("ioctl() I2C_SLAVE failed/n");
		} else {
			reg0 = i2c_smbus_read_byte(device);
			if (reg0 >= 0) { //ist kein Device angeschlossen gibt die Funktion -1 zurück.
//				printf("found i2c device at: %x\n", address);
			}
		}
	}
}
	
int open_BUS_device() {
	int device; //File Descriptor
	
//	printf("Opening SMBus (%s) ... ", I2C_SMBUS_PATH);
	if ((device = open(I2C_SMBUS_PATH, O_RDWR)) < 0 ) {
		perror("open() failed");
		exit(1);
	}
//	printf("OK\n");
	scan_i2c_bus(device); //notwendig?? evlt zum überprüfen ob richtige Adresse
	functions(device); //funktionen Prüfen
	return device;
}

void set_adress(int device) {
//	printf("\nSet address %x for MCP9808 ... ", MCP9808_I2CADDR_DEFAULT); 
	if (ioctl(device, I2C_SLAVE, MCP9808_I2CADDR_DEFAULT) < 0) {
// 	printf("Address could not be set.\n");
    exit(1);
	}
//	printf("OK\n");

}

/* liest die aktuelle Temperaur aus und gibt diese in °C aus */
float a_temp(void) { //evtl später als Rückgabewert???
	
	int device = open_BUS_device(); //Bus File öffnen
	
	int temp_b[2]; 	// Array Index: LSB=0, MSB=1
	float temp_c; 	// Temperatur in ° Celsius
	
	set_adress(device);
	
	// Verwende i2c-dev.h
	temp_b[0] = i2c_smbus_read_word_data(device, MCP9808_REG_AMBIENT_TEMP);
	if (temp_b[0] < 0 ) { //notwendig, da ansonsten 9°C ausgegeben wird.
		perror("i2c_smbus_read_word_data() failed");
		exit(1);
	}
	
	
	/* -------------------------------------------------------------------------------- */
	/* Nach den SMBus Spezifikationen liest das Programm zuerst LSB (Data Byte Low) und */
	/* dann das MSB (Data Byte High) aus. Der MCP9808 speichert im MSB bit 8-15 und im 	*/
	/* LSB bit 0-7. Die Ausgabe nach SMBus erfolgt in folgender Reihenfolge: (MSB-LSB)	*/ 
	/* 									bit: 7-0 & 15-8									*/
	/* -------------------------------------------------------------------------------- */
	
	/* Bedeutung der Bits
		bit 00 = 2^-4 °C
		bit 01 = 2^-3 °C
		...
		bit 10 = 2^6  °C
		bit 11 = 2^7  °C
		bit 12 = Vorzeichen (0 >= 0°C, 1 < 0°C)
		bit 13 - 15 geben Temperaturbegrenzungen an (flag bits).
	*/
	
	//aufteilen in Upper (MSB) und Lower (LSB)
	temp_b[1] = temp_b[0] & 0x00FF;	
	temp_b[0] = (temp_b[0] & 0xFF00)/0xFF;

	//evlt Flag Bits auslesen implementieren....//
	
	temp_b[1] = temp_b[1] & 0x1F; //clear FlagBits
	
	if ((temp_b[1] & 0x10)==0x10) { //Vorzeichen Prüfen: -°C
		temp_b[1] = temp_b[1] & 0x0F; 	// clear SignBit
		
		temp_c = 265 - (temp_b[1] * 16 + static_cast<float>(temp_b[0]) / 16); //Umwandlung von bit in °C
	} else {		// +°C
		temp_c = (temp_b[1] * 16 + static_cast<float>(temp_b[0]) / 16);
	}
	
//	printf("temperature: %f °C\n", temp_c); als Rückgabewert?
	return temp_c;
}

/* bit bedeutung: (10 einstellbare Bits)
	nicht (0 oder 1) = belassen
	0 = auf 0 setzen
	1 = auf 1 setzte */
	
int settings_set (int bit[]) { // Array muss 10 groß sein 
	int device = open_BUS_device(); //Bus File öffnen
	set_adress(device);
	
	int data; // Daten (2 Byte - 16 Bit)
	
	// i2c-dev.h
	data = i2c_smbus_read_word_data(device, MCP9808_REG_CONFIG); //die Konfiguration muss zuerst ausgelesen werden, da alle nicht veränderten Werte gleich gesetzt werden sollen...
	if (data < 0 ) { //notwendig, da ansonsten ein ungültiger Wert ausgegeben wird.
		perror("i2c_smbus_read_word_data() failed");
		exit(1);
	}
	
	/* Reihenfolge: (MSB-LSB) 7-0 & 15-8 beachten! */
	
	// Alert Output Mode
	if (bit[0] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_ALERTMODE);
	} else if (bit[0] == 1) {
		data = data | MCP9808_REG_CONFIG_ALERTMODE;
	}

	// Alert Output Polarity
	if (bit[1] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_ALERTPOL);
	} else if (bit[1] == 1) {
		data = data | MCP9808_REG_CONFIG_ALERTPOL;
	}
	
	// Alert Output Set
	if (bit[2] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_ALERTSEL);
	} else if (bit[2] == 1) {
		data = data | MCP9808_REG_CONFIG_ALERTSEL;
	}

	// Alert Output Control
	if (bit[3] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_ALERTCTRL);
	} else if (bit[3] == 1) {
		data = data | MCP9808_REG_CONFIG_ALERTCTRL;
	}

	// Alert Output Status
	if (bit[4] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_ALERTSTAT);
	} else if (bit[4] == 1) {
		data = data | MCP9808_REG_CONFIG_ALERTSTAT;
	}
	
	// Interrupt Clear
	if (bit[5] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_INTCLR);
	} else if (bit[5] == 1) {
		data = data | MCP9808_REG_CONFIG_INTCLR;
	}

	// Windows Lock (Lock Tupper and Tlower Register)
	if (bit[6] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_WINLOCKED);
	} else if (bit[6] == 1) {
		data = data | MCP9808_REG_CONFIG_WINLOCKED;
	}

	// Critical Lock (Lock Tcritical Register)
	if (bit[7] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_CRITLOCKED);
	} else if (bit[7] == 1) {
		data = data | MCP9808_REG_CONFIG_CRITLOCKED;
	}

	// Schutdown Mode - setzt das Shutdown (PowerSaving Bit) ~0,1 µA
	if (bit[8] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_SHUTDOWN);
	} else if (bit[8] == 1) {
		data = data | MCP9808_REG_CONFIG_SHUTDOWN;
	}

	// Hysterese Bit 2
	if (bit[9] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_HYST_9);
	} else if (bit[9] == 1) { 
		data = data | MCP9808_REG_CONFIG_HYST_9;
	}

	// Hysterese Bit 1
	if (bit[10] == 0) {
		data = ~(~data | MCP9808_REG_CONFIG_HYST_10); 
	} else if (bit[10] == 1) { 
		data = data | MCP9808_REG_CONFIG_HYST_10; 
	}
	
	// i2c-dev.h
	data = i2c_smbus_write_word_data(device, MCP9808_REG_CONFIG, data); // Daten auf den Sensor schreiben
	if (data < 0 ) { //notwendig, da ansonsten ein ungültiger Wert ausgegeben wird.
		perror("i2c_smbus_write_word_data() failed");
		exit(1);
	}
	
	return 0;
}

/* liest die Settings aus */ 
int settings_read (void) {
	int device = open_BUS_device(); //Bus File öffnen
	set_adress(device);
	
	int data; // Daten (2 Byte - 16 Bit)
	
	// i2c-dev.h
	data = i2c_smbus_read_word_data(device, MCP9808_REG_CONFIG); 
	if (data < 0 ) { //notwendig, da ansonsten ein ungültiger Wert ausgegeben wird.
		perror("i2c_smbus_read_word_data() failed");
		exit(1);
	}
	
	/* Reihenfolge: (MSB-LSB) 7-0 & 15-8 */
	
	return data;
}

void settings_help(void) {
	printf("\nTry one of these commands (for further information look at the MCP9808 Datasheet):\n \
		settings (show/help/set) [alert-mode/] [...] \n\n \
		show	| Displays sensor info \n \
		help	| this help \n \
		set 	| set sensor settings \n\n \
		alert-mode	| --> comperator/interrupt \n \
		alert-polarity	| --> low/high	\n \
		alert-select	| --> all/critical \n \
		alert		| --> disable/enable \n \
		window-lock	| --> disable/enable \n \
		critical-lock	| --> disable/enable \n \
		shutdown	| --> no/yes (PowerSaving Mode ~0,1 µA) \n \
		hyst		| --> off/1.5/3/6 \n \
		\n");
}

void settings (int argc, char* argv[], int argument, int debug) {
	
	if (debug == 1) {
		printf("\nEntering settings function: ...\nargc: %i \nargument %i\n", argc, argument);
	}
	
	int data;
	
	if (argv[argument] == 0) { //überprüfe ob noch ein Argument in argv vorhanden ist... --> leer rufe help auf
		settings_help(); //help der Settings ausgeben
		return;
		
	} else if (!strcmp(argv[argument], "show")) { // liest einstellungen und gibt diese aus (oder zurück - implementieren...)
		argument++;
		printf("\nReading device settings: ...");
		
		data = settings_read();
		
		if (debug == 1) {
			printf("data: %x \n", data);
		}
		
		/* Reihenfolge: (MSB-LSB) 7-0 & 15-8 */
		
		/*Bit0 Alert Output Mode*/
		printf("\n  Alert Output Mode........: ");
		if (data & MCP9808_REG_CONFIG_ALERTMODE) { //wenn gleich 0 dann falsch
			printf("Interrupt output");
		} else {
			printf("Comparator output (power-up default)");
		}
		
		/*Bit1 Alert Output Polarity*/
		printf("\n  Alert Output Polarity....: ");
		if (data & MCP9808_REG_CONFIG_ALERTPOL) { 
			printf("Active-high");		
		} else {
			printf("Active-low (power-up default)");
		}
		
		/*Bit2 Alert Output Select*/
		printf("\n  Alert Output Select......: ");
		if (data & MCP9808_REG_CONFIG_ALERTSEL) { 
			printf("T_A > T_CRIT only");
		} else {
			printf("Alert output for T_UPPER, T_LOWER and T_CRIT (power-up default)");
		}

		/*Bit3 Alert Output Control*/
		printf("\n  Alert Output.............: ");
		if (data & MCP9808_REG_CONFIG_ALERTCTRL) { 
			printf("ON");
		} else {
			printf("OFF (power-up default)");
		}		
		
		/*Bit4 Alert Output Status*/
		printf("\n  Alert Output Status......: ");
		if (data & MCP9808_REG_CONFIG_ALERTSTAT) { 
			printf("asserted as a comparator/Interrupt or critical temperature output");
		} else {
			printf("not asserted by the device (power-up default)");
		}
		
		/*Bit5 Interrupt Clear*/
		printf("\n  Interrupt Clear..........: ");
		if (data & MCP9808_REG_CONFIG_INTCLR) { 
						printf("Clear interrupt output");
		} else {
			printf("No effect (power-up default)");
		}
		
		/*Bit6 obere und untere Temperatur Sperre*/
		printf("\n  temperature window lock..: ");
		if (data & MCP9808_REG_CONFIG_WINLOCKED) { 
			printf("Locked (unlock with Power-ON Reset)");
		} else {
			printf("Unlocked (power-up default)");
		}
		
		/*Bit7 kritische Temperatur Sperre*/
		printf("\n  critical temperature lock: ");
		if (data & MCP9808_REG_CONFIG_CRITLOCKED) {
			printf("Locked (unlock with Power-ON Reset)");
		} else {
			printf("Unlocked (power-up default)");
		}
		
		/*Bit8 Shutdown Mode*/
		printf("\n  Shutdown Mode............: ");
		if (data & MCP9808_REG_CONFIG_SHUTDOWN) { //wenn gleich 0 
			printf("ON (low Power Mode ~0,1 µA)");
		} else {
			printf("OFF (power-up default)");
		}
		
		/*Bit9 und 10 Hysterese*/
		printf("\n  Hysteresis limit.........: ");
		if (data & MCP9808_REG_CONFIG_HYST_10) { //wenn gleich 0 
			if (data & MCP9808_REG_CONFIG_HYST_9) {
				printf("+6°C");
			} else {
				printf("+3°C");
			}
		} else {
			if (data & MCP9808_REG_CONFIG_HYST_9) {
				printf("+1,5°C");
			} else {
				printf("0°C (power-up default)");
			}
		}
		printf("\n\n");
		return;	
			
	} else if (!strcmp(argv[argument], "set")) { //Schreibt Einstellungen
		argument++;
		
		int settings[11] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // Setze alle Veränderungen auf gleichbleibend
	
		if (argv[argument] == 0) {
			settings_help(); //help der Settings ausgeben
			return;
		} else {			
			if (argv[argument + 1] == 0) {
				settings_help(); //help der Settings ausgeben
			return;
			}
		}
		
		if (!strcmp(argv[argument], "alert-mode")) {
			if (!strcmp(argv[argument + 1], "comperator")) {
				settings[0] = 0;
			} else if (!strcmp(argv[argument + 1], "interrupt")) {
				settings[0] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else if (!strcmp(argv[argument], "alert-polarity")) {
			if (!strcmp(argv[argument + 1], "low")) {
				settings[1] = 0;
			} else if (!strcmp(argv[argument + 1], "high")) {
				settings[1] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else if (!strcmp(argv[argument], "alert-select")) {
			if (!strcmp(argv[argument + 1], "all")) {
				settings[2] = 0;
			} else if (!strcmp(argv[argument + 1], "critical")) {
				settings[2] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else if (!strcmp(argv[argument], "alert")) {
			if (!strcmp(argv[argument + 1], "disable")) {
				settings[3] = 0;
			} else if (!strcmp(argv[argument + 1], "enable")) {
				settings[3] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
/* ---- Funkion von bit 4 und 5 nicht klar!???? ---- */

		} else if (!strcmp(argv[argument], "window-lock")) {
			if (!strcmp(argv[argument + 1], "disable")) {
				settings[6] = 0;
			} else if (!strcmp(argv[argument + 1], "enable")) {
				settings[6] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else if (!strcmp(argv[argument], "critical-lock")) {
			if (!strcmp(argv[argument + 1], "disable")) {
				settings[7] = 0;
			} else if (!strcmp(argv[argument + 1], "enable")) {
				settings[7] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else if (!strcmp(argv[argument], "shutdown")) {
			if (!strcmp(argv[argument + 1], "no")) {
				settings[8] = 0;
			} else if (!strcmp(argv[argument + 1], "yes")) {
				settings[8] = 1;			
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else if (!strcmp(argv[argument], "hyst")) {
			if (!strcmp(argv[argument + 1], "off")) {
				settings[10] = 0;
				settings[9] = 0;
			} else if (!strcmp(argv[argument + 1], "1.5")) {
				settings[10] = 0;
				settings[9] = 1;
			} else if (!strcmp(argv[argument + 1], "3")) {
				settings[10] = 1;
				settings[9] = 0;	
			} else if (!strcmp(argv[argument + 1], "6")) {
				settings[10] = 1;
				settings[9] = 1;					
			} else {
				settings_help(); //help der Settings ausgeben
				return;
			}
		} else {
			settings_help(); //help der Settings ausgeben
			return;
		}
		
		settings_set(settings);
		
		return;
	
	} else if (!strcmp(argv[argument], "reset")) {
		printf("Some settings may need a power-on reset \n");
		int reset[11] = {0,0,0,0,0,0,0,0,0,0,0}; // wird gleich mit 0er gefüllt initialisiert
		settings_set(reset); // Power-UP Defaults sind null
		return;
	} else if (!strcmp(argv[argument], "help")) {
		settings_help(); //help der Settings ausgeben
		return;
	} else {
		settings_help(); //help der Settings ausgeben
		return;
	}
}

/* Gibt die Hilfestellung aus */
void print_help (void) { 

	printf("\nTry one of these commands:\n \
		-d --debug  |  Show additional information\n \
		-t --text   |  Show text answer \n \
		-h --help   |  Show this help message\n \
		\n \
		settings    |  Read and write sensor settings\n \
		resolution  |  Read and write resolution settings --> (set [2/4/8/16]) \
		\n");
	
}

void resolution(int argc, char* argv[], int argument) {
	//prüft auf Argumente -- wenn keine wird die Temperaturauflösung ausgegeben.
	if (argument == argc) { 
		int device = open_BUS_device(); //Bus File öffnen
		set_adress(device);
		
		int res; //(darf max 2bit aufnehmen - bzw ganze Zahlen von 0-3)
		
		res = i2c_smbus_read_byte_data(device, MCP9808_REG_RESOLUTION);
		if (res < 0 ) { //notwendig, da ansonsten ein ungültiger Wert ausgegeben wird.
			perror("i2c_smbus_read_byte_data() failed");
			exit(1);
		} 
		printf("Resolution is: ");
		if (res == 0) {
			printf("0,5");
		} else if (res == 1) {
			printf("0,25");			
		} else if (res == 2) {
			printf("0,125");	
		} else if (res == 3) {
			printf("0,0625 (power-up default)");		
		}
		printf(" °C\n");
	} else if (!strcmp(argv[argument], "set")) { //stelle Auflösung ein
		argument++;
		if (argument == argc) {
			print_help();
		} else if (!strcmp(argv[argument], "2")) { //0,5°C
			int device = open_BUS_device(); //Bus File öffnen
			set_adress(device);
			if (i2c_smbus_write_byte_data(device, MCP9808_REG_RESOLUTION, MCP9808_REG_RESOLUTION_0) < 0) {
				perror("i2c_smbus_write_byte_data() failed");
				exit(1);
			}
		} else if (!strcmp(argv[argument], "4")) { //0,25°C
			int device = open_BUS_device(); //Bus File öffnen
			set_adress(device);
			if (i2c_smbus_write_byte_data(device, MCP9808_REG_RESOLUTION, MCP9808_REG_RESOLUTION_1) < 0) {
				perror("i2c_smbus_write_byte_data() failed");
				exit(1);
			}
		} else if (!strcmp(argv[argument], "8")) { //0,125°C
			int device = open_BUS_device(); //Bus File öffnen
			set_adress(device);
			if (i2c_smbus_write_byte_data(device, MCP9808_REG_RESOLUTION, MCP9808_REG_RESOLUTION_2) < 0) {
				perror("i2c_smbus_write_byte_data() failed");
				exit(1);
			}
		} else if (!strcmp(argv[argument], "16")) { //0,0625°C
			int device = open_BUS_device(); //Bus File öffnen
			set_adress(device);
			if (i2c_smbus_write_byte_data(device, MCP9808_REG_RESOLUTION, MCP9808_REG_RESOLUTION_3) < 0) {
				perror("i2c_smbus_write_byte_data() failed");
				exit(1);
			}
		} else {
			print_help();
		}
	} else {
		print_help();
		return;
	}
	
}

/* Main nur zu Testzwecken hier - eigener Header?*/
int main (int argc, char* argv[])
{
	int argument = 1; //gibt an an welcher Stelle von argv[] das Programm im Moment ist.
	int text = 0; //false
	int debug = 0; //false
	
	/* Überprüfe übergebene Argumente */
	for (int i = 1; i < argc; i++) { // argc[0] wird ausgelassen, da dieser den Konsolenaufrufbefehl beinhält...

		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
			argument++;
			printf("\nDEBUG Information: \n");
			for (int i = 1; i < argc; i++) { // gebe alle eingegebenen Parameter aus
				printf("Argument %i: %s\n", i, argv[i]);
			}
			debug = 1; // set true
		} else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "--text")) { //Ausgabe in einem Satz formuliert
			text = 1; // set true
			argument++;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			print_help(); //help ausgeben
			return 0;
		} else {
			break;
		}
		
	}

	//prüft auf Argumente -- wenn keine wird die Temperatur ausgegeben.
	if (argument == argc) { 
		if (text) {
			printf("\nTemperature is %f °C.\n", a_temp());
			return 0;
		}
		
		printf("%f\n", a_temp());
		return 0;
	}
	
	//prüfe auf settings
	if (!strcmp(argv[argument], "settings")) {
		argument++;
		settings(argc, argv, argument, debug);
		return 0;
	}
	
		//prüfe auf Resolution
	if (!strcmp(argv[argument], "resolution")) {
		argument++;
		resolution(argc, argv, argument);
		return 0;
	}
	
	print_help(); // wird ein ungültiger Parameter angegeben erschein die Hilfe...

	return 0; //Ordnungsgemäß beendet
	
}


