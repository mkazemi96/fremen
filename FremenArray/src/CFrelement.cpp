#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "fremenarray/CFrelement.hpp"

static constexpr bool debug = false;

int fremenSort(const void* i, const void* j) 
{
    if (((SFrelement*)i)->amplitude < ((SFrelement*)j)->amplitude) 
        return +1;
    return -1;
}

CFrelement::CFrelement()
{
    // Initialization of the frequency set
    for (int i = 0; i < NUM_PERIODICITIES; i++) {
        frelements[i].amplitude = frelements[i].phase = 0; 
    }
    for (int i = 0; i < NUM_PERIODICITIES; i++) {
        frelements[i].period = (24 * 3600) / (i + 1); 
    }
    gain = 0.5;
    firstTime = -1;
    lastTime = -1;
    measurements = 0;
}

CFrelement::~CFrelement()
{
}

// Adds new state observations at given times
int CFrelement::add(uint32_t times[], float states[], int length)
{
    if (measurements == 0 && length > 0) {
        for (int i = 0; i < NUM_PERIODICITIES; i++) {
            frelements[i].realStates = 0;
            frelements[i].imagStates = 0;
        }
        firstTime = times[0];
    }
    int duration = times[length - 1] - firstTime;
    int firstIndex = 0;

    // Discard already known observations 
    for (int i = 0; i < length; i++)
        if (times[i] <= lastTime)
            firstIndex++;
    int numUpdated = length - firstIndex;

    // Verify if there is an actual update
    if (numUpdated <= 0)
        return numUpdated;
    lastTime = times[length - 1];

    // Update the gains accordingly 
    float oldGain = 0;
    float newGain = 0;
    for (int j = firstIndex; j < length; j++)
        newGain += states[j];
    gain = (gain * measurements + newGain) / (measurements + length);

    // Recalculate spectral balance - this is beneficial if the process period does not match the length of the data
    if (oldGain > 0) {
        for (int i = 0; i < NUM_PERIODICITIES; i++) {
            frelements[i].realBalance  = gain * frelements[i].realBalance / oldGain;
            frelements[i].imagBalance  = gain * frelements[i].imagBalance / oldGain;
        }
    } else {
        for (int i = 0; i < NUM_PERIODICITIES; i++) {
            frelements[i].realBalance  = 0;
            frelements[i].imagBalance  = 0;
        }
    }

    float angle = 0;
    // Recalculate the spectral components
    for (int j = firstIndex; j < length; j++) {
        for (int i = 0; i < NUM_PERIODICITIES; i++) {
            angle = 2 * M_PI * (float)times[j] / frelements[i].period;
            frelements[i].realStates   += states[j] * cos(angle);
            frelements[i].imagStates   += states[j] * sin(angle);
            frelements[i].realBalance  += gain * cos(angle);
            frelements[i].imagBalance  += gain * sin(angle);
        }
    }
    measurements += length;

    // Establish amplitudes and phase shifts
    float re, im;
    for (int i = 0; i < NUM_PERIODICITIES; i++) {
        re = frelements[i].realStates - frelements[i].realBalance;
        im = frelements[i].imagStates - frelements[i].imagBalance;
        if (1.5 * frelements[i].period <= duration)
            frelements[i].amplitude = sqrt(re * re + im * im) / measurements; 
        else 
            frelements[i].amplitude = 0;
        if (frelements[i].amplitude < FREMEN_AMPLITUDE_THRESHOLD) 
            frelements[i].amplitude = 0;
        // frelements[i].amplitude = sqrt(re * re + im * im) / measurements;
        frelements[i].phase = atan2(im, re);
    }

    // Sort the spectral components
    qsort(frelements, NUM_PERIODICITIES, sizeof(SFrelement), fremenSort);
    return numUpdated; 
}

int CFrelement::evaluate(uint32_t times[], float signal[], int length, int orderi, float evals[])
{
    float estimate = 0;
    float time;
    float state;
    for (int j = 0; j <= orderi; j++) 
        evals[j] = 0;
    for (int j = 0; j < length; j++) {
        time = times[j];
        state = signal[j];
        estimate = gain;
        evals[0] += fabs(state - estimate);
        for (int i = 0; i < orderi; i++) {
            estimate += 2 * frelements[i].amplitude * cos(time / frelements[i].period * 2 * M_PI - frelements[i].phase);
            evals[i + 1] += fabs(state - estimate);
        }
    }
    for (int j = 0; j <= orderi; j++) 
        evals[j] = evals[j] / length;

    // Get best model order
    float error = 10.0;
    int index = 0;
    for (int j = 0; j <= orderi; j++) {
        if (evals[j] < error - 0.001) {
            index = j;
            error = evals[j]; 
        }
    }
    return index;
}

/* Not required in incremental version */
void CFrelement::update(int modelOrder)
{
}

/* Text representation of the fremen model */
void CFrelement::print(int orderi)
{
    int errs = 0;
    std::cout << " Prior: " << gain << " Size: " << measurements << " ";
    if (orderi > 0) 
        std::cout << std::endl;
    float ampl = gain;
    for (int i = 0; i < orderi; i++) {
        std::cout << "Frelement " << i << " " << frelements[i].amplitude << " " << frelements[i].phase << " " << frelements[i].period << std::endl;
    }
    std::cout << std::endl; 
}

int CFrelement::estimate(uint32_t times[], float probs[], int length, int orderi)
{
    float estimate = 0;
    float time;
    for (int j = 
