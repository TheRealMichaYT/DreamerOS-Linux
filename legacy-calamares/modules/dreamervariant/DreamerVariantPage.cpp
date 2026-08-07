// DreamerVariantPage.cpp

#include "DreamerVariantPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QCheckBox>
#include <QFrame>

DreamerVariantPage::DreamerVariantPage( QWidget* parent )
    : QWidget( parent )
{
    auto* root = new QVBoxLayout( this );

    m_titleLabel = new QLabel( this );
    m_titleLabel->setObjectName( "dreamerPageTitle" );
    root->addWidget( m_titleLabel );

    m_stack = new QStackedWidget( this );
    root->addWidget( m_stack );

    m_cardsPage = new QWidget( m_stack );
    m_softwarePage = new QWidget( m_stack );
    m_stack->addWidget( m_cardsPage );
    m_stack->addWidget( m_softwarePage );

    m_cardGroup = new QButtonGroup( this );
    m_cardGroup->setExclusive( true );

    m_softwareListLayout = new QVBoxLayout( m_softwarePage );
}

void
DreamerVariantPage::setTitle( const QString& title )
{
    m_titleLabel->setText( title );
}

void
DreamerVariantPage::loadVariants( const QVariantList& variants )
{
    m_variants = variants;

    auto* cardsLayout = new QHBoxLayout( m_cardsPage );

    for ( const QVariant& v : variants )
    {
        QVariantMap m = v.toMap();
        const QString id = m.value( "id" ).toString();
        const QString name = m.value( "name" ).toString();
        const QString tagline = m.value( "tagline" ).toString();

        // Card = a checkable button styled as a rounded panel (gold outline
        // when selected, grey when not — see dreameros.qss for the actual
        // #dreamerCard[checked="true"] style rule matching the mockup).
        auto* card = new QPushButton( m_cardsPage );
        card->setObjectName( "dreamerVariantCard" );
        card->setCheckable( true );
        card->setProperty( "variantId", id );

        auto* cardLayout = new QVBoxLayout( card );
        auto* nameLabel = new QLabel( name, card );
        nameLabel->setObjectName( "dreamerCardTitle" );
        auto* taglineLabel = new QLabel( tagline, card );
        taglineLabel->setObjectName( "dreamerCardSubtitle" );
        cardLayout->addStretch();
        cardLayout->addWidget( nameLabel, 0, Qt::AlignHCenter );
        cardLayout->addWidget( taglineLabel, 0, Qt::AlignHCenter );
        cardLayout->addStretch();

        m_cardGroup->addButton( card );
        cardsLayout->addWidget( card );
    }

    connect( m_cardGroup, &QButtonGroup::buttonClicked, this, &DreamerVariantPage::onCardClicked );
}

void
DreamerVariantPage::onCardClicked( QAbstractButton* button )
{
    m_selectedVariantId = button->property( "variantId" ).toString();
    rebuildSoftwarePage();
    emit selectionChanged();
}

void
DreamerVariantPage::rebuildSoftwarePage()
{
    // Clear any previous variant's checklist before rebuilding
    QLayoutItem* item;
    while ( ( item = m_softwareListLayout->takeAt( 0 ) ) != nullptr )
    {
        delete item->widget();
        delete item;
    }
    m_selectedOptionalPackageIds.clear();

    QVariantMap variantConfig;
    for ( const QVariant& v : m_variants )
    {
        QVariantMap m = v.toMap();
        if ( m.value( "id" ).toString() == m_selectedVariantId )
        {
            variantConfig = m;
            break;
        }
    }

    auto* heading = new QLabel( tr( "%1 includes:" ).arg( variantConfig.value( "name" ).toString() ), m_softwarePage );
    heading->setObjectName( "dreamerSectionHeading" );
    m_softwareListLayout->addWidget( heading );

    for ( const QVariant& pkgV : variantConfig.value( "packages" ).toStringList() )
    {
        auto* row = new QLabel( QStringLiteral( "\u2022 %1" ).arg( pkgV.toString() ), m_softwarePage );
        m_softwareListLayout->addWidget( row );
    }

    const QVariantList optionalPackages = variantConfig.value( "optionalPackages" ).toList();
    if ( !optionalPackages.isEmpty() )
    {
        auto* optHeading = new QLabel( tr( "Optional:" ), m_softwarePage );
        optHeading->setObjectName( "dreamerSectionHeading" );
        m_softwareListLayout->addWidget( optHeading );

        for ( const QVariant& optV : optionalPackages )
        {
            QVariantMap opt = optV.toMap();
            auto* check = new QCheckBox( opt.value( "label" ).toString(), m_softwarePage );
            check->setChecked( opt.value( "default", false ).toBool() );
            check->setProperty( "optionalId", opt.value( "id" ).toString() );
            if ( check->isChecked() )
            {
                m_selectedOptionalPackageIds << opt.value( "id" ).toString();
            }
            connect( check, &QCheckBox::toggled, this, &DreamerVariantPage::onOptionalPackageToggled );
            m_softwareListLayout->addWidget( check );
        }
    }

    m_softwareListLayout->addStretch();
}

void
DreamerVariantPage::onOptionalPackageToggled( bool checked )
{
    auto* check = qobject_cast<QCheckBox*>( sender() );
    if ( !check )
    {
        return;
    }
    const QString id = check->property( "optionalId" ).toString();
    if ( checked && !m_selectedOptionalPackageIds.contains( id ) )
    {
        m_selectedOptionalPackageIds << id;
    }
    else if ( !checked )
    {
        m_selectedOptionalPackageIds.removeAll( id );
    }
}

void
DreamerVariantPage::showVariantCards()
{
    m_currentSubPage = VariantCards;
    m_stack->setCurrentWidget( m_cardsPage );
}

void
DreamerVariantPage::showSoftwareSelection()
{
    m_currentSubPage = SoftwareSelection;
    m_stack->setCurrentWidget( m_softwarePage );
}

QStringList
DreamerVariantPage::selectedOptionalPackageIds() const
{
    return m_selectedOptionalPackageIds;
}
