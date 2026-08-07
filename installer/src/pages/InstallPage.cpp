// InstallPage.cpp

#include "InstallPage.h"
#include "../InstallerState.h"
#include "../InstallWorker.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QLabel>

InstallPage::InstallPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    m_stack = new QStackedWidget( this );
    root->addWidget( m_stack );

    // page09: confirm & install button
    m_confirmPage = new QWidget( m_stack );
    auto* confirmLayout = new QVBoxLayout( m_confirmPage );
    auto* confirmLabel = new QLabel( tr( "Ready to install DreamerOS. This will make changes to your disk." ), m_confirmPage );
    confirmLabel->setWordWrap( true );
    auto* installButton = new QPushButton( tr( "Install Now" ), m_confirmPage );
    installButton->setObjectName( "dreamerInstallButton" );
    connect( installButton, &QPushButton::clicked, this, &InstallPage::onInstallClicked );
    confirmLayout->addWidget( confirmLabel );
    confirmLayout->addWidget( installButton );
    m_stack->addWidget( m_confirmPage );

    // page10: live log list — no fake progress bar, real lines as steps run,
    // same as the original package's design that the initial review praised.
    m_progressPage = new QWidget( m_stack );
    auto* progressLayout = new QVBoxLayout( m_progressPage );
    m_logList = new QListWidget( m_progressPage );
    m_logList->setObjectName( "dreamerInstallLog" );
    progressLayout->addWidget( m_logList );
    m_stack->addWidget( m_progressPage );

    m_stack->setCurrentWidget( m_confirmPage );
}

void
InstallPage::onEnter()
{
    m_stack->setCurrentWidget( m_confirmPage );
    m_finished = false;
}

void
InstallPage::onInstallClicked()
{
    m_stack->setCurrentWidget( m_progressPage );
    m_logList->clear();

    m_worker = new InstallWorker( m_state, this );
    connect( m_worker, &InstallWorker::stepStarted, this, &InstallPage::onStepStarted );
    connect( m_worker, &InstallWorker::stepFinished, this, &InstallPage::onStepFinished );
    connect( m_worker, &InstallWorker::installFailed, this, &InstallPage::onInstallFailed );
    connect( m_worker, &InstallWorker::installFinished, this, &InstallPage::onInstallFinished );
    connect( m_worker, &InstallWorker::finished, m_worker, &QObject::deleteLater );

    m_worker->start();
    emit validityChanged();
}

void
InstallPage::onStepStarted( const QString& description )
{
    m_logList->addItem( description );
    m_logList->scrollToBottom();
}

void
InstallPage::onStepFinished( const QString& description, bool ok )
{
    Q_UNUSED( description )
    Q_UNUSED( ok ) // failures are reported separately via installFailed() with the full-stop reason
}

void
InstallPage::onInstallFailed( const QString& reason )
{
    m_logList->addItem( tr( "\u2717 %1" ).arg( reason ) );
    m_logList->scrollToBottom();
}

void
InstallPage::onInstallFinished()
{
    m_finished = true;
    m_logList->addItem( tr( "\u2713 Installation complete." ) );
    m_logList->scrollToBottom();
    emit validityChanged();
}

bool
InstallPage::isNextEnabled() const
{
    return m_finished;
}

bool
InstallPage::isBackEnabled() const
{
    return m_worker == nullptr; // no going back once the install has started
}
