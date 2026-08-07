// ApplyDesktopJob.h
// Exec-time job: installs the chosen DE's display manager config, sets
// Plasma/GNOME to dark or light, and (if a custom wallpaper was picked)
// copies the matching colorA-colorB-wallpaper.png into the target and
// points the desktop config at it. If no custom wallpaper was chosen, the
// default diagonal-lines wallpaper is used instead.

#pragma once

#include <Job.h>
#include <QString>

class ApplyDesktopJob : public Calamares::Job
{
    Q_OBJECT
public:
    ApplyDesktopJob( const QString& desktopId,
                      bool darkMode,
                      bool customWallpaper,
                      const QString& color1,
                      const QString& color2,
                      QObject* parent = nullptr );

    QString prettyName() const override;
    Calamares::JobResult exec() override;

private:
    QString m_desktopId;
    bool m_darkMode;
    bool m_customWallpaper;
    QString m_color1;
    QString m_color2;
};
