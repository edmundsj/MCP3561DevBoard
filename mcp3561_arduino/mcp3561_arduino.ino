#include <SPI.h>
#include "teensyDAQ.h"

struct scpi_parser_context ctx;
char line_buffer[256];
unsigned char read_length;

void setup() {
  scpi_init(&ctx);
  struct scpi_command* motor;
  struct scpi_command* sync;

  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "*IDN?", identify);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "MEASURE?", measure);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "CONFIGURE", configure);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "CONFIGURE?", queryConfiguration);
  scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "*RST", resetDevice);
  
  sync = scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "SYNC", NULL);
  scpi_register_command2(sync, SCPI_CL_CHILD, "NUMPOINTS?", sendSyncNumPoints);
  scpi_register_command2(sync, SCPI_CL_CHILD, "DATA?", sendSyncData);

  motor = scpi_register_command2(ctx.command_tree, SCPI_CL_SAMELEVEL, "MOTOR", NULL);
  scpi_register_command2(motor, SCPI_CL_CHILD, "POSITION", setPosition);
  scpi_register_command2(motor, SCPI_CL_CHILD, "POSITION?", queryPosition);
  scpi_register_command2(motor, SCPI_CL_CHILD, "DIRECTION?", queryDirection);
  scpi_register_command2(motor, SCPI_CL_CHILD, "ROTATE", rotateMotor);
  scpi_register_command2(motor, SCPI_CL_CHILD, "ROTATE?", getMotorRotating);
  scpi_register_command2(motor, SCPI_CL_CHILD, "ENABLE", enableMotor);
  scpi_register_command2(motor, SCPI_CL_CHILD, "DISABLE", disableMotor);
  scpi_register_command2(motor, SCPI_CL_CHILD, "ENABLED?", getMotorEnabled);
  scpi_register_command2(motor, SCPI_CL_CHILD, "PERIOD", setMotorPeriod);
  scpi_register_command2(motor, SCPI_CL_CHILD, "PERIOD?", getMotorPeriod);
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(CS_PIN, OUTPUT);
  pinMode(IRQ_PIN, INPUT);
  pinMode(MCLK_PIN, OUTPUT);
  pinMode(MISO_PIN, INPUT);
  analogWriteFrequency(MCLK_PIN, 10240000);
  analogWrite(MCLK_PIN, 128);
  SPI.begin();
  
  delay(500); // I think we need an initial delay.
  adc.writeRegisterDefaults();

  adc.readRegisters();
  //adc.verifyRegisters();
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
