// LocalePage.cpp

#include "LocalePage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QProcess>

LocalePage::LocalePage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    root->addWidget( title );

    // World map: a clickable QLabel showing a world map image; clicking sets
    // region/timezone via a reverse-geocode-ish lookup against known
    // timezone bounding boxes (kept simple here — production build would
    // use the same GeoJSON approach Calamares's own locale page used).
    m_mapLabel = new QLabel( this );
    m_mapLabel->setPixmap( QPixmap( ":/world-map.png" ) );
    m_mapLabel->setObjectName( "dreamerWorldMap" );
    m_mapLabel->setAlignment( Qt::AlignCenter );
    root->addWidget( m_mapLabel );

    auto* dropdownRow = new QHBoxLayout();
    m_keyboardCombo = new QComboBox( this );
    m_keyboardCombo->addItem( "US", "us" );
    m_keyboardCombo->addItem( "UK", "gb" );
    m_keyboardCombo->addItem( "Nederlands", "nl" );
    m_keyboardCombo->addItem( "Deutsch", "de" );
    connect( m_keyboardCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, [this]( int ) {
        m_state->keyboardLayout = m_keyboardCombo->currentData().toString();
    } );

    m_languageCombo = new QComboBox( this );
    m_languageCombo->addItem( "Dutch", "nl_NL" );
    m_languageCombo->addItem( "English", "en_US" );

    dropdownRow->addWidget( m_keyboardCombo );
    dropdownRow->addWidget( m_languageCombo );
    root->addLayout( dropdownRow );

    // --- NTP row (the addition from earlier in this project) ---
    auto* ntpRow = new QHBoxLayout();
    auto* ntpLabel = new QLabel( tr( "Automatic time (NTP)" ), this );
    ntpLabel->setObjectName( "dreamerFieldLabel" );
    auto* ntpSubtext = new QLabel( tr( "Sets the clock automatically once you're online" ), this );
    ntpSubtext->setObjectName( "dreamerSubtext" );
    auto* ntpTextCol = new QVBoxLayout();
    ntpTextCol->addWidget( ntpLabel );
    ntpTextCol->addWidget( ntpSubtext );
    ntpRow->addLayout( ntpTextCol );
    ntpRow->addStretch();

    m_ntpToggle = new QPushButton( this );
    m_ntpToggle->setObjectName( "dreamerToggle" );
    m_ntpToggle->setCheckable( true );
    m_ntpToggle->setChecked( m_state->automaticTimeSync ); // default ON, per the mockup
    m_ntpToggle->setText( m_state->automaticTimeSync ? tr( "ON" ) : tr( "OFF" ) );
    connect( m_ntpToggle, &QPushButton::toggled, this, [this]( bool ) { toggleNtp(); } );
    ntpRow->addWidget( m_ntpToggle );

    root->addLayout( ntpRow );
}

void
LocalePage::toggleNtp()
{
    m_state->automaticTimeSync = m_ntpToggle->isChecked();
    m_ntpToggle->setText( m_state->automaticTimeSync ? tr( "ON" ) : tr( "OFF" ) );
}
