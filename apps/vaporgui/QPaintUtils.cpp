#include "QPaintUtils.h"
#include <QPicture>
#include <QBitmap>
#include <QPixmap>
#include <QPainter>

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

void QPaintUtils::DropShadow(QPainter &p, QPicture &picture, float radius, QColor color)
{
    // Why is this code so weird?
    // Turns out this part of Qt's API is broken (in 4.8 at least)

    float opacity = color.alphaF();
    color.setAlphaF(1.0);

    QRect pictureBB = picture.boundingRect();
    QRect pixmapBB = pictureBB.adjusted(-radius, -radius, radius, radius);

    QPixmap pixmap(pixmapBB.size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.drawPicture(-pixmapBB.topLeft(), picture);

    QPixmap shadowLayer(pixmapBB.size());
    shadowLayer.fill(color);
    shadowLayer.setMask(pixmap.mask());

    QImage shadowImage = shadowLayer.toImage();
    pixmap.fill(Qt::transparent);
    pixmapPainter.setRenderHints((QPainter::Antialiasing | QPainter::SmoothPixmapTransform));    //?
    qt_blurImage(&pixmapPainter, shadowImage, radius, false, false);
    p.setOpacity(opacity);
    p.drawPixmap(pixmapBB.topLeft(), pixmap);
    p.setOpacity(1.0);
}

void QPaintUtils::InnerShadow(QPainter &p, QPicture &picture, float radius, QColor color)
{
    // Why is this code so weird?
    // Turns out this part of Qt's API is broken (in 4.8 at least)

    float opacity = color.alphaF();
    color.setAlphaF(1.0);

    QRect pictureBB = picture.boundingRect();
    QRect pixmapBB = pictureBB.adjusted(-radius, -radius, radius, radius);

    QPixmap pixmap(pixmapBB.size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.drawPicture(-pixmapBB.topLeft(), picture);

    QPixmap shadowLayer(pixmapBB.size());
    shadowLayer.fill(color);

    QBitmap mask = pixmap.mask();
    shadowLayer.setMask(pixmap.createMaskFromColor(Qt::transparent, Qt::MaskOutColor));

    QImage shadowImage = shadowLayer.toImage();
    pixmap.fill(Qt::transparent);
    qt_blurImage(&pixmapPainter, shadowImage, radius, false, false);
    QPainter sp(&shadowLayer);
    sp.setCompositionMode(QPainter::CompositionMode_SourceOut);
    sp.drawPixmap(0, 0, pixmap);
    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::NoPen);
    p.setOpacity(opacity);
    p.drawPixmap(pixmapBB.topLeft(), shadowLayer);
    p.setOpacity(1.0);
}

void QPaintUtils::BoxDropShadow(QPainter &p, QRect box, float radius, QColor color)
{
    QPicture picture;
    QPainter pp(&picture);
    pp.fillRect(box, QBrush(Qt::black));
    pp.end();

    DropShadow(p, picture, radius, color);
}

void QPaintUtils::BoxInnerShadow(QPainter &p, QRect box, float radius, QColor color)
{
    QPicture picture;
    QPainter pp(&picture);
    pp.fillRect(box, QBrush(Qt::black));
    pp.end();

    InnerShadow(p, picture, radius, color);
}
