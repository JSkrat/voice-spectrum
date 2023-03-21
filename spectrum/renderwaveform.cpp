#include "renderwaveform.h"

RenderWaveform::RenderWaveform(int x, int y, int w, int h, SpectralData &data, QWidget *parent) :
    Renderer(x, y, w, h, data, parent)
{
}

void RenderWaveform::paintEvent(QPaintEvent *event)
{
    (void) event;
    QPainter painter(this);
    for (int i = 0; i < std::min(this->data.levels.count(), this->width); i++) {
        painter.fillRect(i, this->height * (1 - this->data.levels.at(i)), 1, this->height * this->data.levels.at(i), Qt::green);
    }
    // cursor
    painter.fillRect(this->data.position+1, 0, 1, this->height, Qt::black);
}
