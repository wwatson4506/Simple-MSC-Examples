/*
  MSC Drive and SD card read/write copy file.
 
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
 Modified 2017-2023 Warren Watson
 This example code is in the public domain.
 	 
 */
 
#include <USBHost_t36.h>
#include <SD.h>

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

// mscController is now USBDrive. 
USBDrive msDrive1(myusb);
USBDrive msDrive2(myusb);

// USBFilesystem is a class based on claimed partitions discovered during
// initialization. We are using the first discovered partition here. More
// discovered partitions can be used by adding more instances of the
// 'USBFilesystem' class.
USBFilesystem partition1(myusb);

const int chipSelect = BUILTIN_SDCARD;

File myFile;
File myFile1;

// Size of read/write.
// Play with with this to see affect on read/write speeds.(8192 - 65536)
const size_t BUF_SIZE = 16384;
// File size in MB where MB = 1,024,000 bytes.
const uint32_t FILE_SIZE_MB = 32;

// File size in bytes.
const uint32_t FILE_SIZE = 1024000UL*FILE_SIZE_MB;
uint8_t* buf[BUF_SIZE];
uint32_t t;
uint32_t flSize = 0;
float MBs = 1.0f;

void setup()
{
 // Open serial communications and wait for port to open:
//  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect.
  }

  // Start USBHost_t36, HUB(s) and USB devices.
  myusb.begin();

  Serial.print("\nInitializing USB MSC drive...");
  while (!partition1) { myusb.Task(); }
   Serial.println("initialization done.\n");
 
  File root = partition1.open("/");
  Serial.println("Directory listing for USB drive:");
  printDirectory(root, 0);

  Serial.print("\nInitializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.\n");
  File root1 = SD.open("/");
  Serial.println("Directory listing for SD card:");
  printDirectory(root1, 0);

  // fill buf with known data
  if (BUF_SIZE > 1) {
    for (size_t i = 0; i < (BUF_SIZE - 2); i++) {
      buf[i] = (uint8_t *)('A' + (i % 26));
    }
    buf[BUF_SIZE-2] = (uint8_t *)'\r';
  }
  buf[BUF_SIZE-1] = (uint8_t *)'\n';
  
  uint32_t n = FILE_SIZE/BUF_SIZE;

  waitforInput(); // Wait for a keypress.

  //*******************************
  // If "test.txt" exists delete it.
  // (partition1 is USB drive).
  //*******************************
  if(partition1.exists("test.txt")) {
	Serial.println("\ntest.txt exists, removing...");
	partition1.remove("test.txt");
  }
  //********************************************************
  // Open test.txt file for write.
  // (FILE_WRITE_BEGIN) starts writing at the begining
  // of the file. (FILE_WRITE) appends the end of the file.
  // Write data and close the file.
  //********************************************************
  myFile = partition1.open("test.txt", FILE_WRITE_BEGIN);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("\nWriting test.txt to USB drive...");
  t = millis();
  for (uint32_t i = 0; i < n; i++) {
    if (myFile.write(buf, BUF_SIZE) != BUF_SIZE) {
      Serial.printf("Write Failed: Stopping Here...");
      while(1);
    }
  }
  t = millis() - t;
  flSize = myFile.size();
  MBs = (float)flSize / t;
  Serial.printf("Wrote %lu bytes %f seconds. Speed : %f MB/s\n",
                      flSize, (1.0 * t)/1000.0f, MBs / 1000.0f);
  // close the file:
  myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening test.txt: Write Failed: Stoppiing Here...");
    while(1);
  }

  //**************************************************************
  // Re-open "test.txt" file for reading. Defaults to (FILE_READ)
  // if not specified. If "copy.txt" exists, delete it.
  // Open "copy.txt" for write. (FILE_WRITE_BEGIN).
  //**************************************************************
  myFile = partition1.open("test.txt");
  if (myFile) {
	if(partition1.exists("copy.txt")) {
      Serial.println("\ncopy.txt exists, removing...");
	  partition1.remove("copy.txt");
	}
    // open the second file for writing seek to beginning of file. 
    myFile1 = partition1.open("copy.txt", FILE_WRITE_BEGIN);
    // if the file opened okay, write to it:
    if (myFile1) {
      Serial.printf("Copying USB drive test.txt to USB drive copy.txt...");
	  t = millis();
      // Read from "test.txt and write to "copy.txt".
      while(myFile.read(buf, BUF_SIZE) == BUF_SIZE) {
        if (myFile1.write(buf, BUF_SIZE) != BUF_SIZE) {
          Serial.printf("Write Failed: Stoppiing Here...");
          while(1);
        }
      }
    }
    //*********************************
    // Print out some copy time stats. 
    //*********************************
    t = millis() - t;
    flSize = myFile.size();
    MBs = flSize / t;
    Serial.printf("Copied %lu bytes %f seconds. Speed : %f MB/s\n",
                         flSize, (1.0 * t)/1000.0f, MBs/1000.0f);
    // close the files:
    myFile.close();
    myFile1.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  //***************************************
  // Re-open "copy.txt" file for reading.
  // Calculate read time stats.
  //***************************************
  myFile1 = partition1.open("copy.txt");
  if (myFile1) {
    // open the file for a read. 
    myFile1 = partition1.open("copy.txt");
    // if the file opened okay, write to it:
    if (myFile1) {
      Serial.printf("Reading File from USB drive copy.txt...");
	  t = millis();
      while(myFile1.read(buf, BUF_SIZE) == BUF_SIZE);
    }
    t = millis() - t;
    flSize = myFile1.size();
    MBs = flSize / t;
    Serial.printf("Read %lu bytes %f seconds. Speed : %f MB/s\n",
                       flSize, (1.0 * t)/1000.0f, MBs/1000.0f);
    // close the files:
    myFile1.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("Error opening copy.txt");
  }

  Serial.print("\nNow lets copy test.txt from the USB drive to the built in SD card.\n");
  waitforInput(); // Wait for a keypress.

  //**************************************************************
  // Open "test.txt" file on USB drive for reading. Defaults to (FILE_READ)
  // Defaults to (FILE_READ) if not specified. If "copy.txt" exists
  // delete it.
  // Open "copy.txt" for write to SD card. (FILE_WRITE_BEGIN).
  //**************************************************************
  myFile = partition1.open("test.txt"); 
  if (myFile) {
	if(SD.exists("copy.txt")) {// If it exists on SD card
    	Serial.println("\ncopy.txt exists, removing...");
		SD.remove("copy.txt");// delete it...
	}
    // Open "copy.txt" file on SD card for writing.
    // seek to beginning of file. 
    myFile1 = SD.open("copy.txt", FILE_WRITE_BEGIN);
    // if the file opened okay, write to it:
    if (myFile1) {
      Serial.printf("Copying USB drive test.txt to SD card copy.txt...");
	  t = millis();
      // Read from "test.txt and write to "copy.txt".
      while(myFile.read(buf, BUF_SIZE) == BUF_SIZE) {
        if (myFile1.write(buf, BUF_SIZE) != BUF_SIZE) {
          Serial.printf("Write Failed: Stoppiing Here...");
          while(1);
        }
      }
    }
    //*********************************
    // Print out some copy time stats. 
    //*********************************
    t = millis() - t;
    flSize = myFile.size();
    MBs = flSize / t;
    Serial.printf("Copied %lu bytes %f seconds. Speed : %f MB/s\n",
                         flSize, (1.0 * t)/1000.0f, MBs/1000.0f);
    // close the files:
    myFile.close();
    myFile1.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  //***************************************
  // Re-open "copy.txt" file for reading.
  // Calculate read time stats. (SD card)
  //***************************************
  myFile1 = SD.open("copy.txt");
  if (myFile1) {
    // open the file for a read. 
    myFile1 = SD.open("copy.txt");
    // if the file opened okay, write to it:
    if (myFile1) {
      Serial.printf("Reading File SD card copy.txt...");
	  t = millis();
      while(myFile1.read(buf, BUF_SIZE) == BUF_SIZE);
    }
    t = millis() - t;
    flSize = myFile1.size();
    MBs = flSize / t;
    Serial.printf("Read %lu bytes %f seconds. Speed : %f MB/s\n",
                       flSize, (1.0 * t)/1000.0f, MBs/1000.0f);
    // close the files:
    myFile1.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("Error opening copy.txt on SD card");
  }
  Serial.print("Done..\n");
}

void loop()
{
	// nothing happens after setup
}

void printDirectory(File dir, int numSpaces) {
   while(true) {
     File entry = dir.openNextFile();
     if (! entry) {
       //Serial.println("** no more files **");
       break;
     }
     printSpaces(numSpaces);
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numSpaces+2);
     } else {
       // files have sizes, directories do not
       printSpaces(48 - numSpaces - strlen(entry.name()));
       Serial.print("  ");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void waitforInput()
{
  Serial.println("Press anykey to continue");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}

