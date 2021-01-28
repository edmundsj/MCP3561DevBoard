MCP3561 Development Board
============================

Getting Started
------------------
- NOTE: To run unit tests with a different sampling frequency than 4.8kHz, you need to change the sampling frequency in the unit test suite.
Run:
.. code-block::
	
	python -m unittest discover

This should cause 20 unit tests to be run (1 may be skipped). Two should fail if the EXT_SYNC signal is not receiving a 1kHz pulse train, and 17 should pass.

Issues in version 1
---------------------
- [FIXED v2] U3 (TLV opamp) package is incorrect. TSOT353, actual package SOT23-5.

Test Data


Appendix - Notes
-------------------
- I will try to solder on U3 anyway despite its smaller package. I have a microscope, hopefully that will help :p.
- Got it. Damn that was not trivial, don't know how I would have done it without a microscope.
- All input voltages are as expected, 3.3V except for VOS, which is 0.8V (3.3V/4). With the labeled test points it was SO EASY to do this test. It took a total of like 5 seconds. Now we solder on the teensy and attempt to write software to communicate with the ADC.
- Soldering the ADC on applying flux liberally and a very small amount of solder to the tip was pretty trivial.
- Why is the ADC data 32 bits and not 24? Is it sending a checksum or something? Nope, the checksum is 16 bits. Nope, default length is 24 bits.
- Does this chip have no static write? God that's going to be annoying to deal with.
- I was seeing strange periodic pulses on the Teeny's supply - turns out the thing was not actually connected to ground! I left the pin floating because I didn't have a long-enough header and didn't realize that was where my ground pin was connected.
- We are sending a clock signal on the SPI SCK line, but we are not getting any data back from the ADC. We should verify that the registers we wrote have the correct values.
- The ADC is completely silent when I try to read its registers. They all contain nothing. I suspect the ADC is locked or I am not sending the right command.
- Much better. I forgot to send the address on the SPI bus. Stupid protocol. 
- I have verified that the CONFIG0-3 registers have the correct value. MCLK is going off at 4.69MHz, and pin 7 of the Teensy is being driven. The ADC data is in continuous conversion mode, but all the bits we are getting back are zero.
- When I plugged in the ADC initially today it was reading all zeros - this tells me that I am trying to write the register defaults too fast. I added a 100ms delay on startup prior to writing to try to fix this. This doesn't seem to work. 500ms does seem to work.
- For some reason our beginReadingData() function isn't working, we simply re-read the last ADC value over and over again.
- Our readRegisters() command works, but beginReadingData() does not. What's the difference? We are doing an incremental read in the one case and a static read in the other. Not sure why this would make a difference.
- So the static read is working, but the continuous read is not. It seems like the ADC is not continually generating new data (or updating the output register) until we send the read command. This is fine for now. This is fine for now.
- The master clock of the ADC is running at 4.691MHz, and the data appears to be ready at a frequency of 4.584kHz, which would seem to indicate there is one sample every 1024 clock cycles. According to the datasheet at this clock frequency and an oversampling ratio of 256, the data rate should be 4.581kHz, which is very close to what we see. This means the ADC is working as expected. The interrupt pin is also working well, so we need to implement that.
- Everything is now working, sampling at 4.8kHz. 
- Looks like I can sample at 36.8kHz, but the data is in steps of 180, not steps of 1 anymore. That's obnoxious. For now I'll just stick with a sampling rate of 4.8kHz and characterize the ADC, then switch to a higher sampling rate.
- For now, I have commented out all the motor and synchronization code, but even the identify code is not working. Ported code is now working.
- I will test using a 10MHz clock at the input

Unit Test Debugging
____________________
- I am having some issues getting 300,000 bytes back from the arduino. I suspect this is a time-out error, as we need to wait quite a long time to get back this data. I'm receiving around 40kB, which indicates we only take ~10 measurements. I'm going to try to do this manually.
- The teensy itself can send 100k samples no problem.
- Sure enough, our Measure() method is terminating too early. Fixed it, now it's working up to 100k measurements.
- All motor communication unit tests are now passing, as are all measurement tests. Still failing synchronization tests, which means I will have to solder on the connector. Time to do that.
- Today the measurement frequency is 4.58kHz. This is significantly lower than yesterday, it looks like the internal clock drifts quite a bit. Let's drive the clock using the Teensy to try to get a more reliable sampling rate.
- Synchronization is not currently working. I'm going to let that issue rest for now. The discrepancy is small. 
