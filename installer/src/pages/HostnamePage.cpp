// HostnamePage.cpp

#include "HostnamePage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

HostnamePage::HostnamePage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    root->addWidget( title );

    m_hostnameField = new QLineEdit( this );
    m_hostnameField->setText( state->hostname );
    // Valid Linux hostnames: letters, digits, hyphens, not starting/ending with a hyphen.
    auto* validator
        = new QRegularExpressionValidator( QRegularExpression( "^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$" ), this );
    m_hostnameField->setValidator( validator );

    connect( m_hostnameField, &QLineEdit::textChanged, this, [this]( const QString& text ) {
        m_state->hostname = text;
        emit validityChanged();
    } );

    root->addWidget( m_hostnameField );
    root->addStretch();
}

bool
HostnamePage::isNextEnabled() const
{
    return !m_state->hostname.trimmed().isEmpty();
}
