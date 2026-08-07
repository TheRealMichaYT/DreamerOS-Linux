// ScratchStripJob.h
// Applies the Scratch-variant lightweight spec from the session notes:
// removes bundled KDE apps, old kernels, unused locales/firmware, man-pages,
// forces pacman-only, and sets the fixed black wallpaper with no picker.
// Only ever runs when GlobalStorage["dreamerVariant"] == "scratch".

#pragma once

#include <Job.h>
#include <QVariantMap>

class ScratchStripJob : public Calamares::Job
{
    Q_OBJECT
public:
    explicit ScratchStripJob( const QVariantMap& stripConfig, QObject* parent = nullptr );

    QString prettyName() const override;
    Calamares::JobResult exec() override;

private:
    void removeBundledKdeApps();
    void keepOnlyNewestKernel();
    void stripUnusedLocales();
    void stripUnusedFirmware();
    void enforcePacmanOnly();
    void removeManPages();
    void setFixedBlackWallpaper();

    QVariantMap m_config;
    QString m_rootMountPoint;
};
