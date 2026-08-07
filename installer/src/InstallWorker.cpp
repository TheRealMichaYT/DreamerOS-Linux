// InstallWorker.cpp

#include "InstallWorker.h"
#include "InstallerState.h"
#include "PartitionUtil.h"

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

InstallWorker::InstallWorker( InstallerState* state, QObject* parent )
    : QThread( parent )
    , m_state( state )
{
}

bool
InstallWorker::runStep( const QString& description, const std::function<bool()>& fn )
{
    emit stepStarted( description );
    const bool ok = fn();
    emit stepFinished( description, ok );
    return ok;
}

void
InstallWorker::run()
{
    // Order matches settings.conf's old exec sequence from the Calamares
    // version — kept identical so nothing about "what happens when" changed
    // just because the framework did.
    if ( !runStep( tr( "Partitioning disk..." ), [this] { return doPartitioning(); } ) )
    {
        emit installFailed( tr( "Partitioning failed." ) );
        return;
    }
    if ( !runStep( tr( "Mounting filesystems..." ), [this] { return doMount(); } ) )
    {
        emit installFailed( tr( "Could not mount the new filesystems." ) );
        return;
    }
    if ( !runStep( tr( "Installing packages for the selected edition..." ), [this] { return doPacstrap(); } ) )
    {
        emit installFailed( tr( "Package installation failed." ) );
        return;
    }
    if ( !runStep( tr( "Writing fstab..." ), [this] { return doFstab(); } ) )
    {
        emit installFailed( tr( "Could not write fstab." ) );
        return;
    }
    if ( !runStep( tr( "Applying system settings..." ), [this] { return doChrootConfig(); } ) )
    {
        emit installFailed( tr( "Could not apply system settings." ) );
        return;
    }
    if ( !m_state->isScratch() )
    {
        if ( !runStep( tr( "Setting up your desktop..." ), [this] { return doDesktopSetup(); } ) )
        {
            emit installFailed( tr( "Could not configure the desktop." ) );
            return;
        }
    }
    else
    {
        if ( !runStep( tr( "Trimming Scratch down to the essentials..." ), [this] { return doScratchStrip(); } ) )
        {
            emit installFailed( tr( "Could not finish the Scratch trim." ) );
            return;
        }
    }
    if ( !runStep( tr( "Configuring network..." ), [this] { return doNetworkConfig(); } ) )
    {
        emit installFailed( tr( "Could not configure the network." ) );
        return;
    }
    if ( !runStep( tr( "Installing bootloader..." ), [this] { return doBootloader(); } ) )
    {
        emit installFailed( tr( "Bootloader installation failed." ) );
        return;
    }
    if ( !runStep( tr( "Finishing up..." ), [this] { return doUnmount(); } ) )
    {
        emit installFailed( tr( "Could not cleanly unmount the new system." ) );
        return;
    }

    emit installFinished();
}

// ---------------------------------------------------------------------
// Small helper: run a command inside the not-yet-booted target system via
// arch-chroot, the same way Calamares's own "contextualprocess" jobs did.
// ---------------------------------------------------------------------
static bool
chrootCommand( const QString& rootMountPoint, const QStringList& command )
{
    QProcess proc;
    QStringList args;
    args << rootMountPoint << command;
    proc.start( "arch-chroot", args );
    proc.waitForFinished( -1 );
    return proc.exitCode() == 0;
}

bool
InstallWorker::doPartitioning()
{
    QString esp, rootDev, error;
    bool ok = false;

    if ( m_state->partitioning.mode == "erase" )
    {
        ok = PartitionUtil::eraseAndPartition( m_state->partitioning.diskDevice, &esp, &rootDev, &error );
    }
    else if ( m_state->partitioning.mode == "shrink" )
    {
        ok = PartitionUtil::shrinkAndPartition(
            m_state->partitioning.rootDevice /* the partition picked to shrink, set on PartitionPage */,
            m_state->partitioning.shrinkToBytes, &esp, &rootDev, &error );
    }
    else
    {
        // Manual mode: PartitionPage should already have written
        // espDevice/rootDevice directly into m_state->partitioning — a full
        // build's manual-mode UI (mountpoint table) isn't included in this
        // reference implementation, so this branch is effectively a stub.
        ok = !m_state->partitioning.espDevice.isEmpty() && !m_state->partitioning.rootDevice.isEmpty();
    }

    if ( ok )
    {
        m_state->partitioning.espDevice = esp.isEmpty() ? m_state->partitioning.espDevice : esp;
        m_state->partitioning.rootDevice = rootDev.isEmpty() ? m_state->partitioning.rootDevice : rootDev;
    }
    return ok;
}

bool
InstallWorker::doMount()
{
    QString error;
    if ( !PartitionUtil::mount( m_state->partitioning.rootDevice, m_rootMountPoint, &error ) )
    {
        return false;
    }
    const QString bootDir = m_rootMountPoint + "/boot";
    return PartitionUtil::mount( m_state->partitioning.espDevice, bootDir, &error );
}

bool
InstallWorker::doPacstrap()
{
    QStringList packages = m_state->basePackages;

    // resolve optional packages (e.g. mangohud) the same way
    // DreamerVariantViewStep::jobs() used to, just inline here now
    if ( m_state->variantId == "game" && m_state->optionalPackages.contains( "mangohud" ) )
    {
        packages << "mangohud";
    }

    if ( packages.isEmpty() )
    {
        return false;
    }

    QProcess proc;
    QStringList args;
    args << "-K" << m_rootMountPoint << packages;
    proc.start( "pacstrap", args );
    proc.waitForFinished( -1 );
    return proc.exitCode() == 0;
}

bool
InstallWorker::doFstab()
{
    QProcess genfstab;
    genfstab.start( "genfstab", { "-U", m_rootMountPoint } );
    genfstab.waitForFinished( -1 );
    if ( genfstab.exitCode() != 0 )
    {
        return false;
    }

    QFile fstabFile( m_rootMountPoint + "/etc/fstab" );
    if ( !fstabFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        return false;
    }
    fstabFile.write( genfstab.readAllStandardOutput() );
    fstabFile.close();
    return true;
}

bool
InstallWorker::doChrootConfig()
{
    bool ok = true;

    // hostname
    QFile hostnameFile( m_rootMountPoint + "/etc/hostname" );
    if ( hostnameFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream( &hostnameFile ) << m_state->hostname << "\n";
    }

    // timezone
    ok &= chrootCommand( m_rootMountPoint,
                          { "ln", "-sf", "/usr/share/zoneinfo/" + m_state->timezone, "/etc/localtime" } );
    ok &= chrootCommand( m_rootMountPoint, { "hwclock", "--systohc" } );

    // locale
    ok &= chrootCommand( m_rootMountPoint, { "bash", "-c", "echo '" + m_state->language + ".UTF-8 UTF-8' >> /etc/locale.gen" } );
    ok &= chrootCommand( m_rootMountPoint, { "locale-gen" } );

    // keyboard layout (console + X11)
    QFile vconsoleFile( m_rootMountPoint + "/etc/vconsole.conf" );
    if ( vconsoleFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream( &vconsoleFile ) << "KEYMAP=" << m_state->keyboardLayout << "\n";
    }

    // NTP
    if ( m_state->automaticTimeSync )
    {
        ok &= chrootCommand( m_rootMountPoint, { "systemctl", "enable", "systemd-timesyncd" } );
    }

    // users + root password + sudo
    for ( const UserAccount& user : m_state->users )
    {
        QStringList useraddArgs = { "useradd", "-m", "-c", user.fullName };
        if ( user.isSudoer )
        {
            useraddArgs << "-G" << "wheel";
        }
        useraddArgs << user.username;
        ok &= chrootCommand( m_rootMountPoint, useraddArgs );
        ok &= chrootCommand( m_rootMountPoint,
                              { "bash", "-c", QString( "echo '%1:%2' | chpasswd" ).arg( user.username, user.password ) } );

        if ( user.autoLogin && !user.isSudoer ) // "login without password" locked when sudo is on
        {
            // sddm autologin config would be written here in a full build
        }
    }
    ok &= chrootCommand( m_rootMountPoint,
                          { "bash", "-c", QString( "echo 'root:%1' | chpasswd" ).arg( m_state->rootPassword ) } );
    ok &= chrootCommand( m_rootMountPoint, { "bash", "-c", "echo '%wheel ALL=(ALL:ALL) ALL' > /etc/sudoers.d/wheel" } );

    return ok;
}

bool
InstallWorker::doDesktopSetup()
{
    bool ok = true;
    ok &= chrootCommand( m_rootMountPoint, { "systemctl", "enable", "sddm" } );

    if ( m_state->desktopEnvironment == "kde" )
    {
        ok &= chrootCommand(
            m_rootMountPoint,
            { "kwriteconfig5", "--file", "kdeglobals", "--group", "KDE", "--key", "LookAndFeelPackage",
              m_state->darkMode ? "org.kde.breezedark.desktop" : "org.kde.breeze.desktop" } );
    }
    else if ( m_state->desktopEnvironment == "gnome" )
    {
        ok &= chrootCommand( m_rootMountPoint,
                              { "gsettings", "set", "org.gnome.desktop.interface", "color-scheme",
                                m_state->darkMode ? "prefer-dark" : "default" } );
    }

    const QString wallpaperFile = m_state->wallpaperFileName();
    const QString destDir = m_rootMountPoint + "/usr/share/dreameros/wallpapers";
    QDir().mkpath( destDir );
    QProcess::execute( "cp", { "/usr/share/dreameros/wallpapers/" + wallpaperFile, destDir + "/" + wallpaperFile } );

    if ( m_state->desktopEnvironment == "kde" )
    {
        ok &= chrootCommand( m_rootMountPoint,
                              { "kwriteconfig5", "--file", "plasma-org.kde.plasma.desktop-appletsrc", "--group",
                                "Wallpaper", "--group", "org.kde.image", "--group", "General", "--key", "Image",
                                "file:///usr/share/dreameros/wallpapers/" + wallpaperFile } );
    }
    return ok;
}

bool
InstallWorker::doScratchStrip()
{
    bool ok = true;

    // no bundled KDE apps
    ok &= chrootCommand( m_rootMountPoint,
                          { "pacman", "-Rns", "--noconfirm", "kate", "dolphin-plugins", "kde-games",
                            "kdeplasma-addons", "elisa", "gwenview", "okular", "kmail", "korganizer" } );

    // only the newest kernel survives
    ok &= chrootCommand( m_rootMountPoint,
                          { "bash", "-c",
                            "pacman -Q | grep '^linux[0-9]*-lts\\|^linux[0-9]*\\b' | sort -V | head -n -1 | "
                            "awk '{print $1}' | xargs -r pacman -Rns --noconfirm" } );

    // only the picked locale's files stay
    QDir localeDir( m_rootMountPoint + "/usr/share/locale" );
    for ( const QString& entry : localeDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ) )
    {
        if ( !m_state->language.startsWith( entry ) )
        {
            QDir( localeDir.filePath( entry ) ).removeRecursively();
        }
    }

    // pacman only
    ok &= chrootCommand( m_rootMountPoint, { "pacman", "-Rns", "--noconfirm", "flatpak", "snapd" } );

    // no man-pages
    QDir( m_rootMountPoint + "/usr/share/man" ).removeRecursively();

    // fixed black wallpaper, no picker
    const QString destDir = m_rootMountPoint + "/usr/share/dreameros/wallpapers";
    QDir().mkpath( destDir );
    QProcess::execute( "cp", { "/usr/share/dreameros/assets/black.png", destDir + "/black.png" } );
    ok &= chrootCommand( m_rootMountPoint,
                          { "systemctl", "enable", "sddm" } ); // Scratch keeps SDDM->KDE, per session spec (KDE stays, just stripped)
    ok &= chrootCommand( m_rootMountPoint,
                          { "kwriteconfig5", "--file", "plasma-org.kde.plasma.desktop-appletsrc", "--group",
                            "Wallpaper", "--group", "org.kde.image", "--group", "General", "--key", "Image",
                            "file:///usr/share/dreameros/wallpapers/black.png" } );

    return ok;
}

bool
InstallWorker::doNetworkConfig()
{
    if ( m_state->usedEthernet || m_state->wifiSsid.isEmpty() )
    {
        return true; // Ethernet just works via DHCP with NetworkManager enabled below
    }

    // Write a NetworkManager connection profile for the Wi-Fi network that
    // was joined during page01b, so the installed system reconnects to it
    // automatically on first real boot.
    const QString profileDir = m_rootMountPoint + "/etc/NetworkManager/system-connections";
    QDir().mkpath( profileDir );
    QFile profile( profileDir + "/" + m_state->wifiSsid + ".nmconnection" );
    if ( profile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        QTextStream out( &profile );
        out << "[connection]\n";
        out << "id=" << m_state->wifiSsid << "\n";
        out << "type=wifi\n\n";
        out << "[wifi]\n";
        out << "ssid=" << m_state->wifiSsid << "\n\n";
        if ( !m_state->wifiPassword.isEmpty() )
        {
            out << "[wifi-security]\n";
            out << "key-mgmt=wpa-psk\n";
            out << "psk=" << m_state->wifiPassword << "\n";
        }
    }
    profile.setPermissions( QFileDevice::ReadOwner | QFileDevice::WriteOwner );

    return chrootCommand( m_rootMountPoint, { "systemctl", "enable", "NetworkManager" } );
}

bool
InstallWorker::doBootloader()
{
    bool ok = true;
    ok &= chrootCommand( m_rootMountPoint,
                          { "grub-install", "--target=x86_64-efi", "--efi-directory=/boot",
                            "--bootloader-id=DreamerOS" } );
    ok &= chrootCommand( m_rootMountPoint, { "grub-mkconfig", "-o", "/boot/grub/grub.cfg" } );
    return ok;
}

bool
InstallWorker::doUnmount()
{
    QString error;
    bool ok = true;
    ok &= PartitionUtil::unmount( m_rootMountPoint + "/boot", &error );
    ok &= PartitionUtil::unmount( m_rootMountPoint, &error );
    return ok;
}
