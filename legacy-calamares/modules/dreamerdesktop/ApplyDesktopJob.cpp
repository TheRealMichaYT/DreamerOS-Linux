// ApplyDesktopJob.cpp

#include "ApplyDesktopJob.h"

#include <GlobalStorage.h>
#include <JobQueue.h>
#include <utils/System.h>

#include <QDir>
#include <QProcess>

ApplyDesktopJob::ApplyDesktopJob( const QString& desktopId,
                                   bool darkMode,
                                   bool customWallpaper,
                                   const QString& color1,
                                   const QString& color2,
                                   QObject* parent )
    : Calamares::Job( parent )
    , m_desktopId( desktopId )
    , m_darkMode( darkMode )
    , m_customWallpaper( customWallpaper )
    , m_color1( color1 )
    , m_color2( color2 )
{
}

QString
ApplyDesktopJob::prettyName() const
{
    return tr( "Setting up your desktop..." );
}

Calamares::JobResult
ApplyDesktopJob::exec()
{
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    const QString rootMountPoint = gs->value( "rootMountPoint" ).toString();

    if ( rootMountPoint.isEmpty() )
    {
        return Calamares::JobResult::error( tr( "No target mount point" ), tr( "Cannot configure a desktop that isn't mounted." ) );
    }

    // Enable the right display manager for the packages PackageInstallJob
    // already pulled in (sddm for both KDE and GNOME in our package lists).
    CalamaresUtils::System::instance()->targetEnvCommand( QStringList() << "systemctl" << "enable" << "sddm" );

    // Dark/Light: written into Plasma's kdeglobals (GNOME uses its own gsettings
    // path, both handled the same way — branch on m_desktopId).
    if ( m_desktopId == "kde" )
    {
        CalamaresUtils::System::instance()->targetEnvCommand(
            QStringList() << "kwriteconfig5" << "--file" << "kdeglobals" << "--group" << "KDE"
                           << "--key" << "LookAndFeelPackage"
                           << ( m_darkMode ? "org.kde.breezedark.desktop" : "org.kde.breeze.desktop" ) );
    }
    else if ( m_desktopId == "gnome" )
    {
        CalamaresUtils::System::instance()->targetEnvCommand(
            QStringList() << "gsettings" << "set" << "org.gnome.desktop.interface" << "color-scheme"
                           << ( m_darkMode ? "prefer-dark" : "default" ) );
    }

    // Wallpaper: custom pick, or the fixed default — either way it's one of
    // the 30 pre-rendered colorA-colorB-wallpaper.png assets, never
    // generated at install time.
    const QString wallpaperFile = m_customWallpaper
        ? QStringLiteral( "%1-%2-wallpaper.png" ).arg( m_color1, m_color2 )
        : QStringLiteral( "diagonal-lines-blue-gold-wallpaper.png" );

    const QString sourcePath = QStringLiteral( "/usr/share/dreameros/wallpapers/%1" ).arg( wallpaperFile );
    const QString destDir = rootMountPoint + "/usr/share/dreameros/wallpapers";
    QDir().mkpath( destDir );
    QProcess::execute( "cp", { sourcePath, destDir + "/" + wallpaperFile } );

    if ( m_desktopId == "kde" )
    {
        CalamaresUtils::System::instance()->targetEnvCommand(
            QStringList() << "kwriteconfig5" << "--file" << "plasma-org.kde.plasma.desktop-appletsrc"
                           << "--group" << "Wallpaper" << "--group" << "org.kde.image" << "--group" << "General"
                           << "--key" << "Image"
                           << ( "file:///usr/share/dreameros/wallpapers/" + wallpaperFile ) );
    }
    else if ( m_desktopId == "gnome" )
    {
        CalamaresUtils::System::instance()->targetEnvCommand(
            QStringList() << "gsettings" << "set" << "org.gnome.desktop.background" << "picture-uri"
                           << ( "file:///usr/share/dreameros/wallpapers/" + wallpaperFile ) );
    }

    return Calamares::JobResult::ok();
}
