/*

	Example of use of the FFT library to compute FFT for a signal sampled through the ADC.
        Copyright (C) 2018 Enrique Condés and Ragnar Ranøyen Homb

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <Arduino.h>
#include "arduinoFFT.h"
#include <map>
#include "FastLED.h"

#define LED_DT 25                                             // Data pin to connect to the strip.
//#define LED_CK 11                                             // Clock pin for WS2801 or APA102.
#define COLOR_ORDER GRB                                       // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812     
#define NUM_LEDS 10
struct CRGB leds[NUM_LEDS];

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType);

#define CHANNEL 33
#define NUM_BANDS 10
#define NOISE 400

const uint16_t samples = 128; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 9000; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long newTime;

int bandValues[NUM_BANDS] = {0}; 

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup()
{
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Ready");
  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500); 
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
}

void loop()
{
  // Sample the audio pin
  for (int i = 0; i < samples; i++) {
    newTime = micros();
    vReal[i] = analogRead(CHANNEL); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* chill */ }
  }


  FFT.DCRemoval();

  /* Print the results of the sampling according to time */
  //Serial.println("Data:");
  //PrintVector(vReal, samples, SCL_TIME);
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
  //Serial.println("Weighed data:");
  //PrintVector(vReal, samples, SCL_TIME);
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD); /* Compute FFT */
  //Serial.println("Computed Real values:");
  //PrintVector(vReal, samples, SCL_INDEX);
  //Serial.println("Computed Imaginary values:");
  //PrintVector(vImag, samples, SCL_INDEX);
  FFT.ComplexToMagnitude(vReal, vImag, samples); /* Compute magnitudes */
  //Serial.println("Computed magnitudes:");
  // this is a bitshift to the right by 1; seems equivalent to dividing by 2.... in keeping with Nyquist
  //PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
  //double x = FFT.MajorPeak(vReal, samples, samplingFrequency);
  //Serial.println(x, 6); //Print out what frequency is the most dominant.
  //while(1); /* Run Once */
  //delay(2000); /* Repeat after delay */

  

  // Reset bandValues[]
  for (int i = 0; i<NUM_BANDS; i++){
    bandValues[i] = 0;
  }

  // map frequencies along LED strip 

  for(int i = 2; i < (samples/2); i++) {
    if (vReal[i] > NOISE) {
      /*
      //10 bands, 4.5kHz top band
      if (i <= 12) bandValues[0]  += (int)vReal[i];
      if (i > 12 && i<=18) bandValues[1] += (int)vReal[i];
      if (i > 18 && i<=29) bandValues[2] += (int)vReal[i];
      if (i > 29 && i<=45) bandValues[3] += (int)vReal[i];
      if (i > 45 && i<=70) bandValues[4] += (int)vReal[i];
      if (i > 70 && i<=110) bandValues[5] += (int)vReal[i];
      if (i > 110 && i<=171) bandValues[6] += (int)vReal[i];
      if (i > 171 && i<=268) bandValues[7] += (int)vReal[i];
      if (i > 268 && i<=420) bandValues[8] += (int)vReal[i];
      if (i > 420) bandValues[9] += (int)vReal[i];

      */

      // 128 samples - this is way quicker and won't miss kicks, but I can't figure out wtf is going on with the other bands 

      //10 bands, 4.5kHz top band
      if (i<=1 )           bandValues[0]  += (int)vReal[i];
      if (i>1   && i<=2  ) bandValues[1]  += (int)vReal[i];
      if (i>2   && i<=4  ) bandValues[2]  += (int)vReal[i];
      if (i>4   && i<=6  ) bandValues[3]  += (int)vReal[i];
      if (i>6   && i<=9  ) bandValues[4]  += (int)vReal[i];
      if (i>9   && i<=14  ) bandValues[5]  += (int)vReal[i];
      if (i>14   && i<=21  ) bandValues[6]  += (int)vReal[i];
      if (i>21   && i<=34  ) bandValues[7]  += (int)vReal[i];
      if (i>34   && i<=52  ) bandValues[8]  += (int)vReal[i];
      if (i>52             ) bandValues[9]  += (int)vReal[i];



    } // NOISE

  } // bandVals


  // pattern 1

  

  // set each light according to magnitude
EVERY_N_MILLIS(10) { // maybe this will smooth it out a little?
   for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(map(bandValues[i], 0, 1500, 0, 255), 150, map(bandValues[i], 0, 1500, 0, 255));
    //leds[i] = CHSV(90, 150, bandValues[i]);
  }
}


  // pattern 2 

  if (bandValues[1] > 1000) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    
  }
  //FastLED.setBrightness(map(bandValues[0], 0, 5000, 0, 200));

  fadeToBlackBy(leds, NUM_LEDS, 60);
  FastLED.show();

  // will this work for serial plotter to get idea of vals for each band? 
  for (int i = 0; i < NUM_BANDS; i++) {
    Serial.println(bandValues[i]);
  }

  //delay(500);

}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;
    }
    Serial.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }
  Serial.println();
}