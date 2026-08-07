// FinishedPage.cpp

#include "FinishedPage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProcess>

FinishedPage::FinishedPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    root->addStretch();

    auto* label = new QLabel( tr( "DreamerOS is ready. Remove the installation media and reboot." ), this );
    label->setObjectName( "dreamerPageTitle" );
    label->setAlignment( Qt::AlignCenter );
    root->addWidget( label );

    auto* rebootButton = new QPushButton( tr( "Reboot Now" ), this );
    rebootButton->setObjectName( "dreamerInstallButton" );
    connect( rebootButton, &QPushButton::clicked, this, &FinishedPage::onRebootClicked );

    auto* buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    buttonRow->addWidget( rebootButton );
    buttonRow->addStretch();
    root->addLayout( buttonRow );

    root->addStretch();
}

void
FinishedPage::onRebootClicked()
{
    QProcess::startDetached( "systemctl", { "reboot" } );
}
