/*
  MSC USB Drive basic file example
 
 This example shows how to create and destroy an MSC USB drive file 	
 The circuit:
 
 created   Nov 2010
 by David A. Mellis
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
USBDrive myDrive(myusb);
USBFilesystem firstPartition(myusb);

File myFile;

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
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect.
  }
  Serial.println("initialization done.");

  if (firstPartition.exists("example.txt")) {
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");
  }

  // open a new file and immediately close it:
  Serial.println("Creating example.txt...");
  myFile = firstPartition.open("example.txt", FILE_WRITE);
  myFile.close();

  // Check to see if the file exists: 
  if (firstPartition.exists("example.txt")) {
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");  
  }

  // delete the file:
  Serial.println("Removing example.txt...");
  firstPartition.remove("example.txt");

  if (firstPartition.exists("example.txt")){ 
    Serial.println("example.txt exists.");
  }
  else {
    Serial.println("example.txt doesn't exist.");  
  }
}

void loop()
{
  // nothing happens after setup finishes.
}



