#include <Adafruit_Fingerprint.h>

HardwareSerial mySerial(1);
enum FPSTATUS {
  FAILED, WAITING, SUCCESS
};
class fingerprint {
  public:
    Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
    fingerprint() {
      // This is like the setup code
    }

    void initialize() {
      mySerial.begin(57600, SERIAL_8N1, 18, 17);
      // finger.begin(57600);
      if (finger.verifyPassword()) {
       USBSerial.println("Found fingerprint sensor!");
      } else {
       USBSerial.println("Did not find fingerprint sensor :(");
        while (1) { delay(100); }
      }
    }



    int enroll(int step, int idNum) {
      int fpSensorState = -1;

      if (step == 1 || step == 3) {
        USBSerial.println("Place a finger to enroll");
        fpSensorState = finger.getImage();
        switch (fpSensorState) {
          case FINGERPRINT_OK:
            USBSerial.println("Image taken");
            break; // Go to the bottom part
          case FINGERPRINT_NOFINGER:
            return WAITING;
          default:
            USBSerial.println("Unknown error");
            return FAILED;
        }
        int i = step;
        if (step == 3)
          i = 2;
        fpSensorState = finger.image2Tz(i); // Need to do this image conversion process two times
        switch (fpSensorState) {
          case FINGERPRINT_OK:
            USBSerial.println("Image Converted");
            return SUCCESS;
          case FINGERPRINT_IMAGEMESS:
            USBSerial.println("Image too messy");
            return FAILED;
          default:
            USBSerial.println("Unknown error");
            return FAILED;
        }
      }

      if (step == 2) { // The step where you have to wait for user to remove finger
       USBSerial.println("Remove finger");
        if (finger.getImage() == FINGERPRINT_NOFINGER)
          return SUCCESS;
        else
          return WAITING;
      }

      if (step == 4) { // Final step where we create a model from the two fingers and store it into memory
        fpSensorState = finger.createModel();
        switch (fpSensorState) {
          case FINGERPRINT_OK:
            USBSerial.println("Prints matched!");
            break;
          case FINGERPRINT_PACKETRECIEVEERR:
            USBSerial.println("Communication error");
            return FAILED;
          case FINGERPRINT_ENROLLMISMATCH:
            USBSerial.println("Fingerprints did not match");
            return FAILED;
        }

        if (idNum < 1 || idNum > 127) {
          USBSerial.println("ID Out of Bounds");
          return FAILED;
        }

        fpSensorState = finger.storeModel(idNum);
        switch (fpSensorState) {
          case FINGERPRINT_OK:
            USBSerial.println("Successfully Stored!");
            return SUCCESS;
          case FINGERPRINT_PACKETRECIEVEERR:
            USBSerial.println("Communication error");
            return FAILED;
          case FINGERPRINT_BADLOCATION:
            USBSerial.println("Could not store in that location");
            return FAILED;
          case FINGERPRINT_FLASHERR:
            USBSerial.println("Error writing to flash");
            return FAILED;
          default:
            USBSerial.println("Unknown error");
            return FAILED;
        }
      }

      
    }

    FPSTATUS verify() {
      int fpSensorState = finger.getImage();
      switch (fpSensorState) {
        case FINGERPRINT_OK:
        break; //  USBSerial.println("Image taken");
      case FINGERPRINT_NOFINGER:
        return WAITING; //  USBSerial.println("No finger detected");
      case FINGERPRINT_PACKETRECIEVEERR:
        return FAILED; //  USBSerial.println("Communication error");
      case FINGERPRINT_IMAGEFAIL:
        return FAILED; //  USBSerial.println("Imaging error");
      default:
        return FAILED; 
      } 

      fpSensorState = finger.image2Tz();
      switch (fpSensorState) {
      case FINGERPRINT_OK:
        USBSerial.println("Image converted");
        break;
      case FINGERPRINT_IMAGEMESS:
        USBSerial.println("Image too messy");
        return FAILED;
      case FINGERPRINT_PACKETRECIEVEERR:
        USBSerial.println("Communication error");
        return FAILED;
      case FINGERPRINT_FEATUREFAIL:
        USBSerial.println("Could not find fingerprint features");
        return FAILED;
      case FINGERPRINT_INVALIDIMAGE:
        USBSerial.println("Could not find fingerprint features");
        return FAILED;
      default:
        USBSerial.println("Unknown error");
        return FAILED;
      }
      
      fpSensorState = finger.fingerSearch();
      switch (fpSensorState) {
        case FINGERPRINT_OK:
          USBSerial.println("Found a print match!");
          return SUCCESS;
        case FINGERPRINT_PACKETRECIEVEERR:
          USBSerial.println("Communication Error");
          return FAILED;
        case FINGERPRINT_NOTFOUND:
          USBSerial.println("Did not found a match");
          return FAILED;
        default:
          USBSerial.println("Unknown error");
          return FAILED;
      }
    }

    void deleteFingerprint(int id) {
      finger.deleteModel(id);
    }

    void emptyDatabase() {
      finger.emptyDatabase();
    }

};