// VariantPage.cpp
//
// The variant definitions (packages, taglines, optional packages) that used
// to live in dreamervariant.conf are now a plain C++ table below — no YAML
// config loader in this version, everything is compiled in.

#include "VariantPage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>

namespace
{
QVariantList
variantTable()
{
    QVariantMap game;
    game[ "id" ] = "game";
    game[ "name" ] = "Game";
    game[ "tagline" ] = "For gamers";
    game[ "description" ] = "Lightweight, but can't save an old machine.";
    game[ "packages" ]
        = QStringList { "base", "linux", "linux-firmware", "plasma-desktop", "sddm", "steam", "lutris" };
    QVariantMap mangohud;
    mangohud[ "id" ] = "mangohud";
    mangohud[ "label" ] = "MangoHud (in-game performance overlay)";
    mangohud[ "default" ] = false;
    mangohud[ "packages" ] = QStringList { "mangohud" };
    game[ "optionalPackages" ] = QVariantList { mangohud };

    QVariantMap develop;
    develop[ "id" ] = "develop";
    develop[ "name" ] = "Develop";
    develop[ "tagline" ] = "For developers";
    develop[ "description" ] = "A full desktop with the tools builders need, ready to code from first boot.";
    develop[ "packages" ] = QStringList { "base",  "linux",       "linux-firmware", "plasma-desktop",
                                           "sddm", "konsole",     "kate",           "git",
                                           "base-devel", "docker" };

    QVariantMap scratch;
    scratch[ "id" ] = "scratch";
    scratch[ "name" ] = "Scratch";
    scratch[ "tagline" ] = "Lightweight, terminal only";
    scratch[ "description" ] = "Very lightweight \u2014 a stripped version of KDE. Runs on 4GB RAM, 32GB storage.";
    scratch[ "packages" ] = QStringList { "base", "linux", "linux-firmware", "plasma-desktop", "sddm", "brave-bin" };

    return { game, develop, scratch };
}
}

VariantPage::VariantPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    root->addWidget( title );

    m_stack = new QStackedWidget( this );
    root->addWidget( m_stack );

    m_cardsPage = new QWidget( m_stack );
    m_softwarePage = new QWidget( m_stack );
    m_stack->addWidget( m_cardsPage );
    m_stack->addWidget( m_softwarePage );

    auto* cardsLayout = new QHBoxLayout( m_cardsPage );
    m_cardGroup = new QButtonGroup( this );

    for ( const QVariant& v : variantTable() )
    {
        QVariantMap m = v.toMap();
        auto* card = new QPushButton( m_cardsPage );
        card->setObjectName( "dreamerVariantCard" );
        card->setCheckable( true );
        card->setProperty( "variantId", m.value( "id" ) );

        auto* cardLayout = new QVBoxLayout( card );
        auto* nameLabel = new QLabel( m.value( "name" ).toString(), card );
        nameLabel->setObjectName( "dreamerCardTitle" );
        auto* taglineLabel = new QLabel( m.value( "tagline" ).toString(), card );
        taglineLabel->setObjectName( "dreamerCardSubtitle" );
        cardLayout->addStretch();
        cardLayout->addWidget( nameLabel, 0, Qt::AlignHCenter );
        cardLayout->addWidget( taglineLabel, 0, Qt::AlignHCenter );
        cardLayout->addStretch();

        m_cardGroup->addButton( card );
        cardsLayout->addWidget( card );
    }
    connect( m_cardGroup, &QButtonGroup::buttonClicked, this, &VariantPage::onCardClicked );

    m_softwareLayout = new QVBoxLayout( m_softwarePage );
}

QVariantMap
VariantPage::variantConfig( const QString& id ) const
{
    for ( const QVariant& v : variantTable() )
    {
        QVariantMap m = v.toMap();
        if ( m.value( "id" ).toString() == id )
        {
            return m;
        }
    }
    return {};
}

void
VariantPage::onCardClicked( QAbstractButton* button )
{
    m_state->variantId = button->property( "variantId" ).toString();
    const QVariantMap cfg = variantConfig( m_state->variantId );
    m_state->basePackages = cfg.value( "packages" ).toStringList();
    m_state->optionalPackages.clear();
    rebuildSoftwarePage();
    emit validityChanged();
}

void
VariantPage::rebuildSoftwarePage()
{
    QLayoutItem* item;
    while ( ( item = m_softwareLayout->takeAt( 0 ) ) != nullptr )
    {
        delete item->widget();
        delete item;
    }

    const QVariantMap cfg = variantConfig( m_state->variantId );

    auto* heading = new QLabel( tr( "%1 includes:" ).arg( cfg.value( "name" ).toString() ), m_softwarePage );
    heading->setObjectName( "dreamerSectionHeading" );
    m_softwareLayout->addWidget( heading );

    for ( const QString& pkg : cfg.value( "packages" ).toStringList() )
    {
        m_softwareLayout->addWidget( new QLabel( QStringLiteral( "\u2022 %1" ).arg( pkg ), m_softwarePage ) );
    }

    const QVariantList optional = cfg.value( "optionalPackages" ).toList();
    if ( !optional.isEmpty() )
    {
        auto* optHeading = new QLabel( tr( "Optional:" ), m_softwarePage );
        optHeading->setObjectName( "dreamerSectionHeading" );
        m_softwareLayout->addWidget( optHeading );

        for ( const QVariant& optV : optional )
        {
            QVariantMap opt = optV.toMap();
            auto* check = new QCheckBox( opt.value( "label" ).toString(), m_softwarePage );
            check->setChecked( opt.value( "default", false ).toBool() );
            check->setProperty( "optId", opt.value( "id" ) );
            if ( check->isChecked() )
            {
                m_state->optionalPackages << opt.value( "id" ).toString();
            }
            connect( check, &QCheckBox::toggled, this, &VariantPage::onOptionalToggled );
            m_softwareLayout->addWidget( check );
        }
    }
    m_softwareLayout->addStretch();
}

void
VariantPage::onOptionalToggled( bool checked )
{
    auto* check = qobject_cast<QCheckBox*>( sender() );
    if ( !check )
    {
        return;
    }
    const QString id = check->property( "optId" ).toString();
    if ( checked && !m_state->optionalPackages.contains( id ) )
    {
        m_state->optionalPackages << id;
    }
    else if ( !checked )
    {
        m_state->optionalPackages.removeAll( id );
    }
}

bool
VariantPage::isNextEnabled() const
{
    if ( m_subPage == Cards )
    {
        return !m_state->variantId.isEmpty();
    }
    return true;
}

bool
VariantPage::handleNext()
{
    if ( m_subPage == Cards )
    {
        m_subPage = Software;
        m_stack->setCurrentWidget( m_softwarePage );
        emit validityChanged();
        return true; // consumed, stay on this outer page
    }
    return false; // on Software sub-page, let MainWindow advance to UsersPage
}

bool
VariantPage::handleBack()
{
    if ( m_subPage == Software )
    {
        m_subPage = Cards;
        m_stack->setCurrentWidget( m_cardsPage );
        emit validityChanged();
        return true;
    }
    return false;
}
