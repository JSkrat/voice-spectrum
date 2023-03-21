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
    qreal maxAmplitude = 0;
    const qreal peakAmplitude = 200;
    if (this->bufLength*2 <= this->levelsForFFT.length()) {
        kiss_fft_cpx *bufout = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx) * this->bufLength);
        kiss_fft(this->cfg, reinterpret_cast<const kiss_fft_cpx*>(this->levelsForFFT.constData()), bufout);
        for (int i = 1; i < this->bufLength; i++) {
            this->data.amplitudes[i] = sqrt(pow(bufout[i].r, 2) + pow(bufout[i].i, 2));
            if (maxAmplitude < data.amplitudes[i]) maxAmplitude = data.amplitudes[i];
            // apparently the phase is atan(r/i) or something like that, but not sure if we need that at all
//            this->data.phases[i] = bufout[i].i;
        }
        // populate compressed spectrum and find fundamental frequency of the voice
        const int minFrequency = 10;
        const qreal threshold = 100;
        this->data.baseFrequency = 0;
        for (int i = 1; i < this->bufLength; i++) {
            this->data.normalized[i] = peakAmplitude/maxAmplitude*this->data.amplitudes[i];
            if (minFrequency < i && threshold <= data.normalized[i] && 0 == this->data.baseFrequency) {
                this->data.baseFrequency = i;
            }
        }
        free(bufout);
        this->levelsForFFT.clear();
    }
    // update the widget
    this->update();
}
