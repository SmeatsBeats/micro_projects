#include "audio_reactive.h"
#include <Arduino.h>
#include <FastLED.h>
#include <vector>

//#include <paletteData.h>
//#include <LightEffects.h>

// could define params here... but they feel better within the class
// however, they may need to be accessed by main so changes can persist

// you want to average each fft bin of interest
// would also be cool to set num_readings independently for each one sheesh
// then you could fine tune how different octaves respond to transients etc.

int prev_num_readings;

const int num_bins = 16;
// objective is to make num readings variable 
// for a quick solution - initiate array to max value
// then just reduce num readings
// best to then deallocate unused space 



// did quick and dirty approach to test premise and it's sweet 
// definitely necessary to clear out extra values and reallocate the array
const int fft_num_readings = 200;
//int fft_readings[num_bins][fft_num_readings];  // the readings from the analog input
int fft_readIndex = 0;          // the index of the current reading 
//int fft_total[num_bins] = {0};              // the running total
//int fft_average[num_bins] = {0};            // the average

// use vectors instead?

// std::vector<int> arr(5); // Create a vector with initial size 5

// // ... Do something with the arr ...

// arr.clear(); // Clear the elements (size becomes 0)

// int newSize = 10; // New size for the vector
// arr.resize(newSize); // Resize the vector to size 10

// Use the arr with the new size as needed

std::vector<std::vector<int>> fft_readings(num_bins, std::vector<int>(fft_num_readings));

std::vector<int> fft_total(num_bins);
std::vector<int> fft_average(num_bins);

class React {
    public:
        // attributes 
        // create palette from paletteData.h
        //CRGBPalette16 rosy = rosebud;
        React() {
            // constructor
        };

        void runPattern(CRGB* leds, int NUM_LEDS, int fft_num_readings, int num_bins);
        void FFTAvg(int fft_num_readings);
        void initFFTAvg(int fft_num_readings, int num_bins);
        void plotBands();

    private:
};

void React::initFFTAvg(int fft_num_readings, int num_bins) {

    //int fft_readings[num_bins][fft_num_readings]; 
    fft_readIndex = 0;     

    // loop over bins

    // for (int bin = 0; bin < num_bins; bin++) {
    //     // reset scalar vals to 0 
    //     fft_total[bin] = 0; 
    //     fft_average[bin] = 0;
    // }    

    // vector approach 

    fft_total.clear();
    fft_average.clear();

    fft_total.resize(num_bins);
    fft_average.resize(num_bins);

    // Clearing the matrix (setting all elements to 0)
    for (auto& row : fft_readings) {
        row.clear(); // Clear each inner vector
    }
    fft_readings.clear(); // Clear the outer vector

    // Resizing the matrix to a new size (2x6)
    fft_readings.resize(num_bins, std::vector<int>(fft_num_readings));

}

void React::FFTAvg(int fft_num_readings) {
    // this will be used for all reactive patterns and should be moved to its own class eventually... 
    //// this should go in separate file and be called as function only for reactive presets
    // there is probably a way to achieve this upstream by modifying the fft itself... 

    // use same averaging approach for fft bins 
    // this probably belongs in a separate file 
    // also, the FFT is only necessary for reactive patterns - running it for 
    // non-reactive patterns is inefficient 

    //cannot allow 0 for averages - this will try to divide by 0 

    // TODO - the remote will still display one - when this becomes db driven - include encoder min, max and increment info 

    fft_num_readings = max(1, fft_num_readings);


    for (int bin = 0; bin < num_bins; bin++) {
        // Subtract the oldest value from the running sum - why?
        // see simple moving average
        // https://sciencing.com/calculate-exponential-moving-averages-8221813.html
        // otherwise it would infintely increase 
        // remember that this whole thing is basically in a loop 
        // here we loop through all the bins, while the readindex remains unchanged 
        // ex: consider one bin only, taking the average every 3 readings
        // input values are 2 3 4 2 1 5
        // on loop 1: fft_total is 0 and fft_readings at readIndex 0 is not defined so fft_total is 0;
        // the reading of value 2 is inserted at position 0 in fft_readings -> fft_readings[0] = 2; 
        // fft_total is 0 so this is added -> 0 + fft_readings[0] = 2; 
        // fft_readIndex is incremented to 1
        // the index is not reset because 1 is less than 3 
        // the average is taken as total/num_readings so 2/3 -- this is not correct because there is only one reading!
        // when fewer readings than fft_num_readings have been taken, this will always be the case 
        // is it resolved after more readings? 
        // loop 2: 
        // again there is no reading at fft_readings[fft_readIndex] so this does nothing to the total, which remains 2; 
        // the value of 3 is inserted into fft_readings[1]
        // this value of 3 is added to the total of 2 which gives 3 + 2 = 5
        // fft_readIndex is incremented to 2
        // which is less than 3 so we do not wrap 
        // the average is taken as 5/3 -- again there have only been 2 values so this is not correct to divide by 3
        // loop 3: 
        // nothing is removed from the total because there is no value at fft_readings[2] so the total remains 5
        // the value of 4 is inserted into fft_readings[2]
        // this value of 4 is added to the total of 5 which gives 4 + 5 = 9
        // fft_readIndex is incremented to 3
        // 3 >= 3 so readIndex is reset to 0 
        // the average is calculated as 9/3 = 3 -- this is now correct as there have indeed been 3 readings 
        // loop 4: 
        // the value stored in fft_readings[0] is 2. this is subtracted from the total so 9 - 2 = 7
        // the value of 2 is inserted into fft_readings[0] effectively replacing the oldest value which was 2 
        // the value at fft_readings[0] is added to the total which is 7 giving 2 + 7 = 9
        // fft_readIndex is incremented to 1
        // which is less than 3 so we do not wrap 
        // the average is calculated as 9/3 -- which is correct again because we removed the old value, 2, and replaced it with the new value which was the same
        // so there are still effectively 3 values in the running total which is then appropriately divided by 3 
        // ** effectively, the oldest value is removed from the FRONT of the array and the new value replaces it there - the order does not matter
        // loop 5: 
        // the value stored in fft_readings[1] is subtracted from the total removing the next oldest value which is 3 giving 10 - 3 = 7
        // the value of 1 is inserted into fft_readings[1] effectively replacing the next oldest value which was 3
        // the value at fft_readings[1] is added to the total which is 9 giving 1 + 9 = 10
        // fft_readIndex is incremented to 2
        // which is less than 3 so we do not wrap
        // the average is calculated as 10/3 -- which is correct again because we removed the old value, 3, and replaced it with the new value 
        // so there are still effectively 3 values in the running total which is then appropriately divided by 3 
        // this process continues to replace each value up to the fft_num_avgs value and then wraps and repeats again 
        // so... does increasing the number of readings change the smoothness? 
        // yes because the impact that one additional number can make relative to a larger total is minimized
        // so... what happens when fft_num_avgs is updated by the user, potentially in the middle of a loop?
        // the total does not change but the denominator does so for example, in loop 5 is the number of readings is changed to 5,
        // the total will be calculated as 10/5 = 2 where 10 was the sum of 3 numbers NOT 5 
        // probably best to just reset the whole process when number of readings is changed, though there may be a more clever approach 
        // to remove the correct number of old values for example

        // how does this remove the oldest value? fft_readIndex is empty on runs up to fft_num_readings - what about after?
        fft_total[bin] = fft_total[bin] - fft_readings[bin][fft_readIndex];

        fft_readings[bin][fft_readIndex] = fftResult[bin]; 
        
        // Add the new FFT data value to the running sum
        fft_total[bin] = fft_total[bin] + fft_readings[bin][fft_readIndex]; // Assuming fftResult contains FFT data for the bin
        
        // Update the running average for this bin
    // fft_average[bin] = fft_total[bin] / fft_num_readings; // You can change numReadings if needed
    } // END BINS

    // THIS BIT LOOPS BY NATURE OF BEING CALLED FROM THE MAIN LOOP 

    // Update the index for the next FFT data point
    fft_readIndex = fft_readIndex + 1;

    // Wrap around if necessary
    if (fft_readIndex >= fft_num_readings) {
        fft_readIndex = 0;
    }

    for (int bin = 0; bin < num_bins; bin++) {

    // Update the running average for this bin
        // fft_num_readings should be the number of readings taken between each average 
        // perhaps the denominator here should be readIndex? which tracks the actual number of readings before resetting 
        // but it does reset! and the arrays themselves don't - they just remove the oldest value... 
        // perhaps this is the correct denominator but only applies after the first wrap around? 
        fft_average[bin] = fft_total[bin] / fft_num_readings; // You can change numReadings if needed
    }
}

void React::runPattern(CRGB* leds, int NUM_LEDS, int fft_num_readings, int num_bins) {

    // call to print reading values to serial plotter
    // plotBands();


    //cannot allow 0 for averages - this will try to divide by 0 

    fft_num_readings = max(1, fft_num_readings);

    // if the number of readings has changed, reallocate the array 

    if (prev_num_readings != fft_num_readings) {
        initFFTAvg(fft_num_readings, num_bins);
    }

    // interesting putting this here 
    FFTAvg(fft_num_readings);

    int groups = 10;
    int lights = NUM_LEDS / groups;

    //kick flash 

    // if (fftResult[1] > 700) {

    //   fill_solid(leds, NUM_LEDS, CRGB::White);
    
    // }

    for (int r = 1; r <= groups; r++) {
      for (int i = r * lights - lights; i < r * lights; i++) {
        //leds[i] = CHSV(fftResult[r+3], 150, fftResult[r+3]);
        leds[i] = CHSV(fft_average[r+3], 255, fft_average[r+3]);
        //leds[i] = CHSV(90, 150, fftResult[i+3]);
      } // each LED
    }

    prev_num_readings = fft_num_readings;

}

void React::plotBands() {

  int startBand = 0;
  int endBand = 7;

  // could also print avg

  for(int i = startBand; i <= endBand; i++) {
    Serial.print("Band " + String(i) +  ":");
    //Serial.print(fftResult[i]); 
    if (i >= endBand) {
      Serial.println(fftResult[i]);
    }
    else {
      Serial.print(fftResult[i]); 
      Serial.print(",");
    }
   
  }
  
}