/*
  MSC USB Drive Volume Name Error processing testing.
 
 Created 09 Apr 2023
 by Warren Watson
 
 This example code is in the public domain.
 	 
 */
#include "USBHost_t36.h"

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

// Volume name buffer
char	volumeName[32];
uint8_t msResult = MS_INIT_PASS;
char sbuff[256]; // readLine buffer.

// A simple read line function.
char *readLine(char *s) {
	char *s1 = s;
	int	c;
	for(;;) {
		while(!Serial.available());
		switch(c = Serial.read()) {
		case 0x08:
			if(s == s1)
				break;
			s1--;
			Serial.printf("%c%c%c",c,0x20,c);
			break;
		case '\n':
		case '\r':
			Serial.printf("%c",c);
			*s1 = 0;
			return s;
		default:
			if(c <= 0x7E)
				Serial.printf("%c",c);
			*s1++ = c;
			break;
		}
	}
}

uint8_t checkForDrive(USBDrive *drive, USBFilesystem *fs) {
  uint8_t errCode = MS_INIT_PASS;
  uint32_t mscTimeOut = millis();
  while(!*drive) {
    myusb.Task();
    if(((millis()- mscTimeOut) >= MSC_CONNECT_TIMEOUT) && (drive->errorCode() == MS_NO_MEDIA_ERR)) {
      Serial.println("No drive connected yet!!!!");
      Serial.println("Connect a drive to continue...\n");
      while(!*drive);
    } 
	delay(1);
  }
  // Wait for claim proccess to finish.
  // But not forever.
  while(!*fs) {
    myusb.Task();
    errCode = drive->errorCode();
    delay(50);
  }
  return errCode;
}

void setup()
{
  // Wait for port to open:
  while (!Serial && (millis() < 5000)) {;}

  // Start USBHost_t36, HUB(s) and USB devices.
  myusb.begin();
  delay(3000);
  myusb.Task();
}

void loop()
{
  Serial.printf("Press any key to begin\n");
  readLine(sbuff);
  Serial.print("\nInitializing USB MSC drive...\n");
  msResult = checkForDrive(&myDrive, &firstPartition);
  if(msResult != MS_INIT_PASS) {
    Serial.println("Unrecognized File System: ");
    Serial.print("Error Code: 0x");
    Serial.println(msResult, HEX);
    Serial.println("Halting...");
    while(1);
  }
  firstPartition.mscfs.getVolumeLabel(volumeName, sizeof(volumeName));
  Serial.printf("Volume Name: %s\n",volumeName);
}

