/* MSC Hotplug testing */
#include <USBHost_t36.h>

// Setup USBHost_t36 and as many HUB ports as needed.
USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);

// Instances for the number of USB drives you are using.
USBDrive myDrive1(myusb); // Up to two USB drives for testing.
USBDrive myDrive2(myusb);
// Create a drive list
USBDrive *drive_list[] = {&myDrive1, &myDrive2};

// Instances for accessing the files on each drive.
// Up to 4 partitions for each drive times 2 drives = 8 partitions
// if needed. (one partition per drive works as well).
USBFilesystem pf1(myusb);
USBFilesystem pf2(myusb);
USBFilesystem pf3(myusb);
USBFilesystem pf4(myusb);
USBFilesystem pf5(myusb);
USBFilesystem pf6(myusb);
USBFilesystem pf7(myusb);
USBFilesystem pf8(myusb);

// Create a filesystem list for all available partitions.
USBFilesystem *filesystem_list[] = {&pf1, &pf2, &pf3, &pf4, &pf5, &pf6, &pf7, &pf8};

char volname[32];

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for Arduino Serial Monitor to connect.
  }

  // Start USBHost_t36
  myusb.begin();
  delay(500);  // give drives a little time to startup
}

void loop(void) {

  Serial.printf("%c MSC Hotplug Testing\n\n",12); // Best seen using VT100-VT102 aware terminal emulator.
  Serial.printf("This sketch is setup for hotplugging up to 2 devices\n");
  Serial.printf("if using 2 USB drives you will a USB HUB, but is not required for one USB drive testing.\n");
  Serial.printf("After plugging or unplugging drives press enter to refresh display.\n\n");

  myusb.Task();  // Refresh USBHost_t36.

  // lets check each of the drives.
  for (uint16_t drive_index = 0; drive_index < (sizeof(drive_list)/sizeof(drive_list[0])); drive_index++) {
    USBDrive *pdrive = drive_list[drive_index]; // Create a temporary pointer to a drive instance.
    if (*pdrive) { // Is this drive connected??
      Serial.println("***************************************************");
      Serial.printf(" === Drive %d connected and using ", drive_index);
      if (!pdrive->filesystemsStarted()) { // See if filesystem has started yet.
        pdrive->startFilesystems(); // If not started then start filesystem.
      }

      bool first_fs = true;
      // Find filesystem for this drive.
      for (uint16_t fs_index = 0; fs_index < (sizeof(filesystem_list)/sizeof(filesystem_list[0])); fs_index++) {
        USBFilesystem *pfs = filesystem_list[fs_index]; // Create a temporary pointer to filesystem instance.
        if (*pfs && pfs->device == pdrive) { // Associate with correct USB drive.

          Serial.printf("file system %u\n", fs_index);
          const uint8_t *psz; 
          // print out some info about drive.
          if ((psz = pfs->manufacturer()) != nullptr) Serial.printf("            Manufacturer: %s\n", psz);
          if ((psz = pfs->product()) != nullptr) Serial.printf("                 Product: %s\n", psz);
          if ((psz = pfs->serialNumber()) != nullptr) Serial.printf("\t   Serial Number: %s\n", psz);
		  Serial.printf(F("\t      HUB Number: %d\n"), pdrive->msDriveInfo.hubNumber);
          Serial.printf(F("\t        HUB Port: %d\n"), pdrive->msDriveInfo.hubPort);
          Serial.printf(F("\t  Device Address: %d\n"), pdrive->msDriveInfo.deviceAddress);
          Serial.printf(F("\tRemovable Device: "));
          if(pdrive->msDriveInfo.inquiry.Removable == 1) {
            Serial.printf(F("YES\n"));
          } else {
            Serial.printf(F("NO\n"));
          }
        } else {
	      pfs->mscfs.end(); // Unmount partition (sort of!!)
        }
      }
    } else {
      Serial.println("***************************************************");
      Serial.printf(" === Drive %d NOT connected ===\n", drive_index);
	}
      Serial.println("***************************************************");
  }
  while (Serial.read() != -1) ;
  Serial.println("\n *** Press enter after changes to refresh drive list ***");
  while (Serial.read() == -1)  myusb.Task();
  while (Serial.read() != -1) ;
  
}
