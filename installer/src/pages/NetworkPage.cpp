// NetworkPage.cpp

#include "NetworkPage.h"
#include "../InstallerState.h"
#include "../NetworkManagerClient.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>

static constexpr int SecuredRole = Qt::UserRole + 1;

NetworkPage::NetworkPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
    , m_nm( new NetworkManagerClient( this ) )
{
    auto* root = new QVBoxLayout( this );

    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    auto* subtitle = new QLabel( tr( "Pick a Wi-Fi network, or use Ethernet." ), this );
    root->addWidget( title );
    root->addWidget( subtitle );

    auto* row = new QHBoxLayout();
    root->addLayout( row );

    auto* leftCard = new QGroupBox( this );
    leftCard->setObjectName( "dreamerCard" );
    auto* leftLayout = new QVBoxLayout( leftCard );

    m_networkList = new QListWidget( leftCard );
    m_networkList->setObjectName( "dreamerNetworkList" );
    leftLayout->addWidget( m_networkList );

    m_passwordLabel = new QLabel( tr( "Password" ), leftCard );
    m_passwordLabel->setVisible( false );
    m_passwordField = new QLineEdit( leftCard );
    m_passwordField->setEchoMode( QLineEdit::Password );
    m_passwordField->setVisible( false );
    m_passwordField->setPlaceholderText( tr( "Network password" ) );
    leftLayout->addWidget( m_passwordLabel );
    leftLayout->addWidget( m_passwordField );

    row->addWidget( leftCard, 3 );

    auto* rightCard = new QGroupBox( this );
    rightCard->setObjectName( "dreamerCard" );
    auto* rightLayout = new QVBoxLayout( rightCard );
    m_ethernetCheck = new QCheckBox( tr( "I use Ethernet" ), this );
    auto* ethernetSubtext = new QLabel( tr( "Skip Wi-Fi setup, my cable is plugged in" ), this );
    ethernetSubtext->setObjectName( "dreamerSubtext" );
    rightLayout->addWidget( m_ethernetCheck );
    rightLayout->addWidget( ethernetSubtext );
    rightLayout->addStretch();
    row->addWidget( rightCard, 2 );

    m_errorBanner = new QLabel(
        tr( "No network connection detected\nPick a Wi-Fi network or plug in Ethernet to continue." ), this );
    m_errorBanner->setObjectName( "dreamerErrorBanner" );
    m_errorBanner->setWordWrap( true );
    m_errorBanner->setVisible( false );
    root->addWidget( m_errorBanner );

    connect( m_networkList, &QListWidget::itemClicked, this, &NetworkPage::onNetworkSelected );
    connect( m_passwordField, &QLineEdit::returnPressed, this, &NetworkPage::onConnectClicked );
    connect( m_ethernetCheck, &QCheckBox::toggled, this, &NetworkPage::onEthernetToggled );
    connect( m_nm, &NetworkManagerClient::networksUpdated, this, &NetworkPage::onNetworksUpdated );
    connect( m_nm, &NetworkManagerClient::connectionStateChanged, this, [this]( bool connected ) {
        m_state->networkConnected = connected;
        refreshErrorBanner();
        emit validityChanged();
    } );
}

void
NetworkPage::onEnter()
{
    m_nm->requestScan();
}

bool
NetworkPage::isNextEnabled() const
{
    return m_state->networkConnected || m_state->usedEthernet;
}

void
NetworkPage::onNetworksUpdated( const QStringList& ssids, const QVector<int>& signalStrengths, const QVector<bool>& secured )
{
    m_networkList->clear();
    for ( int i = 0; i < ssids.size(); ++i )
    {
        auto* item = new QListWidgetItem( ssids.at( i ), m_networkList );
        item->setData( Qt::UserRole, signalStrengths.value( i, 0 ) );
        item->setData( SecuredRole, secured.value( i, false ) );
    }
}

void
NetworkPage::onNetworkSelected( QListWidgetItem* item )
{
    m_selectedSsid = item->text();
    const bool secured = item->data( SecuredRole ).toBool();
    m_passwordLabel->setVisible( secured );
    m_passwordField->setVisible( secured );

    if ( !secured )
    {
        m_state->wifiSsid = m_selectedSsid;
        m_nm->connectToOpenNetwork( m_selectedSsid );
    }
    else
    {
        m_passwordField->setFocus();
    }
}

void
NetworkPage::onConnectClicked()
{
    if ( m_selectedSsid.isEmpty() )
    {
        return;
    }
    m_state->wifiSsid = m_selectedSsid;
    m_state->wifiPassword = m_passwordField->text();
    m_nm->connectToSecuredNetwork( m_selectedSsid, m_passwordField->text() );
}

void
NetworkPage::onEthernetToggled( bool checked )
{
    m_state->usedEthernet = checked;
    m_networkList->setEnabled( !checked );
    m_passwordField->setEnabled( !checked );
    refreshErrorBanner();
    emit validityChanged();
}

void
NetworkPage::refreshErrorBanner()
{
    m_errorBanner->setVisible( !isNextEnabled() );
}
