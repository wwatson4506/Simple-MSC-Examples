/*
  copyFileUSB.ino
  MSC Drive read/write/copy file
 
 This example code is in the public domain.
 Warren Watson 2017-2023
*/
 
#include <USBHost_t36.h>

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Setup MSC for the number of USB Drives you are using. (one for this
// example). Mutiple  USB drives can be used. Hot plugging is supported.
USBDrive myDrive(myusb);
USBFilesystem firstPartition(myusb);

File myFile;
File myFile1;

//************************************************
// Size of read/write buffer. Start with 4096.
// Have tried 4096, 8192, 16384, 32768 and 65536.
// Usually settle on 16384 
//************************************************
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
  // Wait for port to open:
  while (!Serial) {
    yield(); // wait for serial port to connect.
  }

  // Start USBHost_t36, HUB(s) and USB devices.
  myusb.begin();
  // There is a slight delay after a USB MSC device is plugged in which
  // varys with the device being used. The device is internally 
  // initializing.
  Serial.print("\nInitializing USB MSC drive...");
  // Wait for claim proccess to finish.
  while(!firstPartition) {
    myusb.Task();
  }
  Serial.println("initialization done.");
  Serial.print("\nTesting Write, Copy and Read. BUF_SIZE = ");
  Serial.println(BUF_SIZE);

  Serial.println("\nDirectory Listing");
  File root = firstPartition.open("/");
  Serial.println("Directory listing for USB drive:");
  printDirectory(root, 0);

  // fill buf with known data
  if (BUF_SIZE > 1) {
    for (size_t i = 0; i < (BUF_SIZE - 2); i++) {
      buf[i] = (uint8_t *)('A' + (i % 26));
    }
    buf[BUF_SIZE-2] = (uint8_t *)'\r';
  }
  buf[BUF_SIZE-1] = (uint8_t *)'\n';
  
  uint32_t n = FILE_SIZE/BUF_SIZE;

  // If the file exists then remove it.
  if(firstPartition.exists("test.txt"))
	firstPartition.remove("test.txt");

  // open the file for writing.
  // FILE_WRITE_BEGIN: Start writing at begining of file. 
  // FILE_WRITE: Start writing at end of file. (append)
  myFile = firstPartition.open("test.txt", FILE_WRITE_BEGIN);
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("\nWriting to test.txt...");
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      if (myFile.write(buf, BUF_SIZE) != BUF_SIZE) {
        Serial.printf("Write Failed: Stopping Here...");
        while(1);
      }
    }
    // Display some write timing stats.
    t = millis() - t;
    flSize = myFile.size();
    MBs = flSize / t;
    Serial.printf("Wrote %lu bytes %f seconds. Speed : %f MB/s\n",
                        flSize, (1.0 * t)/1000.0f, MBs / 1000.0f);
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening test.txt: Write Failed: Stopping Here...");
    while(1);
  }
  // re-open the file for reading:
  myFile = firstPartition.open("test.txt");
  if (myFile) {
    // If the file exists then remove it.
	if(firstPartition.exists("copy.txt"))
		firstPartition.remove("copy.txt");
    // open the second file for writing. 
    myFile1 = firstPartition.open("copy.txt", FILE_WRITE_BEGIN);
    // if the file opened okay, write to it:
    if (myFile1) {
      Serial.printf("Copying test.txt to copy.txt...");
	  t = millis();
      while(myFile.read(buf, BUF_SIZE) == BUF_SIZE) {
        if (myFile1.write(buf, BUF_SIZE) != BUF_SIZE) {
          Serial.printf("Write Failed: Stoppiing Here...");
          while(1);
        }
      }
    }
    // Display some copy timing stats.
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
  // re-open the second file for reading:
  myFile1 = firstPartition.open("copy.txt");
  if (myFile1) {
    // open the file for a read. 
    myFile1 = firstPartition.open("copy.txt");
    // if the file opened okay, write to it:
    if (myFile1) {
      Serial.printf("Reading File: copy.txt...");
	  t = millis();
      while(myFile1.read(buf, BUF_SIZE) == BUF_SIZE);
    }
    // Display some read timing stats.
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
  Serial.printf("Done..\n");
}

void loop()
{
	// nothing happens after setup
}

// A simple directory display function.
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


