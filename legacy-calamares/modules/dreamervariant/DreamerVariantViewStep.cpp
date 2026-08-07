// DreamerVariantViewStep.cpp

#include "DreamerVariantViewStep.h"
#include "DreamerVariantPage.h"
#include "PackageInstallJob.h"
#include "ScratchStripJob.h"

#include <GlobalStorage.h>
#include <JobQueue.h>

CALAMARES_PLUGIN_FACTORY_DEFINITION( DreamerVariantViewStepFactory, registerPlugin<DreamerVariantViewStep>(); )

DreamerVariantViewStep::DreamerVariantViewStep( QObject* parent )
    : Calamares::ViewStep( parent )
    , m_widget( new DreamerVariantPage() )
{
    connect( m_widget, &DreamerVariantPage::selectionChanged,
             this, [this]() { emit nextStatusChanged( isNextEnabled() ); } );
}

DreamerVariantViewStep::~DreamerVariantViewStep()
{
    if ( m_widget && m_widget->parent() == nullptr )
    {
        m_widget->deleteLater();
    }
}

QWidget*
DreamerVariantViewStep::widget()
{
    return m_widget;
}

void
DreamerVariantViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_variantsConfig = configurationMap.value( "variants" ).toList();
    m_widget->setTitle( configurationMap.value( "title" ).toString() );
    m_widget->loadVariants( m_variantsConfig );
}

bool
DreamerVariantViewStep::isNextEnabled() const
{
    // On the card page (page03): need a variant picked.
    // On the software page (page03b): always fine, it's just information +
    // the MangoHud checkbox for Game, nothing there blocks progressing.
    if ( m_widget->currentSubPage() == DreamerVariantPage::VariantCards )
    {
        return !m_widget->selectedVariantId().isEmpty();
    }
    return true;
}

bool
DreamerVariantViewStep::isAtBeginning() const
{
    return m_widget->currentSubPage() == DreamerVariantPage::VariantCards;
}

bool
DreamerVariantViewStep::isAtEnd() const
{
    return m_widget->currentSubPage() == DreamerVariantPage::SoftwareSelection;
}

void
DreamerVariantViewStep::next()
{
    if ( m_widget->currentSubPage() == DreamerVariantPage::VariantCards )
    {
        m_widget->showSoftwareSelection();
        emit nextStatusChanged( isNextEnabled() );
    }
    else
    {
        // leaving the whole step: stash the final choice for jobs() and for
        // dreamerdesktop (needs to know if it's Scratch, to skip itself)
        Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
        gs->insert( "dreamerVariant", m_widget->selectedVariantId() );
        gs->insert( "dreamerOptionalPackages", m_widget->selectedOptionalPackageIds() );
    }
}

void
DreamerVariantViewStep::back()
{
    if ( m_widget->currentSubPage() == DreamerVariantPage::SoftwareSelection )
    {
        m_widget->showVariantCards();
    }
}

Calamares::JobList
DreamerVariantViewStep::jobs() const
{
    Calamares::JobList list;

    const QString variantId = m_widget->selectedVariantId();
    QVariantMap variantConfig;
    for ( const QVariant& v : m_variantsConfig )
    {
        QVariantMap m = v.toMap();
        if ( m.value( "id" ).toString() == variantId )
        {
            variantConfig = m;
            break;
        }
    }

    QStringList packages = variantConfig.value( "packages" ).toStringList();
    for ( const QString& optId : m_widget->selectedOptionalPackageIds() )
    {
        for ( const QVariant& optV : variantConfig.value( "optionalPackages" ).toList() )
        {
            QVariantMap opt = optV.toMap();
            if ( opt.value( "id" ).toString() == optId )
            {
                packages << opt.value( "packages" ).toStringList();
            }
        }
    }

    list.append( Calamares::job_ptr( new PackageInstallJob( packages ) ) );

    if ( variantId == "scratch" )
    {
        list.append( Calamares::job_ptr( new ScratchStripJob( variantConfig.value( "scratchStrip" ).toMap() ) ) );
    }

    return list;
}
