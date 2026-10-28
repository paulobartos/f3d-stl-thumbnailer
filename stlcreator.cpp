#include "stlcreator.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QProcess>
#include <QUuid>
#include <QtDebug>

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(StlCreator, "stlthumbnail.json")

Q_LOGGING_CATEGORY(LOG_STL, "com.f3d-stl-thumbnailer")

StlCreator::StlCreator(QObject *parent, const QVariantList &args)
    : KIO::ThumbnailCreator(parent, args)
{
}

KIO::ThumbnailResult StlCreator::create(const KIO::ThumbnailRequest &request)
{
    const QString path = request.url().toLocalFile();
    const int width  = qMax(1, int(request.targetSize().width() * request.devicePixelRatio()));
    const int height = qMax(1, int(request.targetSize().height() * request.devicePixelRatio()));

    // Render via F3D (headless VTK offscreen, no display needed) to a temp PNG.
    const QString outPath = QDir::tempPath() + QStringLiteral("/f3d_stlthumb_%1_%2.png")
        .arg(QCoreApplication::applicationPid())
        .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    QStringList args;
    args << "--config=thumbnail" << "--verbose=quiet"
         << "--output=" + outPath
         << "--resolution=" + QString::number(width) + "," + QString::number(height)
         << path;
    proc.start(QStringLiteral("f3d"), args);
    if (!proc.waitForFinished(30000)) {
        proc.kill();
        QFile::remove(outPath);
        return KIO::ThumbnailResult::fail();
    }
    if (proc.exitCode() != 0) {
        qCWarning(LOG_STL) << "F3D render failed:" << proc.readAll();
        QFile::remove(outPath);
        return KIO::ThumbnailResult::fail();
    }

    QImage img(outPath);
    QFile::remove(outPath);
    if (img.isNull()) {
        return KIO::ThumbnailResult::fail();
    }
    return KIO::ThumbnailResult::pass(img);
}

#include "stlcreator.moc"
