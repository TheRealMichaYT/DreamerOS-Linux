// DreamerNetworkPage.cpp

#include "DreamerNetworkPage.h"
#include "NetworkManagerProxy.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QGroupBox>

// Custom role used to stash "is this network secured" on each list item,
// so we know whether to reveal the password field when it's clicked.
static constexpr int SecuredRole = Qt::UserRole + 1;

DreamerNetworkPage::DreamerNetworkPage( QWidget* parent )
    : QWidget( parent )
    , m_nm( new DreamerNetworkManagerProxy( this ) )
{
    auto* root = new QVBoxLayout( this );

    m_titleLabel = new QLabel( this );
    m_titleLabel->setObjectName( "dreamerPageTitle" ); // styled gold, bold — see dreameros.qss
    m_subtitleLabel = new QLabel( this );
    m_subtitleLabel->setObjectName( "dreamerPageSubtitle" );
    root->addWidget( m_titleLabel );
    root->addWidget( m_subtitleLabel );

    auto* row = new QHBoxLayout();
    root->addLayout( row );

    // --- left: wifi list + password field, inside a card panel ---
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

    // --- right: Ethernet card ---
    auto* rightCard = new QGroupBox( this );
    rightCard->setObjectName( "dreamerCard" );
    auto* rightLayout = new QVBoxLayout( rightCard );

    m_ethernetCheck = new QCheckBox( this );
    m_ethernetSubtextLabel = new QLabel( this );
    m_ethernetSubtextLabel->setObjectName( "dreamerSubtext" );
    rightLayout->addWidget( m_ethernetCheck );
    rightLayout->addWidget( m_ethernetSubtextLabel );
    rightLayout->addStretch();

    row->addWidget( rightCard, 2 );

    // --- error banner (only visible when nothing is connected) ---
    m_errorBanner = new QLabel( this );
    m_errorBanner->setObjectName( "dreamerErrorBanner" );
    m_errorBanner->setWordWrap( true );
    m_errorBanner->setVisible( false );
    root->addWidget( m_errorBanner );

    connect( m_networkList, &QListWidget::itemClicked, this, &DreamerNetworkPage::onNetworkSelected );
    connect( m_passwordField, &QLineEdit::returnPressed, this, &DreamerNetworkPage::onConnectClicked );
    connect( m_ethernetCheck, &QCheckBox::toggled, this, &DreamerNetworkPage::onEthernetToggled );
    connect( m_nm, &DreamerNetworkManagerProxy::networksUpdated, this, &DreamerNetworkPage::onNetworksUpdated );
    connect( m_nm, &DreamerNetworkManagerProxy::connectionStateChanged, this, [this]( bool connected ) {
        m_connected = connected;
        refreshErrorBanner();
        emit connectivityChanged( isConnected() );
    } );
}

void
DreamerNetworkPage::setTexts( const QString& title,
                               const QString& subtitle,
                               const QString& ethernetLabel,
                               const QString& ethernetSubtext,
                               const QString& errorText,
                               const QString& errorSubtext )
{
    m_titleLabel->setText( title );
    m_subtitleLabel->setText( subtitle );
    m_ethernetCheck->setText( ethernetLabel );
    m_ethernetSubtextLabel->setText( ethernetSubtext );
    m_errorBanner->setText( QStringLiteral( "%1\n%2" ).arg( errorText, errorSubtext ) );
}

void
DreamerNetworkPage::startScanning()
{
    m_nm->requestScan();
}

bool
DreamerNetworkPage::isConnected() const
{
    return m_connected || m_ethernetUsed;
}

bool
DreamerNetworkPage::usedEthernet() const
{
    return m_ethernetUsed;
}

void
DreamerNetworkPage::onNetworksUpdated( const QStringList& ssids, const QVector<int>& signalStrengths, const QVector<bool>& secured )
{
    m_networkList->clear();
    for ( int i = 0; i < ssids.size(); ++i )
    {
        auto* item = new QListWidgetItem( ssids.at( i ), m_networkList );
        item->setData( Qt::UserRole, signalStrengths.value( i, 0 ) ); // bars icon painted via delegate
        item->setData( SecuredRole, secured.value( i, false ) );
    }
}

void
DreamerNetworkPage::onNetworkSelected( QListWidgetItem* item )
{
    m_selectedSsid = item->text();
    const bool secured = item->data( SecuredRole ).toBool();

    m_passwordLabel->setVisible( secured );
    m_passwordField->setVisible( secured );

    if ( !secured )
    {
        m_nm->connectToOpenNetwork( m_selectedSsid );
    }
    else
    {
        m_passwordField->setFocus();
    }
}

void
DreamerNetworkPage::onConnectClicked()
{
    if ( m_selectedSsid.isEmpty() )
    {
        return;
    }
    m_nm->connectToSecuredNetwork( m_selectedSsid, m_passwordField->text() );
}

void
DreamerNetworkPage::onEthernetToggled( bool checked )
{
    m_ethernetUsed = checked;
    if ( checked )
    {
        m_networkList->setEnabled( false );
        m_passwordField->setEnabled( false );
    }
    else
    {
        m_networkList->setEnabled( true );
        m_passwordField->setEnabled( true );
    }
    refreshErrorBanner();
    emit connectivityChanged( isConnected() );
}

void
DreamerNetworkPage::refreshErrorBanner()
{
    // Shown only once we know the state (avoids flashing red before the
    // first scan result comes back), and hidden as soon as either path
    // to a connection succeeds.
    m_errorBanner->setVisible( !isConnected() );
}
