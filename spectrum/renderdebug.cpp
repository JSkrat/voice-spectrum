#include "renderdebug.h"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

RenderDebug::RenderDebug(int x, int y, int w, int h, SpectralData &data, QWidget *parent):
    Renderer(x, y, w, h, data, parent)
{

}

void RenderDebug::paintEvent(QPaintEvent *event)
{
    (void) event;
    QPainter painter(this);
    for (int i = 1; i < std::min(this->data.amplitudes.count(), this->width)-1; i++) {
        painter.fillRect(i, 0, 1, (this->data.smooth.at(i)), Qt::gray);
        bool d1SignChanged = sgn(this->data.derivative_1[i+1]) != sgn(this->data.derivative_1[i]);
        if ((0 == this->data.derivative_1[i] || d1SignChanged) && 0 > this->data.derivative_2[i]) {
            painter.fillRect(i, 0, 1, -this->data.derivative_2[i]*10, Qt::red);
        }
    }
//    painter.fillRect(this->data.baseFrequency, 0, 1, this->height, Qt::red);
}
