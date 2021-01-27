#include <SPI.h>
#include "teensyDAQ.h"

struct scpi_parser_context ctx;
char line_buffer[256];
unsigned char read_length;

void setup() {
  scpi_init(&ctx);
  struct scpi_command* sync;

  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", identify);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "MEASURE?", measure);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "CONFIGURE", configure);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "CONFIGURE?", queryConfiguration);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "*RST", resetDevice);
  
  sync = scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "SYNC", NULL);
  scpi_register_command2(sync, SCPI_CL_CHILD, "NUMPOINTS?", sendSyncNumPoints);
  scpi_register_command2(sync, SCPI_CL_CHILD, "DATA?", sendSyncData);
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(CS_PIN, OUTPUT);
  pinMode(IRQ_PIN, INPUT);
  pinMode(MCLK_PIN, INPUT);
  pinMode(MISO_PIN, INPUT);
  SPI.begin();
  
  delay(500); // I think we need an initial delay.
  writeRegisterDefaults();

  readRegisters();
  verifyRegisters();
}

void loop() {
  if(Serial.available()) {
    read_length = Serial.readBytesUntil('\n', line_buffer, 256);
    if(read_length > 0)
    {
      scpi_execute_command(&ctx, line_buffer, read_length);
    }
  }
}
