#include <Arduino.h>
#include "BinForge.h"

struct s_blob_contents {
  char name[16];
  uint8_t BCDserialNo[16];
};

struct s_blob_contents* blob_contents= (struct s_blob_contents*)blob;

void setup() {
  Serial.begin(115200);
  Serial.println("BinForge Demo Firmware");
  // This check prevents the compiler from optimizing away the blob and its verification
  if (VerifyBlob()) {
    Serial.print("Name: ");
    Serial.println("Unregistered Device");
    Serial.print("BCD Serial No: ");
    Serial.println("N/A");
  } 
  else 
  {
    // Our blob has been changed
    Serial.print("Name: ");
    Serial.println(blob_contents->name);
    Serial.print("BCD Serial No: ");
    // Blob data should be considered defined
    for (int i = 0; i < 16; i++) {
      Serial.print(blob_contents->BCDserialNo[i]); // Each digit is a byte
      if ((i != 0) & !(i % 4))
      Serial.print(":");
    }
    Serial.println();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}

