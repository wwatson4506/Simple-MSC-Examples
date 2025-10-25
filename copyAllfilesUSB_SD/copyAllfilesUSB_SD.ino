/*
  Multi MSC USB Drive and SD card filecopy testing. 
   
 This example shows how use the mscfs and SD libraries to copy a file
 between USB and SDIO card devices. It also demonstrates hot plugging
 USB drive. Plugged in or replugged devices are auto mounted.
 
 Created 2-15-2021
 Modified 10-23-25
 by Warren Watson
*/

#include "SPI.h"
#include "Arduino.h"
#include <USBHost_t36.h>
#include "SD.h"

// USB and SD card numeric identifiers.
#define USB1 3
#define SD1  4

// Change to false to NOT display copy progresss bar and stats for each
// file copied.
#define USE_STATS true

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Setup MSC for the number of USB Drives you are using. (one for this
// example). Hot plugging is supported.
USBDrive msDrive1(myusb);

// SDIO card chipselect pin def. (254)
const int chipSelect = BUILTIN_SDCARD;

// Create USB instances. Two USB drives.
USBFilesystem msc1(myusb);

// Create file copy source and destination file pointers.
File file1; // src file
File file2; // dest file
File USBroot; // File pointer to USB root directory.
File SDroot; // File pointer to SDIO root directory.

// Used to check for USB drives connection.
elapsedMillis mscTimeOut;

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
    dest->close(); // Desstination
	finish = (micros() - start); // Get total copy time.
    float MegaBytes = (bytesRW*1.0f)/(1.0f*finish); // Convert to float.
	
	if(stats) // If true, display time stats.
		Serial.printf("\nCopied %u bytes in %f seconds. Speed: %3.1f MB/s\n",
		                 bytesRW,(1.0*finish)/1000000.0,MegaBytes);
	return copyError; // Return any errors or success.
}

// This function scans through the root directory for every occurence of
// a regular file on the source drive then copies it to the destination
// drive. It opens the source file for reading and opens the destination
// file for writing. (bails on error).
//   File dir: is the source directory for source file(s0 to read.
//   uint8_t source: is the source drive identifier (USB1 or SD1).
// Then it calls the fileCopy(&file1,&file2,stats) function.
//   &file1: Address of source file pointer.
//   &file2: Address of desination file pointer.
//   bool stats: TRUE = print progress bar and copy stats. False = quiet.
int copyDirectory(File dir, uint8_t source) {
   int copyResult = 0;
   while(true) {
     File entry = dir.openNextFile(); // Get next directory entry.
     if (! entry) { // No more enries, we are done.
       //Serial.println("** no more files **");
       break; // Break out of while loop and close the directory.
     }
     Serial.println(); Serial.println(entry.name());  // Show name of file we are copying.
     if (entry.isDirectory()) { // Not doing recursive copy in sub dirs.
       break; // Skip over directories.
     } else {
       if(source == USB1) { // Copy from USB to SD. (USB is src and SD is dest)
         file1 = msc1.open(entry.name(), FILE_READ); // Open source file.
		 if(!file1) { // Did not open. Return error.
		   Serial.printf("\nERROR: could not open source file: %s\n",entry.name());
		   return -1;
		 }
		 file2 = SD.open(entry.name(), FILE_WRITE_BEGIN); // Open destination file.
		 if(!file2) { // Did not open. Return error.
		   Serial.printf("\nERROR: could not open destination file: %s\n",entry.name());
		   return -1;
		 }
		 copyResult = fileCopy(&file1, &file2, USE_STATS); // Do the actual file copy.
		 if(copyResult != 0) { // Copy failed. Return error.
		   Serial.printf("File Copy Failed with code: %d\n",entry.name());
           return -1;
		 }
      } else { // Copy from SD to USB. (SD is src and USB is dest)
		 file1 = SD.open(entry.name(), FILE_READ); // Open source file.
		 if(!file1) { // Did not open. Return error.
		   Serial.printf("\nERROR: could not open source file: %s\n",entry.name());
		   return -1;
		 }
		 file2 = msc1.open(entry.name(), FILE_WRITE_BEGIN); // Open destination file.
		 if(!file2) { // Did not open. Return error.
		   Serial.printf("\nERROR: could not open destination file: %s\n",entry.name());
		   return -1;
		 }
		 copyResult = fileCopy(&file1, &file2, USE_STATS);
		 if(copyResult != 0) { // Copy failed. Return error.
		   Serial.printf("File Copy Failed with code: %d\n",copyResult);
		   return -1;
		 }
       }
     }
     entry.close();  // Close directory.
   }
   return 0;
}

// List Directories using SdFat "ls()" call.
void listDirectories(uint8_t device) {
	Serial.printf("-------------------------------------------------\n");
    switch(device) {
		case USB1:
			Serial.printf("\nUSB drive 1 directory listing:\n");
			msc1.mscfs.ls("/", LS_R | LS_DATE | LS_SIZE);
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
    Serial.println("Connect a drive to continue.");
    while(!drive.msDriveInfo.connected) delay(1000); // Wait for a device to be plugged in.
  }
  return true; // Always return true.
}

// Setup USB and SDIO and mount them.
void setup()
{
  // Wait for port to open:
  while (!Serial) {
    yield(); // wait for serial port to connect.
  }

  Serial.printf("%cUSB DRIVE AND SD CARD MULTI FILE COPY TESTING\n\n",12);
 
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

  // Initialize SDIO card
  Serial.print("Initializing SDIO card...");
  if (!SD.begin(chipSelect)) {
	SD.sdfs.initErrorPrint(&Serial);
    Serial.println("Halting"); // For now require a reset.
    while(1);
  } else {
     Serial.println("SDIO card is present.");
  }
}

// Display menu and process user selection.
void loop(void) {
	uint8_t c = 0;

	Serial.printf("\n------------------------------------------------------------------\n");
	Serial.printf("Select:\n");
	Serial.printf("   1)  to copy all files from USB drive 1 to SDIO card.\n");
	Serial.printf("   2)  to copy all files from SDIO card to USB drive 1.\n");
	Serial.printf("   3)  List USB Drive 1 Directory\n");
	Serial.printf("   4)  List SD card Directory\n");

	Serial.printf("------------------------------------------------------------------\n");

	while(!Serial.available()) myusb.Task(); // Support hot plugging.
	c = Serial.read();
	while(Serial.available()) Serial.read(); // Get rid of CR and/or LF if there.
    // Process input 1 to 4.
	switch(c) {
		case '1':
			Serial.printf("\n1) Copying all files from USB drive 1 to SDIO card\n");
			USBroot = msc1.open("/"); // Open USB device USB1 root directory.
            copyDirectory(USBroot, USB1); // Copy all files from USB to SD card.
			break;
		case '2':
			Serial.printf("\n2) Copying all files from SDIO card to USB drive 1 \n");
            SDroot = SD.open("/"); // Open SD card device SD1 root directory.
            copyDirectory(SDroot, SD1); // Copy all files from SD card to USB.
			break;
		case '3':
			listDirectories(3); // List USB drive files.
			break;
		case '4':
			listDirectories(4); // List SD card files.
			break;
		default:
			break;
	}
}
