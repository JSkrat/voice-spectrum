#ifndef RENDERSPECTRUM_H
#define RENDERSPECTRUM_H

#include <QWidget>
#include <QPainter>
#include "renderer.h"
#include "spectraldata.h"

class RenderSpectrum : public Renderer
{
public:
    RenderSpectrum(int x, int y, int w, int h, SpectralData &data, QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;

};

#endif // RENDERSPECTRUM_H
