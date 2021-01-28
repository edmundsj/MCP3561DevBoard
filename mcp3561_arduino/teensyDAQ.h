#include <scpiparser.h>
#include "mcp3561.h"
#include "stepper_motor.h"



/** Base class that implements communication to and from an external Serial or USB interface via
* According to the SCPI specification, the measurement process is broken down into several stages.
* First, there is configuration of the measurement done via the CONFigure command. The INITiate then
* physically performs the measurement, and FETCh? does any necessary postprocessing and returns the data.
* I will implement MEASURE, CONFigure, and FETCH, as I don't see a purpose to implement INITiate or READ.
*/
scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t measure(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t configure(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t queryConfiguration(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t resetDevice(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t sendSyncNumPoints(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t sendSyncData(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t setPosition(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t queryPosition(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t setDirection(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t queryDirection(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t rotateMotor(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t getMotorRotating(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t disableMotor(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t enableMotor(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t getMotorEnabled(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t setMotorPeriod(struct scpi_parser_context* context, struct scpi_token* command);
scpi_error_t getMotorPeriod(struct scpi_parser_context* context, struct scpi_token* command);

StepperMotor motor;
MCP3561 adc;
IntervalTimer motorTimer;


void interruptRotate(void) {
  motor.Rotate();

  // If we are out of steps or we get the command to stop rotating by having the motor disabled
  if(motor.stepsRemaining <= 0 || motor.motorEnabled == false) {
    motorTimer.end();
    motor.motorRotating = false;
  }
}

scpi_error_t identify(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println("OIC,Embedded SCPI Example,1,10");
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t resetDevice(struct scpi_parser_context* context, struct scpi_token* command)
{
  detachInterrupt(digitalPinToInterrupt(IRQ_PIN));
  detachInterrupt(digitalPinToInterrupt(EXTERNAL_SYNC_PIN));
  adc.Reset();
  motor.Reset();
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t measure(struct scpi_parser_context* context, struct scpi_token* command)
{
  //AD7766::synchronizationCounter = 0;
  adc.data_counter = 0;
  Serial.print("#");
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), adc.readADCData, FALLING);
  attachInterrupt(digitalPinToInterrupt(EXTERNAL_SYNC_PIN), adc.recordSync, RISING);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t configure(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args = command;
  struct scpi_numeric commandData;
  //unsigned char output_value;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  commandData = scpi_parse_numeric(args->value, args->length, 1, 1, 25e6);
  MCP3561::data_points_to_sample = (unsigned long) (commandData.value);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t queryConfiguration(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args = command;
  //struct scpi_numeric commandData;
  //unsigned char output_value;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  Serial.println(MCP3561::data_points_to_sample);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t sendSyncNumPoints(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(MCP3561::synchronization_counter);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/* Send the raw synchronization data - the measurements at which we have a L->H transition */
scpi_error_t sendSyncData(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.print('#');
  
  for(uint16_t i = 0; i < adc.synchronization_counter; i++) {
    Serial.write(adc.synchronization_data[i] >> 16); // MSB first
    Serial.write(adc.synchronization_data[i] >> 8);
    Serial.write(adc.synchronization_data[i]); // LSB last. Only send 24 bits, should be enough for even 100s of data.
  }
  
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t setPosition(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args = command;
  struct scpi_numeric commandData;
  
  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }
  commandData = scpi_parse_numeric(args->value, args->length, 0, 0, 10e6);
  motor.motorPosition = (uint32_t) (commandData.value);
  
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t queryPosition(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(motor.motorPosition);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t queryDirection(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(motor.motorDirection);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t rotateMotor(struct scpi_parser_context* context, struct scpi_token* command)
{
  // do some stuff with the motor controller
  struct scpi_token* args = command;
  struct scpi_numeric commandData;
  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }
  
  commandData = scpi_parse_numeric(args->value, args->length, 0, -1e6, 1e6);
  int motorSteps = int (commandData.value);
  
  motor.beginRotation(motorSteps);
  motorTimer.begin(interruptRotate, 1000*motor.motorPeriod);
  
  //motor.Rotate(motorSteps);
  
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t disableMotor(struct scpi_parser_context* context, struct scpi_token* command)
{
  motor.Disable();
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t enableMotor(struct scpi_parser_context* context, struct scpi_token* command)
{
  motor.Enable();
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t setMotorPeriod(struct scpi_parser_context* context, struct scpi_token* command)
{
  // do some stuff with the motor controller
  struct scpi_token* args = command;
  struct scpi_numeric commandData;
  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }
  
  commandData = scpi_parse_numeric(args->value, args->length, 0, 1, 1e6);
  uint32_t motorPeriod = uint32_t (commandData.value);
  
  motor.motorPeriod = motorPeriod;
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t getMotorPeriod(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(motor.motorPeriod);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t getMotorEnabled(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(motor.motorEnabled);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t getMotorRotating(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.println(int(motor.motorRotating));
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}
