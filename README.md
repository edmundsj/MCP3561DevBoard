MCP3561 Development Board
============================

Getting Started
------------------
NOTE: To run unit tests with a different sampling frequency than 4.8kHz, you need to change the sampling frequency in the unit test suite. Run:

.. code-block::
	
	python -m unittest discover

This should cause 20 unit tests to be run (1 may be skipped). Two should fail if the EXT_SYNC signal is not receiving a 1kHz pulse train, and 17 should pass.


Version 1 Features
---------------------------
- 3.3V on-board reference, with low-noise 3.3V digital and analog supplies
- Single-ended input voltage from 0-3.3V
- Sampling rate tested up to 10kHz

Measured with MCLK=10MHz, OSR=256, fs=9.76kHz, Av=1
- Rms voltage noise 9.8uV, PSD=143nV/rtHz, 0.2kHz to fs/2

Measured with MCLK=10MHz, OSR=256, fs=9.76kHz, Av=10
- Rms voltage noise is 13.4uV, PSD=195nV/rtHz, 0.2kHz to fs/2

Version 1 Data
----------------
![Fig. 1](figures/MCP3561DevBoard_grounded_representative.png)
![Fig. 2](figures/MCP3561DevBoard_grounded.png)

Fig. 1 and 2 show the time-domain and frequency-domain representation of 0.1s
of measured signal with the input to the board grounded.

![Fig. 3](figures/MCP3561DevBoard_10mVrms_representative.png)
![Fig. 4](figures/MCP3561DevBoard_10mVrms.png)

Fig. 3 and 4 show the same plots with an input signal of 10mVrms (Av=10) at
105Hz.

![Fig. 3](figures/MCP3561DevBoard_100mVrms_representative.png)
![Fig. 4](figures/MCP3561DevBoard_100mVrms.png)

Fig. 3 and 4 show the same plots with an input signal of 100mVrms (Av=10) at
205Hz. 

Issues in version 1
---------------------
- [FIXED v2] U3 (TLV opamp) package is incorrect - TSOT353, actual package is SOT23-5.
- 1/f noise of TLV opamp is appreciable when configured for a gain of 10. 

