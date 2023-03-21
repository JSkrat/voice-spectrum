#include "renderspectrum.h"

RenderSpectrum::RenderSpectrum(int x, int y, int w, int h, SpectralData &data, QWidget *parent):
    Renderer(x, y, w, h, data, parent)
{

}

void RenderSpectrum::paintEvent(QPaintEvent *event)
{
    (void) event;
    QPainter painter(this);
    // blue instant spectrogram
    for (int i = 0; i < std::min(this->data.amplitudes.count(), this->width); i++) {
        painter.fillRect(i, 0, 1, (this->data.normalized.at(i)), Qt::cyan);
        painter.fillRect(i, 0, 1, (this->data.amplitudes.at(i))*1, Qt::blue);
    }
}
