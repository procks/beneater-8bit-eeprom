/**
 * This sketch is specifically for programming the EEPROM used in the 8-bit
 * decimal display decoder described in https://youtu.be/dLh1n2dErzE
 */

void writeEEPROM(int address, uint8_t data) {
  static int lastLine = -1;

  int line = address / 16;
  int col = address % 16;

  // Print new line label if starting a new line
  if (line != lastLine) {
    if (lastLine != -1) Serial.println();  // line break after previous line
    Serial.print((line * 16 < 256 ? "0" : "")); // pad single-digit hundreds
    Serial.print((line * 16 < 16 ? "0" : ""));  // pad single-digit tens
    Serial.print(line * 16, HEX);
    Serial.print(": ");
    lastLine = line;
  }

  // Space between byte groups
  if (col == 8) Serial.print("  ");

  // Print byte in hex format
  if (data < 0x10) Serial.print("0");  // pad single-digit hex
  Serial.print(data, HEX);
  Serial.print(" ");
}

void setup() {
  Serial.begin(115200);


  // Bit patterns for the digits 0..9
  byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b };

  Serial.println("Programming ones place");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value, digits[value % 10]);
  }
  //Serial.println("Programming tens place");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value + 256, digits[(value / 10) % 10]);
  }
  //Serial.println("Programming hundreds place");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value + 512, digits[(value / 100) % 10]);
  }
  //Serial.println("Programming sign");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value + 768, 0);
  }

  //Serial.println("Programming ones place (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    writeEEPROM((byte)value + 1024, digits[abs(value) % 10]);
  }
  //Serial.println("Programming tens place (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    writeEEPROM((byte)value + 1280, digits[abs(value / 10) % 10]);
  }
  //Serial.println("Programming hundreds place (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    writeEEPROM((byte)value + 1536, digits[abs(value / 100) % 10]);
  }
  //Serial.println("Programming sign (twos complement)");
  for (int value = -128; value <= 127; value += 1) {
    if (value < 0) {
      writeEEPROM((byte)value + 1792, 0x01);
    } else {
      writeEEPROM((byte)value + 1792, 0);
    }
  }

  Serial.println("Done");
}


void loop() {
  // put your main code here, to run repeatedly:

}
