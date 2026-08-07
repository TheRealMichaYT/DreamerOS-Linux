// InstallerState.h
// Every page reads/writes into one shared state object instead of
// Calamares's GlobalStorage (which doesn't exist here — this is our own
// app now). MainWindow owns one instance and passes a pointer to every page.

#pragma once

#include <QString>
#include <QStringList>
#include <QMap>

struct PartitionChoice
{
    QString diskDevice;      // e.g. "/dev/sda"
    QString mode;            // "erase" | "shrink" | "manual"
    QString espDevice;       // e.g. "/dev/sda1" — EFI System Partition
    QString rootDevice;      // e.g. "/dev/sda3" — mounted at /
    QString bootMountOption; // combined-with-esp | separate
    qint64 shrinkToBytes = 0; // used when mode == "shrink"
};

struct UserAccount
{
    QString fullName;
    QString username;
    QString password;   // held in memory only, hashed at install time
    bool isSudoer = true;
    bool autoLogin = false;
};

class InstallerState
{
public:
    // page01
    QString language = "en_US";

    // page01b
    bool networkConnected = false;
    bool usedEthernet = false;
    QString wifiSsid;
    QString wifiPassword;

    // page02
    QString region;
    QString timezone;
    QString keyboardLayout = "us";
    bool automaticTimeSync = true;

    // page03 / page03b
    QString variantId; // "game" | "develop" | "scratch"
    QStringList basePackages;
    QStringList optionalPackages; // e.g. "mangohud" if checked on Game

    // page04a-d
    QList<UserAccount> users;
    QString rootPassword;
    bool rootLoginDisabled = true; // "login without password" locked when sudo is on

    // page05
    QString hostname = "dreameros";

    // page06-06d — skipped entirely (stays at defaults) when variantId == "scratch"
    QString desktopEnvironment = "kde"; // "kde" | "gnome"
    bool darkMode = true;
    bool customWallpaper = false;
    QString wallpaperColor1;
    QString wallpaperColor2;

    // page07
    PartitionChoice partitioning;
    bool encryptDisk = false; // kept in the model even though the LUKS page itself was cut

    // computed helpers
    QString wallpaperFileName() const
    {
        if ( variantId == "scratch" )
        {
            return "black.png";
        }
        if ( customWallpaper && !wallpaperColor1.isEmpty() && !wallpaperColor2.isEmpty() )
        {
            return QStringLiteral( "%1-%2-wallpaper.png" ).arg( wallpaperColor1, wallpaperColor2 );
        }
        return "diagonal-lines-blue-gold-wallpaper.png";
    }

    bool isScratch() const { return variantId == "scratch"; }
};
