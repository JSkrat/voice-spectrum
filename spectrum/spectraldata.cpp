#include "spectraldata.h"

SpectralData::SpectralData(int spectrumLength): spectrumLength(spectrumLength)
{
    this->levels.fill(0, this->waveformLength);
    this->amplitudes.fill(0, this->spectrumLength);
    this->phases.fill(0, this->spectrumLength);
    this->normalized.fill(0, this->spectrumLength);
    this->voice.fill(0, this->spectrumLength);
}

void SpectralData::waveformAddSample(qreal value)
{
    this->levels[this->position++] = value;
    if (this->waveformLength <= this->position) this->position = 0;
}
