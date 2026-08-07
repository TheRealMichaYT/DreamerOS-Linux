// SummaryPage.cpp

#include "SummaryPage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QLabel>

SummaryPage::SummaryPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    root->addWidget( title );

    m_summaryLabel = new QLabel( this );
    m_summaryLabel->setWordWrap( true );
    root->addWidget( m_summaryLabel );
    root->addStretch();
}

void
SummaryPage::onEnter()
{
    QStringList lines;
    lines << tr( "Language: %1" ).arg( m_state->language );
    lines << tr( "Keyboard: %1" ).arg( m_state->keyboardLayout );
    lines << tr( "Network: %1" )
                 .arg( m_state->usedEthernet ? tr( "Ethernet" )
                                              : ( m_state->networkConnected ? m_state->wifiSsid : tr( "Not connected" ) ) );
    lines << tr( "Automatic time sync: %1" ).arg( m_state->automaticTimeSync ? tr( "On" ) : tr( "Off" ) );
    lines << tr( "Edition: %1" ).arg( m_state->variantId );
    if ( !m_state->optionalPackages.isEmpty() )
    {
        lines << tr( "Optional packages: %1" ).arg( m_state->optionalPackages.join( ", " ) );
    }
    for ( const UserAccount& user : m_state->users )
    {
        lines << tr( "User: %1 (%2)%3" )
                     .arg( user.username, user.fullName, user.isSudoer ? tr( " \u2014 administrator" ) : QString() );
    }
    lines << tr( "Hostname: %1" ).arg( m_state->hostname );

    if ( !m_state->isScratch() )
    {
        lines << tr( "Desktop: %1, %2 mode" )
                     .arg( m_state->desktopEnvironment == "kde" ? "KDE Plasma" : "GNOME",
                           m_state->darkMode ? tr( "Dark" ) : tr( "Light" ) );
        lines << tr( "Wallpaper: %1" ).arg( m_state->wallpaperFileName() );
    }
    else
    {
        lines << tr( "Desktop: Scratch \u2014 stripped KDE, fixed black wallpaper" );
    }

    lines << tr( "Disk: %1 (%2)" ).arg( m_state->partitioning.diskDevice, m_state->partitioning.mode );

    m_summaryLabel->setText( lines.join( "\n" ) );
}
