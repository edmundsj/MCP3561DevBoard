#include <scpiparser.h>
#include "mcp3561.h"



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
  writeRegisterDefaults();
  //motor.Reset();
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t measure(struct scpi_parser_context* context, struct scpi_token* command)
{
  //AD7766::synchronizationCounter = 0;
  data_counter = 0;
  Serial.print("#");
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), readADCData, FALLING);
  //attachInterrupt(digitalPinToInterrupt(EXTERNAL_SYNC_PIN), AD7766::recordSync, RISING);
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
  data_points_to_sample = (unsigned long) (commandData.value);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t queryConfiguration(struct scpi_parser_context* context, struct scpi_token* command)
{
  struct scpi_token* args = command;
  struct scpi_numeric commandData;
  //unsigned char output_value;

  while(args != NULL && args->type == 0)
  {
    args = args->next;
  }

  Serial.println(data_points_to_sample);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

scpi_error_t sendSyncNumPoints(struct scpi_parser_context* context, struct scpi_token* command)
{
  //Serial.println(AD7766::synchronizationCounter);
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}

/* Send the raw synchronization data - the measurements at which we have a L->H transition */
scpi_error_t sendSyncData(struct scpi_parser_context* context, struct scpi_token* command)
{
  Serial.print('#');
  /*
  for(int i = 0; i < AD7766::synchronizationCounter; i++) {
    Serial.write(AD7766::synchronizationData[i] >> 16); // MSB first
    Serial.write(AD7766::synchronizationData[i] >> 8);
    Serial.write(AD7766::synchronizationData[i]); // LSB last. Only send 24 bits, should be enough for even 100s of data.
  }
  */
  scpi_free_tokens(command);
  return SCPI_SUCCESS;
}
