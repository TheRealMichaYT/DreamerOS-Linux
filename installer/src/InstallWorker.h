// InstallWorker.h
// Runs on its own QThread so the UI (the live log list on page10) stays
// responsive while pacstrap etc. run, which can take minutes. Each step
// emits stepStarted()/stepFinished() so InstallPage can append real lines
// to the log — no fake progress bar, matching the original review's praise
// for that approach.

#pragma once

#include <QThread>
#include <QString>
#include <functional>

class InstallerState;

class InstallWorker : public QThread
{
    Q_OBJECT
public:
    explicit InstallWorker( InstallerState* state, QObject* parent = nullptr );

Q_SIGNALS:
    void stepStarted( const QString& description );
    void stepFinished( const QString& description, bool ok );
    void installFailed( const QString& reason );
    void installFinished();

protected:
    void run() override;

private:
    bool doPartitioning();
    bool doMount();
    bool doPacstrap();
    bool doFstab();
    bool doChrootConfig();       // hostname, locale, timezone, users, NTP
    bool doDesktopSetup();       // DE, dark/light, wallpaper (skipped on Scratch)
    bool doScratchStrip();       // only when InstallerState::isScratch()
    bool doNetworkConfig();      // writes the NetworkManager profile chosen on page01b
    bool doBootloader();         // grub-install + grub-mkconfig
    bool doUnmount();

    bool runStep( const QString& description, const std::function<bool()>& fn );

    InstallerState* m_state;
    QString m_rootMountPoint = "/mnt/dreameros-target";
};
