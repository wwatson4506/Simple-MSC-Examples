/*
  MSC USB Drive file dump
 
 This example shows how to read a file from the MSC USB drive using the
 USBHost_t36 library and send it over the serial port.
 	
 created  22 December 2010
 by Limor Fried
 modified 9 Apr 2012
 by Tom Igoe
 modified 08 Apr 2023
 by Warren Watson
 
 This example code is in the public domain.
 	 
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
USBDrive myDrive1(myusb);
USBFilesystem firstPartition(myusb);

void setup()
{
  
 // Open serial communications and wait for port to open:
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
  
  // open the file.
  File dataFile = firstPartition.open("datalog.txt");

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  } 
}

void loop()
{
}

