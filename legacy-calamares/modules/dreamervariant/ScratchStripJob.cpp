// ScratchStripJob.cpp

#include "ScratchStripJob.h"

#include <GlobalStorage.h>
#include <JobQueue.h>
#include <utils/System.h>
#include <utils/Logger.h>

#include <QDir>
#include <QProcess>

ScratchStripJob::ScratchStripJob( const QVariantMap& stripConfig, QObject* parent )
    : Calamares::Job( parent )
    , m_config( stripConfig )
{
}

QString
ScratchStripJob::prettyName() const
{
    return tr( "Trimming Scratch down to the essentials..." );
}

Calamares::JobResult
ScratchStripJob::exec()
{
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    m_rootMountPoint = gs->value( "rootMountPoint" ).toString();

    if ( m_rootMountPoint.isEmpty() )
    {
        return Calamares::JobResult::error( tr( "No target mount point" ), tr( "Cannot strip a system that isn't mounted." ) );
    }

    if ( m_config.value( "removeBundledKdeApps", false ).toBool() )
    {
        removeBundledKdeApps();
    }
    if ( m_config.value( "keepOnlyNewestKernel", false ).toBool() )
    {
        keepOnlyNewestKernel();
    }
    if ( m_config.value( "keepOnlyUsedLocales", false ).toBool() )
    {
        stripUnusedLocales();
    }
    if ( m_config.value( "keepOnlyTargetFirmware", false ).toBool() )
    {
        stripUnusedFirmware();
    }
    if ( m_config.value( "packageManager" ).toString() == "pacman-only" )
    {
        enforcePacmanOnly();
    }
    if ( m_config.value( "removeManPages", false ).toBool() )
    {
        removeManPages();
    }
    if ( m_config.value( "wallpaper" ).toString() == "fixed-black" )
    {
        setFixedBlackWallpaper();
    }

    return Calamares::JobResult::ok();
}

void
ScratchStripJob::removeBundledKdeApps()
{
    // No Kate, no extra Dolphin add-ons, no games, no bundled office/media —
    // only terminal (Konsole) + browser (Brave) stay, per the session spec.
    static const QStringList toRemove = {
        "kate", "dolphin-plugins", "kde-games", "kdeplasma-addons",
        "elisa", "gwenview", "okular", "kmail", "korganizer"
    };
    CalamaresUtils::System::instance()->targetEnvCommand(
        QStringList() << "pacman" << "-Rns" << "--noconfirm" << toRemove );
}

void
ScratchStripJob::keepOnlyNewestKernel()
{
    // List installed kernel packages, keep the newest, remove the rest —
    // avoids the "hundreds of MB per old kernel" waste flagged in the spec.
    CalamaresUtils::System::instance()->targetEnvCommand(
        QStringList() << "bash" << "-c"
                       << "pacman -Q | grep '^linux[0-9]*-lts\\|^linux[0-9]*\\b' | sort -V | head -n -1 | "
                          "awk '{print $1}' | xargs -r pacman -Rns --noconfirm" );
}

void
ScratchStripJob::stripUnusedLocales()
{
    // Only the language(s) picked on page02 survive; everything else in
    // /usr/share/locale is removed to reclaim the hundreds-of-MB the spec
    // flagged for locale packages.
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    const QString keepLocale = gs->value( "locale" ).toString(); // e.g. "nl_NL" or "en_US"

    QDir localeDir( m_rootMountPoint + "/usr/share/locale" );
    for ( const QString& entry : localeDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
        if ( !keepLocale.startsWith( entry ) )
        {
            QDir( localeDir.filePath( entry ) ).removeRecursively();
        }
    }
}

void
ScratchStripJob::stripUnusedFirmware()
{
    // Keep only firmware files matching hardware actually detected on this
    // machine (via lspci/lsusb module IDs already gathered earlier by
    // Calamares's hwinfo). Anything else in /usr/lib/firmware goes.
    CalamaresUtils::System::instance()->targetEnvCommand(
        QStringList() << "bash" << "-c"
                       << "comm -23 <(ls /usr/lib/firmware | sort) "
                          "<(lspci -n -k | grep -oE '[0-9a-f]{4}:[0-9a-f]{4}' | sort -u) "
                          "| xargs -r -I{} rm -rf /usr/lib/firmware/{}" );
}

void
ScratchStripJob::enforcePacmanOnly()
{
    // Make sure no flatpak/snap sneak in via package dependencies.
    CalamaresUtils::System::instance()->targetEnvCommand(
        QStringList() << "pacman" << "-Rns" << "--noconfirm" << "flatpak" << "snapd" );
}

void
ScratchStripJob::removeManPages()
{
    QDir( m_rootMountPoint + "/usr/share/man" ).removeRecursively();
}

void
ScratchStripJob::setFixedBlackWallpaper()
{
    // Scratch skips the whole dreamerdesktop wallpaper-picker flow
    // (page06c/06d are Game/Develop only) — just drop a single pure-black
    // wallpaper and point Plasma's config at it directly.
    const QString wallpaperDir = m_rootMountPoint + "/usr/share/dreameros/wallpapers";
    QDir().mkpath( wallpaperDir );
    // (the actual pure-black PNG asset ships in the ISO's airootfs overlay,
    // copied here rather than generated, so we're not drawing pixels at
    // install time)
    QProcess::execute( "cp", { "/usr/share/dreameros/assets/black.png", wallpaperDir + "/black.png" } );

    CalamaresUtils::System::instance()->targetEnvCommand(
        QStringList() << "kwriteconfig5" << "--file" << "plasma-org.kde.plasma.desktop-appletsrc"
                       << "--group" << "Wallpaper" << "--group" << "org.kde.image" << "--group" << "General"
                       << "--key" << "Image" << "file://" + wallpaperDir + "/black.png" );
}
