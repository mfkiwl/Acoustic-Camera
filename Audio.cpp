// Default libraries
#include <iostream>
#include <cmath>
#include <complex>
#include <vector>
#include <math.h>
#include <iomanip>
#include <sys/mman.h>

// User headers
#include "PARAMS.h"
#include "sharedMemory.h"
#include "Audio.h"

using namespace std;

//==============================================================================================

// Initialize upper and lower frequencies
int lowerFrequency;
int upperFrequency; 

//==============================================================================================

// Needs to be 3D for input***
// 3rd dimension is buffered data***
// Will be made 3D after data acquisition is made***
vector<vector<float>> DATA = // For testing***
{
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 1, 1}
};

//==============================================================================================

int main()
{
	//==============================================================================================

	// Initialize shared memory class for Audio
	sharedMemory audioData(AUDIO_SHM, AUDIO_SEM_1, AUDIO_SEM_2, NUM_ANGLES, NUM_ANGLES);

	if(!audioData.createAll())
	{
		cerr << "1. createAll failed.\n";
	}

	// Initialize shared memory class for userConfigs
	sharedMemory userConfigs(CONFIG_SHM, CONFIG_SEM_1, CONFIG_SEM_2, NUM_CONFIGS, 1);

	if(!userConfigs.openAll())
	{
		cerr << "1. openAll failed.\n";
	}

	//==============================================================================================

	// Initialize variables for audio handling
    snd_pcm_t *pcm_handle;
    snd_pcm_uframes_t frames;
    int pcm;

    // 3D vector to hold float data for M_AMOUNT x N_AMOUNT microphones and FFT_SIZE samples
    vector<vector<vector<float>>> audioDataIn(M_AMOUNT, vector<vector<float>>(N_AMOUNT, vector<float>(FFT_SIZE)));

	//==============================================================================================

    // Setup audio
    pcm = setupAudio(&pcm_handle, &frames);
    if (pcm < 0) 
    {
        return 1;  // Exit if setup fails
    }

	// Initializes constants for later use
	vector<cfloat> complexVector1;
	vector<vector<cfloat>> complexVector2;
	constantCalcs(complexVector1, complexVector2);
	
	// Initializes variables for user configs
	int bandTypeSelection = 0;
	int thirdOctaveBandSelection = 0;
	int fullOctaveBandSelection = 0;
	bool isRecording = 0;

	vector<int> USER_CONFIGS(NUM_CONFIGS, 0); // Main user config array

	// Initialize variables for audio data 
	vector<vector<float>> audioDataFFT; // Data after FFT
	vector<vector<float>> audioDataOut; // Fully processed data

	//==============================================================================================

	/* Switch case legend
	dont change legend if removing frequencies. just limit user input
	potentially remove lower frequencies due to too big bins***
	potentially remove higher frequencies due to mic response***
	
	Full Octave:
	0 - 63 Hz
	1 - 125 Hz
	2 - 250 Hz
	3 - 500 Hz
	4 - 1000 Hz
	5 - 2000 Hz
	6 - 4000 Hz
	7 - 8000 Hz
	8 - 16000 Hz
	
	Third Octave:
	0  - 50 Hz
	1  - 63 Hz
	2  - 80 Hz
	3  - 100 Hz
	4  - 125 Hz
	5  - 160 Hz
	6  - 200 Hz
	7  - 250 Hz
	8  - 315 Hz
	9  - 400 Hz
	10 - 500 Hz
	11 - 630 Hz
	12 - 800 Hz
	13 - 1000 Hz
	14 - 1250 Hz
	15 - 1600 Hz
	16 - 2000 Hz
	17 - 2500 Hz
	18 - 3150 Hz
	19 - 4000 Hz
	20 - 5000 Hz
	21 - 6300 Hz
	22 - 8000 Hz
	23 - 10000 Hz
	24 - 12500 Hz
	25 - 16000 Hz
	26 - 20000 Hz	
	*/
	
	//==============================================================================================
	
	// Main loop
	while (1)
	{
		// Read USER_CONFIGS and set relevant configs
		if(!userConfigs.read(USER_CONFIGS))
		{
			cerr << "1. read failed.\n";
		}

		/* Configs Legend
		0. 0 = Broadband
		   1 = Full octave
		   2 = Third Octave 
		1. 0 - 8 Full octave band selection
		2. 0 - 26 Third octave band selection
		3. 0 = Is not recording
		   1 = Is recording
		4. 
		5. 
		*/

		bandTypeSelection = USER_CONFIGS[0];
		fullOctaveBandSelection = USER_CONFIGS[1];
		thirdOctaveBandSelection = USER_CONFIGS[2];
		isRecording = USER_CONFIGS[3];

		//==============================================================================================

		// Read data from microphones
        pcm = captureAudio(audioDataIn, pcm_handle);
        if (pcm < 0) 
        {
            cerr << "Error capturing audio" << endl;
            // Decide whether to break or continue
            break;
        }

		//==============================================================================================

		// Change input to audioDataIn*******
		// User Configs
		switch (bandTypeSelection)
		{
			// Broadband
			case 0:
				arrayFactor(DATA, audioDataOut); // *** for testing only with 2-D array

				//lowerFrequency = 1; upperFrequency = 511;
			break;
			
			//==============================================================================================
			
			// Full Octave Band
			case 1:
			
				switch (fullOctaveBandSelection)
				{
					// 63 Hz
					case 0:	lowerFrequency = 1;   upperFrequency = 3;   break;
						
					// 125 Hz
					case 1:	lowerFrequency = 2;   upperFrequency = 5;   break;
						
					// 250 Hz
					case 2:	lowerFrequency = 4;   upperFrequency = 9;   break;
					
					// 500 Hz
					case 3:	lowerFrequency = 8;   upperFrequency = 17;  break;

					// 1000 Hz
					case 4:	lowerFrequency = 16;  upperFrequency = 33;  break;
					
					// 2000 Hz
					case 5:	lowerFrequency = 32;  upperFrequency = 66;  break;

					// 4000 Hz
					case 6:	lowerFrequency = 65;  upperFrequency = 132; break;

					// 8000 Hz
					case 7:	lowerFrequency = 131; upperFrequency = 264; break;

					// 16000 Hz
					case 8:	lowerFrequency = 263; upperFrequency = 511; break;	
				} // end Full Octave Band
				
			break;
			
			//==============================================================================================
			
			// Third Octave Band
			case 2:
			
				switch (thirdOctaveBandSelection)
				{
					// 50 Hz
					case 0: lowerFrequency = 1;   upperFrequency = 1;   break; // idk

					// 63 Hz
					case 1: lowerFrequency = 2;   upperFrequency = 2;   break; // idk

					// 80 Hz
					case 2: lowerFrequency = 1;   upperFrequency = 3;   break; // idk
				
					// 100 Hz
					case 3: lowerFrequency = 2;   upperFrequency = 3;   break;

					// 125 Hz
					case 4: lowerFrequency = 2;   upperFrequency = 4;   break;

					// 160 Hz
					case 5: lowerFrequency = 3;   upperFrequency = 5;   break;

					// 200 Hz
					case 6: lowerFrequency = 4;   upperFrequency = 6;   break;

					// 250 Hz
					case 7: lowerFrequency = 5;   upperFrequency = 7;   break;

					// 315 Hz
					case 8: lowerFrequency = 6;   upperFrequency = 9;   break;

					// 400 Hz
					case 9: lowerFrequency = 8;   upperFrequency = 11;  break;

					// 500 Hz
					case 10: lowerFrequency = 10;  upperFrequency = 14;  break;

					// 630 Hz
					case 11: lowerFrequency = 13;  upperFrequency = 17;  break;

					// 800 Hz
					case 12: lowerFrequency = 16;  upperFrequency = 21;  break;

					// 1000 Hz
					case 13: lowerFrequency = 20;  upperFrequency = 27;  break;

					// 1250 Hz
					case 14: lowerFrequency = 26;  upperFrequency = 33;  break;

					// 1600 Hz
					case 16: lowerFrequency = 32;  upperFrequency = 42;  break;

					// 2000 Hz
					case 17: lowerFrequency = 41;  upperFrequency = 52;  break;

					// 2500 Hz
					case 18: lowerFrequency = 51;  upperFrequency = 66;  break;
	
					// 3150 Hz
					case 19: lowerFrequency = 65;  upperFrequency = 83;  break;

					// 4000 Hz
					case 20: lowerFrequency = 82;  upperFrequency = 104; break;

					// 5000 Hz
					case 21: lowerFrequency = 103; upperFrequency = 131; break;

					// 6300 Hz
					case 22: lowerFrequency = 130; upperFrequency = 165; break;

					// 8000 Hz
					case 23: lowerFrequency = 164; upperFrequency = 207; break;

					// 10000 Hz
					case 24: lowerFrequency = 206; upperFrequency = 261; break;

					// 12500 Hz
					case 25: lowerFrequency = 260; upperFrequency = 329; break;
					
					// 16000 Hz
					case 26: lowerFrequency = 328; upperFrequency = 413; break;
					
					// 20000 Hz
					case 27: lowerFrequency = 412; upperFrequency = 511; break;			
				} // end Third Octave Band		
			
			//==============================================================================================
			
			// Filters data to remove unneeded frequencies and sums all frequency bins
			// Applies per-band calibration
			//FFTSum(audioDataIn, audioDataFFT); ***disabled until data acquisition is made***

			// Does beamforming algorithm and converts to gain
			//arrayFactor(filteredData, audioDataOut);	***disabled until data acquisition is made***	
			
			break; // end frequency selection
		} // end main loop
		
		//==============================================================================================
		
		// Output data to Video script
		if(!audioData.write2D(audioDataOut))
		{
			cerr << "1. write2D failed.\n";
		}
	}
	
	/*
	// Print gain array in .csv format (debugging)
	for (int index1 = 0; index1 < ANGLE_AMOUNT; ++index1)
	{
		for (int index2 = 0; index2 < ANGLE_AMOUNT; ++index2)
		{
			// Output gain value followed by comma
			cout << gain[index1][index2];
			if (index2 < ANGLE_AMOUNT - 1)
			{
				cout << ",";
			}
		}
		cout << endl;
	} // end print gain
	*/

    // Clean up ALSA
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);

	// Clean up shm
	audioData.closeAll();
	audioData.~sharedMemory();
	userConfigs.closeAll();
	userConfigs.~sharedMemory();

} // end main