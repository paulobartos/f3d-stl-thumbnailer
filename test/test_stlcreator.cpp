#include "stlcreator.h"

#include <KIO/ThumbnailCreator>
#include <QColor>
#include <QCoreApplication>
#include <QImage>
#include <QUrl>
#include <QtDebug>

// Loads a real STL through the ThumbnailCreator and asserts the render is
// non-blank. Exits 0 on success, non-zero on failure.
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        qCritical() << "usage: test_stlcreator <path-to-stl>";
        return 2;
    }
    const QString path = QString::fromLocal8Bit(argv[1]);

    StlCreator creator(nullptr, {});
    KIO::ThumbnailRequest request(QUrl::fromLocalFile(path), QSize(256, 256),
                                  QStringLiteral("model/stl"), 1.0, 0.0f);

    const KIO::ThumbnailResult result = creator.create(request);
    if (!result.isValid()) {
        qCritical() << "FAIL: create() returned an invalid result";
        return 1;
    }
    const QImage img = result.image();
    if (img.isNull()) {
        qCritical() << "FAIL: null image";
        return 1;
    }

    // Count pixels that differ from the corner (background) by more than a threshold.
    const QRgb corner = img.pixel(0, 0);
    long nonBackground = 0;
    const long total = long(img.width()) * img.height();
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const QRgb p = img.pixel(x, y);
            const int diff = qAbs(qRed(p) - qRed(corner)) + qAbs(qGreen(p) - qGreen(corner)) +
                             qAbs(qBlue(p) - qBlue(corner));
            if (diff > 30) {
                ++nonBackground;
            }
        }
    }
    const double fraction = double(nonBackground) / double(total);
    qInfo().noquote() << QStringLiteral("rendered %1x%2, non-background fraction = %3%")
                             .arg(img.width())
                             .arg(img.height())
                             .arg(fraction * 100.0, 0, 'f', 2);

    if (fraction < 0.01) {
        qCritical() << "FAIL: thumbnail is blank";
        return 1;
    }
    qInfo() << "PASS: thumbnail has content";
    return 0;
}
