MCP3561 Development Board
============================

Getting Started
------------------
NOTE: To run unit tests with a different sampling frequency than 4.8kHz, you need to change the sampling frequency in the unit test suite. Run:

.. code-block::
	
	python -m unittest discover

This should cause 20 unit tests to be run (1 may be skipped). Two should fail if the EXT_SYNC signal is not receiving a 1kHz pulse train, and 17 should pass.

Issues in version 1
---------------------
- [FIXED v2] U3 (TLV opamp) package is incorrect - TSOT353, actual package is SOT23-5.
- 1/f noise of TLV opamp is appreciable when configured for a gain of 10. 

Version 1 Features
---------------------------
- 3.3V on-board reference, with low-noise 3.3V digital and analog supplies
- Single-ended input voltage from 0-3.3V
- Sampling rate tested up to 10kHz

Measured with MCLK=10MHz, OSR=256, fs=9.76kHz, Av=1
- Rms voltage noise 9.8uV, PSD=143nV/rtHz, 0.2kHz to fs/2

Measured with MCLK=10MHz, OSR=256, fs=9.76kHz, Av=10
- Rms voltage noise is 13.4uV, PSD=195nV/rtHz, 0.2kHz to fs/2



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
- When applying an input sinusoid wih a shielded cable, it looks like the noise goes up to high heaven. This looks like it is low-frequency noise from the signal generator itself, as there is no noise present when I turn the output of the signal generator off. Fortunately this shouldn't matter for us, as the signal generator is only used to apply voltages to my devices, but this is very good to know.

Debugging TIA
________________
- After testing the ADC, with the ADC + TIA, the noise goes up to 13uVrms / 190nV/rtHz, which is *exactly* what I would expect of a 1MOhm resistor. There is now a -110dBV 60Hz signal present in the spectra which was not there before. This indicates that my current TIA is coupling this noise, likely via the cables, into the input of the ADC. But DAMN the noise is close to what I would expect. The single-ended-to-differential discrete PCB appears to have been the main noise culprit.
- I can now relegate my old ADC, the AD7766, to the ashheap of history. I still need to properly wire the motor controller, but that frees up my old teensy as well. The -110dBV harmonic is now up to -105dBV.
- With the photodiode attached, the 60Hz harmonic goes up to -93dBV, and the 120Hz harmonic goes up to -90dBV. This goe up dramatically when I remove the Faraday cage, though I suspect this has to do with light.
- When I mount the new Si photodiode, the noise is just as bad. This tells me it's not a shielding problem, but an ambient light problem. When i cover the device with my hand, it goes away.
- After putting the photodiode in a cardboard box, the noise went down to ~-80dBV, and after wrapping that mostly in aluminum foil, the noise went down to -106dBV, indicating we are now limited by the TIA. If we want to get down to below -110dBV, we need to fabricate a new TIA. This means we can do shot-noise limited measurements at 60Hz above 70uA. But with a TIA redesign, we can bring that interferer down below the TIA noise floor, and do shot-noise limited measurements down to 70nA. With the losses in the system, this is probably going to be necessary. I should send out a new TIA design with the new Teensy and thermal management PCB designs.

Debugging Motor
__________________
- It looks like the Teensy thinks the motor is enabled, but the motor is not getting any current.
- For some reason the teensy is not consistently writing to the enable/disable pin when I tell it to. When the device is reset, it does not, but when the enable/disable function is called directly, it does. This means there is something in the reset code that is setting the motor back to an enabled state. Indeed, that was the case.
- Now, it looks like my waitForMotor() function is not working at all - my test code is not waiting long enough fro my motor to rotate. I suspect the teensy does not respond fast enough and when the python code queries it, the motor has not set the rotating bit to true yet. We may need to add a tiny delay up-front to fix this.
- Nope this appears not to be the issue. The code appears to run just fine when I run it in a python shell. Here is my guess: the time.sleep function does not work in the unittest module. Nope, this is not the case either. Ah, I think the motor is not on. Yup, adding an internal check for whether the motor is on fixed this.
- When rotating the motor there is a -115dBV interferer at 500Hz and a -118dBV harmonic at 1kHz, -122dBV at 1.5kHz. I suspect this is due to the motor. This means we probably don't care if the motor is enabled. but we do care if it's actually rotating.
- OK, now we have got the motor completely working once again.
- Today there is a -100dBV interferer at 60Hz coupling in from the TIA and a -110dBV interferer at 180Hz. When I turn on the Tekpower DC supply which connects to the motor there is also a -97dBV interferer at 4kHz, which is not present when the motor is disabled. So today we just have slightly worse 60Hz noise on the TIA. Not unexpected. With the photodiode plugged in, this stays about the same but there is also now a -120dBV interferer at 120Hz, which is indicative that I need a better aluminum foil box. With extra tinfoil, the interferer dissapeared, indicating it was in fact due to overhead lighting.
- The DC offset with the TIA connect is 1.678V, which is almost exactly where I expect it (1.65), off by 25mV (<1%) which is within the specs of the linear supply.

More Debugging
________________
- With a gain of 10, the 1/f noise starts to become visible. The downward
sloping of about 10dB from 0.5kHz to 5kHz is observed in the ADC's datasheet,
so I don't think this is a major problem.
