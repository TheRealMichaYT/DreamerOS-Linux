// InstallPage.h — page09 (Install button) + page10 (Installing... live log)

#pragma once

#include "../InstallerPage.h"

class QListWidget;
class QPushButton;
class QStackedWidget;
class InstallWorker;

class InstallPage : public InstallerPage
{
    Q_OBJECT
public:
    explicit InstallPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Ready to Install" ); }
    void onEnter() override;
    bool isNextEnabled() const override;
    bool isBackEnabled() const override;

private Q_SLOTS:
    void onInstallClicked();
    void onStepStarted( const QString& description );
    void onStepFinished( const QString& description, bool ok );
    void onInstallFailed( const QString& reason );
    void onInstallFinished();

private:
    QStackedWidget* m_stack;
    QWidget* m_confirmPage;
    QWidget* m_progressPage;
    QListWidget* m_logList;
    InstallWorker* m_worker = nullptr;
    bool m_finished = false;
};
