#include "renderer.h"

/// constructor gets drawing window coordinates and size
///  and a link to data source
Renderer::Renderer(int x, int y, int w, int h, SpectralData &data, QWidget *parent) :
    QWidget(parent),
    data(data)
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->width = w;
    this->height = h;
    this->setGeometry(x, y, this->width, this->height);
}
