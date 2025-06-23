/**
   This sketch programs the microcode EEPROMs for the 8-bit breadboard computer
   See this video for more: https://youtu.be/JUVt_KYAp-I
*/

#define HLT 0b1000000000000000  // Halt clock
#define MI  0b0100000000000000  // Memory address register in
#define RI  0b0010000000000000  // RAM data in
#define RO  0b0001000000000000  // RAM data out
#define IO  0b0000100000000000  // Instruction register out
#define II  0b0000010000000000  // Instruction register in
#define AI  0b0000001000000000  // A register in
#define AO  0b0000000100000000  // A register out
#define EO  0b0000000010000000  // ALU out
#define SU  0b0000000001000000  // ALU subtract
#define BI  0b0000000000100000  // B register in
#define OI  0b0000000000010000  // Output register in
#define CE  0b0000000000001000  // Program counter enable
#define CO  0b0000000000000100  // Program counter out
#define J   0b0000000000000010  // Jump (program counter in)
#define FI  0b0000000000000001  // Flags In

#define FLAGS_Z0C0 0
#define FLAGS_Z0C1 1
#define FLAGS_Z1C0 2
#define FLAGS_Z1C1 3

#define JC  0b0111
#define JZ  0b1000
#define JNC 0b1001
#define JNZ 0b1010

const PROGMEM uint16_t UCODE_TEMPLATE[16][8] = {
  {MI|CO,  RO|II|CE,  0,      0,            0,            0, 0, 0}, // 0000 - 00 - NOP
  {MI|CO,  RO|II|CE,  IO|MI,  RO|AI,        0,            0, 0, 0}, // 0001 - 01 - LDA
  {MI|CO,  RO|II|CE,  IO|MI,  RO|BI,        EO|AI|FI,     0, 0, 0}, // 0010 - 02 - ADD
  {MI|CO,  RO|II|CE,  IO|MI,  RO|BI,        EO|AI|SU|FI,  0, 0, 0}, // 0011 - 03 - SUB
  {MI|CO,  RO|II|CE,  IO|MI,  AO|RI,        0,            0, 0, 0}, // 0100 - 04 - STA
  {MI|CO,  RO|II|CE,  IO|AI,  0,            0,            0, 0, 0}, // 0101 - 05 - LDI
  {MI|CO,  RO|II|CE,  IO|J,   0,            0,            0, 0, 0}, // 0110 - 06 - JMP
  {MI|CO,  RO|II|CE,  0,      0,            0,            0, 0, 0}, // 0111 - 07 - JC
  {MI|CO,  RO|II|CE,  0,      0,            0,            0, 0, 0}, // 1000 - 08 - JZ
  {MI|CO,  RO|II|CE,  0,      0,            0,            0, 0, 0}, // 1001 - 09 - JNC
  {MI|CO,  RO|II|CE,  0,      0,            0,            0, 0, 0}, // 1010 - 10 - JNZ
  {MI|CO,  RO|II|CE,  IO|BI,  EO|AI|FI,     0,            0, 0, 0}, // 1011 - 11 - INC
  {MI|CO,  RO|II|CE,  IO|BI,  EO|AI|SU|FI,  0,            0, 0, 0}, // 1100 - 12 - DEC
  {MI|CO,  RO|II|CE,  IO|MI,  RO|OI,      HLT,            0, 0, 0}, // 1101 - 13 - DSP / HLT
  {MI|CO,  RO|II|CE,  AO|OI,  0,            0,            0, 0, 0}, // 1110 - 14 - OUT
  {MI|CO,  RO|II|CE,  HLT,    0,            0,            0, 0, 0}, // 1111 - 15 - HLT
};

uint16_t ucode[4][16][8];

void initUCode() {
  // ZF = 0, CF = 0
  memcpy_P(ucode[FLAGS_Z0C0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z0C0][JNC][2] = IO|J;
  ucode[FLAGS_Z0C0][JNZ][2] = IO|J;

  // ZF = 0, CF = 1
  memcpy_P(ucode[FLAGS_Z0C1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z0C1][JC][2] = IO|J;
  ucode[FLAGS_Z0C1][JNZ][2] = IO|J;

  // ZF = 1, CF = 0
  memcpy_P(ucode[FLAGS_Z1C0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z1C0][JZ][2] = IO|J;
  ucode[FLAGS_Z1C0][JNC][2] = IO|J;

  // ZF = 1, CF = 1
  memcpy_P(ucode[FLAGS_Z1C1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z1C1][JC][2] = IO|J;
  ucode[FLAGS_Z1C1][JZ][2] = IO|J;
}

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
  // put your setup code here, to run once:
  initUCode();

  Serial.begin(115200);

  // Program data bytes
  Serial.println("Programming EEPROM");

  // Program the 8 high-order bits of microcode into the first 128 bytes of EEPROM
  for (int address = 0; address < 1024; address += 1) {
    int flags       = (address & 0b1100000000) >> 8;
    int byte_sel    = (address & 0b0010000000) >> 7;
    int instruction = (address & 0b0001111000) >> 3;
    int step        = (address & 0b0000000111);

    if (byte_sel) {
      writeEEPROM(address, ucode[flags][instruction][step]);
    } else {
      writeEEPROM(address, ucode[flags][instruction][step] >> 8);
    }

  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
