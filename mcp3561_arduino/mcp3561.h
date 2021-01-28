/*
 * NOTES - 
 * The MCP3561 uses SPI mode 0,0.
 * This should be turned into a class so that it's simpler to use.
 */
#define DEVICE_ADDRESS 0b01
#define DEVICE_ADDRESS_MASK (DEVICE_ADDRESS << 6)
#define COMMAND_ADDR_POS 2

// USEFUL MASKS FOR ADC COMMUNICATION
#define DATA_READY_MASK 0b00000100 // Tells us whether data is ready from an SPI transaction
#define ADDRESS_MASK 0b00111000
#define WRITE_COMMAND_MASK 0b00000010
#define WRITE_COMMAND WRITE_COMMAND_MASK | DEVICE_ADDRESS_MASK
#define IREAD_COMMAND_MASK 0b00000011 // Incremental read command
#define IREAD_COMMAND IREAD_COMMAND_MASK | DEVICE_ADDRESS_MASK
#define SREAD_COMMAND_MASK 0b1 // Static read command
#define SREAD_DATA_COMMAND SREAD_COMMAND_MASK | DEVICE_ADDRESS_MASK

#define CONFIG0_ADDR 0x01
#define CONFIG0_WRITE (CONFIG0_ADDR << COMMAND_ADDR_POS) | WRITE_COMMAND
#define CONFIG0_CLK_SEL_MASK 0b00110000
#define CONFIG0_CLK_SEL_POS 4
#define CONFIG0_CLK_SEL_INT 0b11 << CONFIG0_CLK_SEL_POS
#define CONFIG0_CLK_SEL_EXT 0b00 << CONFIG0_CLK_SEL_POS
#define CONFIG0_ADC_MODE_POS 0
#define CONFIG0_ADC_MODE_CONV 0b11 << CONFIG0_ADC_MODE_POS

#define CONFIG1_ADDR 0x02
#define CONFIG1_WRITE (CONFIG1_ADDR << COMMAND_ADDR_POS) | WRITE_COMMAND
#define CONFIG1_OSR_POS 2
#define CONFIG1_OSR_32 0b0000 << CONFIG1_OSR_POS
#define CONFIG1_OSR_256 0b0011 << CONFIG1_OSR_POS

#define CONFIG3_ADDR 0x04
#define CONFIG3_WRITE (CONFIG3_ADDR << COMMAND_ADDR_POS) | WRITE_COMMAND
#define CONFIG3_CONV_MODE_POS 6
#define CONFIG3_CONV_MODE_CONTINUOUS 0b11 << CONFIG3_CONV_MODE_POS

#define IRQ_ADDR 0x05
#define IRQ_WRITE (IRQ_ADDR << COMMAND_ADDR_POS) | WRITE_COMMAND
#define IRQ_MODE_POS 2
#define IRQ_MODE_HIGH 0b01110111

#define CS_PIN 9
#define IRQ_PIN 8 // data ready interrupt pin. HIGH = data ready. LOW = data not ready.
#define MCLK_PIN 7
#define MISO_PIN 12
#define EXTERNAL_SYNC_PIN 5
#define MAX_SYNCHRONIZATION_POINTS 5000

// USEFUL FAST COMMANDS AND OTHER COMMANDS
// Resets the device registers to their default  values
#define DEVICE_RESET_COMMAND DEVICE_ADDRESS_BYTE | 0b111000

class MCP3561 {
  public: 
    // Class methods
    void readRegisters(void);
    void verifyRegisters(void);
    void writeRegisterDefaults(void);
    void printRegisters(void);
    void Reset(void);
    static void readADCData(void);
    static void recordSync(void);

    // Static class variables used in interrupts
    static uint32_t data_counter;
    static uint32_t data_points_to_sample;
    static byte adc_data[3];
    static uint32_t adc_sample;
    static uint32_t synchronization_data[MAX_SYNCHRONIZATION_POINTS];
    static uint32_t synchronization_counter;

    // Class variables
    byte config0_data;
    byte config1_data;
    byte config2_data;
    byte config3_data;
    byte irq_data;
    byte mux_data;
    uint32_t scan_data; // A 24-bit register
    uint32_t timer_data; // A 24-bit register
    uint32_t offsetcal_data; // A 24-bit register
    uint32_t gaincal_data; // A 24-bit register
    uint32_t reserved_1_data;
    byte reserved_2_data;
    byte lock_data;
    uint16_t reserved_3_data;
    uint16_t crccfg_data;
    
    bool config0_ok = false; 
    bool config1_ok = false;
    bool config2_ok = false;
    bool config3_ok = false;
    bool irq_ok = false;
    bool mux_ok = false;
    bool scan_ok = false;
    bool timer_ok = false;
    bool offsetcal_ok = false;
    bool gaincal_ok = false;
    bool reserved1_ok = false;
    bool reserved2_ok = false;
    bool lock_ok = false;
    bool reserved3_ok = false;
    bool crccfg_ok = false;
};

uint32_t MCP3561::data_counter;
uint32_t MCP3561::data_points_to_sample = 1;
byte MCP3561::adc_data[3];
uint32_t MCP3561::adc_sample = 0;
uint32_t MCP3561::synchronization_data[MAX_SYNCHRONIZATION_POINTS];
uint32_t MCP3561::synchronization_counter = 0;

void MCP3561::writeRegisterDefaults(void) {
  byte command_byte = 0;
  byte data_byte = 0;
  
  // First write to CONFIG0 register
  command_byte = CONFIG0_WRITE;
  // Configure the ADC to read use its own internal clock and output it to the MCLK pin
  data_byte = CONFIG0_CLK_SEL_EXT;
  // Ronfigure the ADC to be in conversion mode rather than shutdown mode
  data_byte |= CONFIG0_ADC_MODE_CONV;
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command_byte);
  SPI.transfer(data_byte);
  digitalWrite(CS_PIN, HIGH);
  // TODO

  // Next write to CONFIG1 register. 
  command_byte = CONFIG1_WRITE;
  // This gives a oversampling ratio of 32 with sampling at MCLK
  data_byte = CONFIG1_OSR_256;
  
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command_byte);
  SPI.transfer(data_byte);
  digitalWrite(CS_PIN, HIGH);
  // TODO

  // Next, write to CONFIG2 register. No need to change anything, defaults OK.

  // Next, write to CONFIG3 register.
  command_byte = CONFIG3_WRITE;
  // Change ADC mode to continuous conversion
  data_byte = CONFIG3_CONV_MODE_CONTINUOUS;
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command_byte);
  SPI.transfer(data_byte);
  digitalWrite(CS_PIN, HIGH);

  // Next, write to the IRQ register to enable a conversion to trigger an interrupt.
  command_byte = IRQ_WRITE;
  data_byte = IRQ_MODE_HIGH;
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command_byte);
  SPI.transfer(data_byte);
  digitalWrite(CS_PIN, HIGH);
}

void MCP3561::Reset(void) {
  MCP3561::synchronization_counter = 0;
  writeRegisterDefaults();
  MCP3561::data_counter = 0;
  MCP3561::data_points_to_sample = 1;
}
void MCP3561::readRegisters(void) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(IREAD_COMMAND);
  // First, read the ADCDATA register
  adc_data[2] = SPI.transfer(0);
  adc_data[1] = SPI.transfer(0);
  adc_data[0] = SPI.transfer(0);
  adc_sample = (adc_data[2] << 16) | (adc_data[1] << 8) | (adc_data[0]);

  config0_data = SPI.transfer(0);
  config1_data = SPI.transfer(0);
  config2_data = SPI.transfer(0);
  config3_data = SPI.transfer(0);
  irq_data = SPI.transfer(0);
  mux_data = SPI.transfer(0);
  byte temp0 = SPI.transfer(0);
  byte temp1 = SPI.transfer(0);
  byte temp2 = SPI.transfer(0);
  scan_data = (temp2 << 16) | (temp1 << 8) | temp0;
  temp0 = SPI.transfer(0);
  temp1 = SPI.transfer(0);
  temp2 = SPI.transfer(0);
  timer_data = (temp2 << 16) | (temp1 << 8) | temp0;
  temp0 = SPI.transfer(0);
  temp1 = SPI.transfer(0);
  temp2 = SPI.transfer(0);
  offsetcal_data =  (temp2 << 16) | (temp1 << 8) | temp0;
  temp0 = SPI.transfer(0);
  temp1 = SPI.transfer(0);
  temp2 = SPI.transfer(0);
  gaincal_data = (temp2 << 16) | (temp1 << 8) | temp0;
  temp0 = SPI.transfer(0);
  temp1 = SPI.transfer(0);
  temp2 = SPI.transfer(0);
  reserved_1_data = (temp2 << 16) | (temp1 << 8) | temp0;
  reserved_2_data = SPI.transfer(0);
  lock_data = SPI.transfer(0);
  temp0 = SPI.transfer(0);
  temp1 = SPI.transfer(0);
  reserved_3_data = (temp1 << 8) | temp0;
  temp0 = SPI.transfer(0);
  temp1 = SPI.transfer(0);
  crccfg_data = (temp1 << 8) | temp0;
  digitalWrite(CS_PIN, HIGH);

}

// PARTIALLY COMPLETE. DOES NOT VERIFY ALL REGISTERS.
void MCP3561::verifyRegisters(void) {
  if(config0_data == 0b00000011) config0_ok = true;
  if(config1_data == 0b00001100) config1_ok = true;
  if(config2_data == 0b10001011) config2_ok = true;
  if(config3_data == 0b11000000) config3_ok = true;
  if(irq_data == 0b00110111) irq_ok = true;

  bool all_ok = config0_ok && config1_ok && config2_ok && config3_ok && irq_ok;

  if(all_ok == true) {
    Serial.println("ALL REGISTERS OK.");
  }
  else
  {
    Serial.println("SOME REGISTER NOT OK. REGISTER TABLE:");
    Serial.print("CONFIG0: ");
    Serial.println(config0_ok ? "OK" : "BAD");
    Serial.print("CONFIG1: ");
    Serial.println(config1_ok ? "OK" : "BAD");
    Serial.print("CONFIG2: ");
    Serial.println(config2_ok ? "OK" : "BAD");
    Serial.print("CONFIG3: ");
    Serial.println(config3_ok ? "OK" : "BAD");
    Serial.print("IRQ: ");
    Serial.println(irq_ok ? "OK" : "BAD");
  }
  
}

void MCP3561::printRegisters(void) {
  Serial.print("ADCDATA Register: ");
  Serial.print(adc_data[2], BIN);
  Serial.print(adc_data[1], BIN);
  Serial.println(adc_data[0], BIN);
  Serial.print("CONFIG0: ");
  Serial.println(config0_data, BIN);
  Serial.print("CONFIG1: ");
  Serial.println(config1_data, BIN);
  Serial.print("CONFIG2: ");
  Serial.println(config2_data, BIN);
  Serial.print("CONFIG3: ");
  Serial.println(config3_data, BIN);
  Serial.print("IRQ: ");
  Serial.println(irq_data, BIN);
  Serial.print("MUX: ");
  Serial.println(mux_data, BIN);
  Serial.print("SCAN: ");
  Serial.println(scan_data, BIN);
  Serial.print("TIMER: ");
  Serial.println(scan_data, BIN);
  Serial.print("OFFSETCAL: ");
  Serial.println(scan_data, BIN);
  Serial.print("GAINCAL: ");
  Serial.println(scan_data, BIN);
  Serial.print("RESERVED1: ");
  Serial.println(scan_data, HEX);
  Serial.print("RESERVED2: ");
  Serial.println(scan_data, HEX);
  Serial.print("LOCK: ");
  Serial.println(scan_data, BIN);
  Serial.print("RESERVED3: ");
  Serial.println(scan_data, HEX);
  Serial.print("CRCCFG: ");
  Serial.println(scan_data, BIN);
  Serial.println("-----------------------------");
}

void MCP3561::readADCData(void) {
  if(MCP3561::data_counter < MCP3561::data_points_to_sample) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(SREAD_DATA_COMMAND);
    MCP3561::adc_data[0] = SPI.transfer(0);
    MCP3561::adc_data[1] = SPI.transfer(0);
    MCP3561::adc_data[2] = SPI.transfer(0);
    MCP3561::adc_sample = (adc_data[2] << 16) | (adc_data[1] << 8) | (adc_data[0]);
    digitalWrite(CS_PIN, HIGH);
    Serial.write(MCP3561::adc_data, 3);
    MCP3561::data_counter += 1;
  }
  else {
    detachInterrupt(digitalPinToInterrupt(IRQ_PIN));
    detachInterrupt(digitalPinToInterrupt(EXTERNAL_SYNC_PIN));
    MCP3561::data_counter = 0;
  }
}

void MCP3561::recordSync() {
  MCP3561::synchronization_data[MCP3561::synchronization_counter] = MCP3561::data_counter;
  MCP3561::synchronization_counter += 1;
}
