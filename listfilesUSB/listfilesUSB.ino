/*
 MSC USB Drive basic directory list example
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

// Volume name buffer
char	volumeName[32];
elapsedMillis mscTimeOut; 
uint8_t errCode = MS_CBW_PASS;

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
  Serial.print("\nInitializing USB MSC drive...\n");

  mscTimeOut = 0;
  while(!myDrive) {
    if((mscTimeOut > MSC_CONNECT_TIMEOUT) && (myDrive.errorCode() == MS_NO_MEDIA_ERR)) {
      Serial.println("No drive connected yet!!!!");
      Serial.println("Connect a drive to continue...\n");
      while(!myDrive);
    } 
	delay(1);
  }
  mscTimeOut = 0;
  // Wait for claim proccess to finish.
  // But not forever.
  while(!firstPartition) {
    myusb.Task();
    if(mscTimeOut >  MSC_CONNECT_TIMEOUT) {
      Serial.print("Timeout --> error code: ");
      Serial.println(myDrive.errorCode(), DEC);
      Serial.println("Halting...");
      while(1);
    }
    delay(1);
  }
  Serial.println("initialization done.");
  // Get the volume name if it exists.
  firstPartition.mscfs.getVolumeLabel(volumeName, sizeof(volumeName));
  Serial.println("\nDirectory Listing:");
  Serial.print("Volume Name: ");
  Serial.println(volumeName);
  File root = firstPartition.open("/");
  printDirectory(root, 0);
  
  Serial.println("done!");
}

void loop()
{
  // nothing happens after setup finishes.
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
       Serial.printf(F("%10ld"),entry.size()); // Formatted filesize.
       DateTimeFields datetime;
       if (entry.getModifyTime(datetime)) {
         printSpaces(4);
         printTime(datetime);
       }
       Serial.println();
     }
     entry.close();
   }
}

void printSpaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(" ");
  }
}

void printTime(const DateTimeFields tm) {
  const char *months[12] = {
    "January","February","March","April","May","June",
    "July","August","September","October","November","December"
  };
  for(uint8_t i = 0; i <= 1; i++) Serial.printf(F(" "));
  Serial.printf(F("%9s %02d %02d %02d:%02d:%02d"), // Show date/time.
				months[tm.mon],tm.mday,tm.year + 1900, 
				tm.hour,tm.min,tm.sec);
}
