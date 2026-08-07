// WelcomePage.cpp

#include "WelcomePage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>

WelcomePage::WelcomePage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* layout = new QVBoxLayout( this );
    layout->addStretch();

    m_languageCombo = new QComboBox( this );
    m_languageCombo->setObjectName( "dreamerLanguageCombo" );
    // Real build ships the full locale list generated from `locale -a` at
    // build time; kept short here for clarity of the reference implementation.
    m_languageCombo->addItem( "Nederlands", "nl_NL" );
    m_languageCombo->addItem( "English", "en_US" );
    m_languageCombo->addItem( "Deutsch", "de_DE" );
    m_languageCombo->addItem( "Fran\u00e7ais", "fr_FR" );
    m_languageCombo->addItem( "Espa\u00f1ol", "es_ES" );

    connect( m_languageCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, [this]( int ) {
        m_state->language = m_languageCombo->currentData().toString();
    } );
    m_state->language = m_languageCombo->currentData().toString();

    auto* comboRow = new QHBoxLayout();
    comboRow->addStretch();
    comboRow->addWidget( m_languageCombo );
    comboRow->addStretch();
    layout->addLayout( comboRow );

    layout->addStretch();
}
