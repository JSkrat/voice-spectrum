#include "spectrumcalculator.h"

SpectrumCalculator::SpectrumCalculator(SpectralData &data, int bufferLength, QObject *parent) :
    QObject(parent),
    bufLength(bufferLength),
    data(data)
{
    this->levelsForFFT.clear();
    // now init fft
    this->cfg = kiss_fft_alloc(this->bufLength, 0, 0, 0);
}

/// slot to accept audio data
/// recalculate spectrogramm every this->bufLength'th tick
void SpectrumCalculator::setLevel(qreal value)
{
    // waveform buffer
    this->data.waveformAddSample(value);
    // accumulate waveform sample for fft
    this->levelsForFFT.append(value); // amplitude, real part
    this->levelsForFFT.append(0); // imaginary part
    if (this->bufLength*2 <= this->levelsForFFT.length()) {
        kiss_fft_cpx *bufout = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx) * this->bufLength);
        kiss_fft(this->cfg, reinterpret_cast<const kiss_fft_cpx*>(this->levelsForFFT.constData()), bufout);
        for (int i = 0; i < this->bufLength; i++) {
            this->data.amplitudes[i] = sqrt(pow(bufout[i].r, 2) + pow(bufout[i].i, 2));
            // apparently the phase is atan(r/i) or something like that, but not sure if we need that at all
//            this->data.phases[i] = bufout[i].i;
        }
        free(bufout);
        this->levelsForFFT.clear();
    }
    // update the widget
    this->update();
}
