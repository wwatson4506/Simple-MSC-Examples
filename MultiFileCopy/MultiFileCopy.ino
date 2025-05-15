/*
  MSC Drive and SD card multiple file copy example.
 
 This example shows how to read and write data to and from an SD card file
 and USB MSC drive. 	

 The SD card circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11, pin 7 on Teensy with audio board
 ** MISO - pin 12
 ** CLK - pin 13, pin 14 on Teensy with audio board
 ** CS - pin 4, pin 10 on Teensy with audio board
 
 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 Modified 2017-2025 Warren Watson
 This example code is in the public domain.
 	 
 */
 
#include <USBHost_t36.h>
#include <SD.h>

//********************************************************
//********************************************************
//********************************************************
// Uncomment this define to copy from USB drive to SDCARD.
// Comment out this define to copy from SDCARD to USB drive.
#define USB_TO_SD 1
//********************************************************
//********************************************************
//********************************************************

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Setup MSC for the number of USB Drives you are using. (Two for this example)
// Mutiple  USB drives can be used. Hot plugging is supported. There is a slight
// delay after a USB MSC device is plugged in. This is waiting for initialization
// but after it is initialized ther should be no delay.
// ********* mscController is now USBDrive. *************
USBDrive msDrive1(myusb);
USBDrive msDrive2(myusb);

// USBFilesystem is a class based on claimed partitions discovered during
// initialization. We are using the first discovered partition here. More
// discovered partitions can be used by adding more instances of the
// 'USBFilesystem' class.
USBFilesystem partition1(myusb); // Up to four partions per device supported.

const int chipSelect = BUILTIN_SDCARD; // Using the built in SD slot.

// Create a copy from File instance (USB).
File myFile;
// Create a copy to File instance (SD).
File myFile1;

char fname[256]; // String buffer for filename to copy.


void setup()
{
// Wait for Serial port to open (up to 5 milliseconds):
   while (!Serial && (millis() < 5000)) {
    ; // wait for serial port to connect.
  }

  Serial.printf("%c***** USB to SD or SD to USB multiple file copy example *****\n\n",12);

  Serial.println("This sketch will copy all files in the root directory of");
  Serial.println("a USB drive to SDCARD or from an SDCARD to USB drive.");
  Serial.println("\nThere is define at the beginning of the sketch that can");
  Serial.println("you can change to determine the source and destination devices.");
  Serial.println("'#define USB_TO_SD 1'. The default is uncommented for USB to SDCARD.");
  Serial.println("Comment out this define to copy files from SDCARD to the USB drive\n");
  waitforInput();

  // Start USBHost_t36, HUB(s) and USB devices.
  myusb.begin();
  delay(500);  // give drives a little time to startup
  myusb.Task();  // Refresh USBHost_t36.

  // Detect and initialize USB drive.
  Serial.print("Initializing USB MSC drive.\n");
  Serial.print("*** NOTE: If no USB drive is connected, sketch will hang here forever ***\n");
  Serial.print("Connect USB drive now if not connected and sketch will resume running...");
  while (!partition1) { myusb.Task(); }
  Serial.println("initialization done.\n");

  // Detect and initialize SDCARD.
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    Serial.println("*** Insert SDCARD and restart program ***\n");
  }
  Serial.println("initialization done.\n");

#if defined(USB_TO_SD)
  // Open the root directory on the USB drive.
  File root = partition1.open("/"); // Source is root directory on USB drive.
#else
  // Open the root directory on the SDCARD.
  File root = SD.open("/"); // Source it root directory on SDCARD.
#endif

  while(true) {
    File entry = root.openNextFile(); // Get next file entry.
    if (! entry) break; // If entry is NULL then we are at the end of the dirrectory.
    if(!entry.isDirectory()) { // Skip over directories Only
		                       // copy files in root directory.
      strcpy(fname, entry.name()); // Copy filename to "fname" buffer.
      // Now copy file to SDCARD or USB drive.
#if defined(USB_TO_SD)
      Serial.printf("\nCopying '%s' from USB drive to SD card\n",fname);
#else
      Serial.printf("\nCopying '%s' from SD card to USB drive\n",fname);
#endif
      if(copyFile(fname) < 0) { // Check for copy error (-1).
        Serial.printf("An error occured while copying file %s to the SD drive!!\n",fname);
      }
      entry.close(); // Close thiss file entry.
    }
  }
  Serial.println("\nAll files in USB root directory have been copied to the SDCARD.");
  Serial.println("**** FINISHED ****");
  	
}

void loop()
{
	// nothing happens after setup
}

// *********************************************************
// This is the function that copies one file to the SD card
// or USB drive.
// filnam: Filename of file to copy.
// *********************************************************
int copyFile(char *filnam) {

  int32_t br = 0, bw = 0;          // File read/write count
  uint32_t bufferSize = 16*1024; // Buffer size. Play with this:)
  uint8_t buffer[bufferSize];  // File copy buffer
  uint32_t bytesRW = 0; // Accumulation of total file size.
  float MegaBytes = 0;
  
#if defined(USB_TO_SD) // USB_TO_SD == 1 then copy from USB drive to SDCARD.
  myFile = partition1.open(filnam); // Open USB drive source file for read op.
  myFile1 = SD.open(filnam, FILE_WRITE_BEGIN); // Open "filnam" file on SD card for writing.
#else // USB_TO_SD == 0 then copy from SDCARD drive to USB drive.
  myFile1 = SD.open(filnam); // Open SD card source file for read op.
  myFile = partition1.open(filnam, FILE_WRITE_BEGIN); // Open "filnam" file on USB drive for writing.
#endif

  // if the write file opened okay, write to it:
#if defined(USB_TO_SD)
  if (myFile) {
#else
  if (myFile1) {
#endif
	
	/* Copy source to destination */
	uint32_t start = micros();
    for(;;) {
      // Read from USB file and write to SD file.
#if defined(USB_TO_SD)
      br = myFile.read(buffer, bufferSize); // Read a 'BUF_SIZE' chunk of file.
#else
      br = myFile1.read(buffer, bufferSize); // Read a 'BUF_SIZE' chunk of file.
#endif
	  if (br <= 0) break; // Error or EOF
        // Write chunk to SD drive.
#if defined(USB_TO_SD)
      bw = myFile1.write(buffer, br); // Write buffer full to destnation drive.
#else
      bw = myFile.write(buffer, br); // Write buffer full to destnation drive.
#endif
	  if (bw < br) break; // Error or disk is full
      bytesRW += (uint64_t)bw;  // Keep track of total bytes written.
    }
    // close the files:
    myFile.close();
    myFile1.close();
    // Proccess posible errors.
    if(br < 0) {
      Serial.println("**** A read error occured!! ****");
      return -1; // Return failure.
    } else if(bw < br) {
      Serial.println("**** A write error occured!! ****"); // Can also be disk full error.
      return -1; // Return failure.
	}

#if 1 // Set to 1 to turn on copy speed display.
	uint32_t finish = (micros() - start); // Get total copy time.
	MegaBytes = (bytesRW*1.0f)/(1.0f*finish); // Compute MB/s.
	Serial.printf("Copied %lu bytes in %f seconds. Speed: %f MB/s\n",
                   bytesRW,(1.0*finish)/1000000.0,MegaBytes);
#endif

  }
  return 0; // 0 = success.
}

void waitforInput()
{
  Serial.println("Press anykey to continue");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}

