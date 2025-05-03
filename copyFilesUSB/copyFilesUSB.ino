/*
  Multi MSC USB Drive and SD card filecopy testing. 
   
 This example shows how use the mscfs and SD libraries to copy a file
 between USB and SDIO card devices. It also demonstrates hot plugging
 both USB drives. Plugged in or replugged devices are auto mounted.
 You will need to copy "32MEGfile.dat" to one of the devices to run
 this sketch. You will find it in the same directory as this sketch.
 
 Created 2-15-2021
 Modified 06-04-23
 by Warren Watson
*/

#include "SPI.h"
#include "Arduino.h"
#include <USBHost_t36.h>
#include "SD.h"

// Define the number of USB drives to test (1 or 2). If count equals 2
// and only one USB drive plugged in sketch will wait for a drive to
// be plugged in. This will also happen if no drives are pluged in.
#define DRIVE_COUNT 2 //1

#define USB1 7
#define USB2 8
#define SD1  9

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Setup MSC for the number of USB Drives you are using. (one for this
// example). Mutiple  USB drives can be used. Hot plugging is supported.
USBDrive msDrive1(myusb);
USBDrive msDrive2(myusb);

const int chipSelect = BUILTIN_SDCARD;

// Create USB instances. Two USB drives.
USBFilesystem msc1(myusb);
USBFilesystem msc2(myusb);

// Create SdFat source and destination file pointers.
File file1; // src file
File file2; // dest file

elapsedMillis mscTimeOut; // Used to check for USB drives.

// File to copy. This file is provided in this sketches folder.
// Change the following filename to any other file you wish to copy.
// The file needs to be on one of the devices used.
const char *file2Copy = "32MEGfile.dat";

// Copy a file from one drive to another.
// Set 'stats' to true to display a progress bar, copy speed and copy time. 
int fileCopy(File *src, File *dest, bool stats) {
    int br = 0, bw = 0;          // File read/write count
	uint32_t bufferSize = 32*1024; // Buffer size. *** Play with this:) ***
	uint8_t buffer[bufferSize];  // File copy buffer
	uint32_t cntr = 0;
	uint32_t start = 0, finish = 0;
	uint32_t bytesRW = 0;
	int copyError = 0;
	
    /* Copy source to destination */
	start = micros();
    for (;;) {
		if(stats) { // If true, display progress bar.
			cntr++;
			if(!(cntr % 10)) Serial.printf("*");
			if(!(cntr % 640)) Serial.printf("\n");
		}
		br = src->read(buffer, sizeof(buffer));  // Read buffer size of source file (USB Type)
        if (br <= 0) {
			copyError = br;
			break; // Error or EOF
		}
		bw = dest->write(buffer, br); // Write it to the destination file (USB Type)
        if (bw < br) {
			copyError = bw; // Error or disk is full
			break;
		}
		bytesRW += (uint32_t)bw; // Update bytes transfered count.
    }
	dest->flush(); // Flush write buffer.
    // Close open files
	src->close(); // Source
    dest->close();
	finish = (micros() - start); // Get total copy time.
    float MegaBytes = (bytesRW*1.0f)/(1.0f*finish); // Convert to float.
	if(stats) // If true, display time stats.
		Serial.printf("\nCopied %u bytes in %f seconds. Speed: %3.1f MB/s\n",
		                 bytesRW,(1.0*finish)/1000000.0,MegaBytes);
	return copyError; // Return any errors or success.
}

// List Directories using SdFat "ls()" call.
void listDirectories(uint8_t device) {
	Serial.printf("-------------------------------------------------\n");
    switch(device) {
		case USB1:
			Serial.printf("\nUSB drive 1 directory listing:\n");
			msc1.mscfs.ls("/", LS_R | LS_DATE | LS_SIZE);
			break;
		case USB2:
			Serial.printf("\nUSB drive 2 directory listing:\n");
			msc2.mscfs.ls("/", LS_R | LS_DATE | LS_SIZE);
			break;
		case SD1:
			Serial.printf("\nSDIO card directory listing:\n");
			SD.sdfs.ls("/", LS_R | LS_DATE | LS_SIZE);
		default:
			Serial.printf("-------------------------------------------------\n");
			return;
    }
	Serial.printf("-------------------------------------------------\n");
}

// Check for connected USB drives.
bool checkConnected(USBDrive &drive) {
  mscTimeOut = 0;
  while(!drive.msDriveInfo.connected && (mscTimeOut < MSC_CONNECT_TIMEOUT))
    delay(1);
  if(!drive.msDriveInfo.connected) { // If nothing turns up, nothing connected.
    Serial.println("No drive connected yet!!!!");
    Serial.println("Connect a drive to continue...");
    while(!drive.msDriveInfo.connected) delay(1); // Wait for a device to be plugged in.
  }
  return true; // Always return true.
}

void setup()
{
  // Wait for port to open:
  while (!Serial) {
    yield(); // wait for serial port to connect.
  }

  Serial.printf("%cMULTI USB DRIVE AND SD CARD FILE COPY TESTING\n\n",12);
 
  // Start USBHost_t36, HUB(s) and USB devices.
  myusb.begin();

  // There is a slight delay after a USB MSC device is plugged in which
  // varys with the device being used. The device is internally initializing.
  // Initialize USB drive 1
  Serial.print("Initializing USB MSC drive 1...");
  checkConnected(msDrive1); //See if this device is plugged in.
  Serial.println("USB drive 1 is present.");
  //Check for supported filesystem.
  myusb.Task();
  if(msc1.partitionType == 0) { // 0 indicates an unrecognized filesystem.
    Serial.println("Unrecognized filesystem error...");
    Serial.printf("partitionType = %d\n",msc1.partitionType);
    Serial.println("Halting"); // For now require a reset.
    while(1);
  }

#if DRIVE_COUNT == 2
  // Initialize USB drive 2
  Serial.print("Initializing USB MSC drive 2...");
  checkConnected(msDrive2);
  Serial.println("USB drive 2 is present.");
  myusb.Task();
  if(msc2.partitionType == 0) {
    Serial.println("Unrecognized filesystem error...");
    Serial.printf("partitionType = %d\n",msc2.partitionType);
    Serial.println("Halting");
    while(1);
  }
#endif

  // Initialize SDIO card
  Serial.print("Initializing SDIO card...");
  if (!SD.begin(chipSelect)) {
	SD.sdfs.initErrorPrint(&Serial);
  } else {
     Serial.println("SDIO card is present.");
  }
}

void loop(void) {
	uint8_t c = 0;
	int copyResult = 0;

	Serial.printf("\n------------------------------------------------------------------\n");
	Serial.printf("Select:\n");
#if DRIVE_COUNT == 2
	Serial.printf("   1)  to copy '%s' from USB drive 1 to USB drive 1.\n", file2Copy);
	Serial.printf("   2)  to copy '%s' from USB drive 2 to USB drive 2.\n", file2Copy);
#endif
	Serial.printf("   3)  to copy '%s' from USB drive 1 to SDIO card.\n", file2Copy);

#if DRIVE_COUNT == 2
	Serial.printf("   4)  to copy '%s' from USB drive 2 to SDIO card.\n", file2Copy);
#endif
	Serial.printf("   5)  to copy '%s' from SDIO card to USB drive 1.\n", file2Copy);
#if DRIVE_COUNT == 2
	Serial.printf("   6)  to copy '%s' from SDIO card to USB drive 2.\n", file2Copy);
#endif
	Serial.printf("   7)  List USB Drive 1 Directory\n");
#if DRIVE_COUNT == 2
	Serial.printf("   8)  List USB Drive 2 Directory\n");
#endif
	Serial.printf("   9)  List SD card Directory\n");

	Serial.printf("------------------------------------------------------------------\n");

	while(!Serial.available()) myusb.Task(); // Support hot plugging.
	c = Serial.read();
	while(Serial.available()) Serial.read(); // Get rid of CR and/or LF if there.

	// This is a rather large and bloated switch() statement. And there are better ways to do this
	// but it served the quick copy paste modify senario:)
	switch(c) {
		case '1':
			Serial.printf("\n1) Copying from USB drive 1 to USB drive 2\n");
			// Attempt to open source file
			file1 = msc1.open(file2Copy, FILE_READ);
			if(!file1) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			// Attempt to create destination file
			file2 = msc2.open(file2Copy, FILE_WRITE_BEGIN);
			if(!file2) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(&file1, &file2, true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;

		case '2':
			Serial.printf("\n2) Copying from USB drive 2 to USB drive 1\n");
			file1 = msc2.open(file2Copy, FILE_READ);
			if(!file1) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			file2 = msc1.open(file2Copy, FILE_WRITE_BEGIN);
			if(!file2) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(&file1, &file2, true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;

		case '3':
			Serial.printf("\n3) Copying from USB drive 1 to SDIO card\n");
			file1 = msc1.open(file2Copy, FILE_READ);
			if(!file1) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			file2 = SD.open(file2Copy, FILE_WRITE_BEGIN);
			if(!file2) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(&file1, &file2, true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;

		case '4':
			Serial.printf("\n4) Copying from USB drive 2 to SDIO card\n");
			file1 = msc2.open(file2Copy, FILE_READ);
			if(!file1) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			file2 = SD.open(file2Copy, FILE_WRITE_BEGIN);
			if(!file2) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(&file1, &file2, true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;
		case '5':
			Serial.printf("\n5) Copying from SDIO card to USB drive 1 \n");
			file1 = SD.open(file2Copy, FILE_READ);
			if(!file1) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			file2 = msc1.open(file2Copy, FILE_WRITE_BEGIN);
			if(!file2) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(&file1, &file2, true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;
		case '6':
			Serial.printf("\n6) Copying from SDIO card to USB drive 2\n");
			file1 = SD.open(file2Copy, FILE_READ);
			if(!file1) {
				Serial.printf("\nERROR: could not open source file: %s\n",file2Copy);
				break;
			}
			file2 = msc2.open(file2Copy, FILE_WRITE_BEGIN);
			if(!file2) {
				Serial.printf("\nERROR: could not open destination file: %s\n",file2Copy);
				break;
			}
			copyResult = fileCopy(&file1, &file2, true);
			if(copyResult != 0) {
				Serial.printf("File Copy Failed with code: %d\n",copyResult);
			}
			break;
		case '7':
			listDirectories(7);
			break;
#if DRIVE_COUNT == 2
		case '8':
			listDirectories(8);
			break;
#endif
		case '9':
			listDirectories(9);
			break;
		default:
			break;
	}
}
