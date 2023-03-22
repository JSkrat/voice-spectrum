#ifndef RENDERDEBUG_H
#define RENDERDEBUG_H

#include <QWidget>
#include <QPainter>
#include "renderer.h"
#include "spectraldata.h"

class RenderDebug : public Renderer
{
public:
    RenderDebug(int x, int y, int w, int h, SpectralData &data, QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;

};

#endif // RENDERDEBUG_H
