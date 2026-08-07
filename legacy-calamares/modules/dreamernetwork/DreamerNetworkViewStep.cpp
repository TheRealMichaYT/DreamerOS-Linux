// DreamerNetworkViewStep.cpp

#include "DreamerNetworkViewStep.h"
#include "DreamerNetworkPage.h"

#include <GlobalStorage.h>
#include <JobQueue.h>
#include <utils/Logger.h>

CALAMARES_PLUGIN_FACTORY_DEFINITION( DreamerNetworkViewStepFactory, registerPlugin<DreamerNetworkViewStep>(); )

DreamerNetworkViewStep::DreamerNetworkViewStep( QObject* parent )
    : Calamares::ViewStep( parent )
    , m_widget( new DreamerNetworkPage() )
{
    connect( m_widget, &DreamerNetworkPage::connectivityChanged,
             this, &DreamerNetworkViewStep::onConnectivityChanged );
}

DreamerNetworkViewStep::~DreamerNetworkViewStep()
{
    if ( m_widget && m_widget->parent() == nullptr )
    {
        m_widget->deleteLater();
    }
}

QWidget*
DreamerNetworkViewStep::widget()
{
    return m_widget;
}

void
DreamerNetworkViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_requireConnection = configurationMap.value( "requireConnection", true ).toBool();
    m_title = configurationMap.value( "title", tr( "Connect to a Network" ) ).toString();
    m_subtitle = configurationMap.value( "subtitle", QString() ).toString();

    m_widget->setTexts(
        m_title,
        m_subtitle,
        configurationMap.value( "ethernetLabel" ).toString(),
        configurationMap.value( "ethernetSubtext" ).toString(),
        configurationMap.value( "errorText" ).toString(),
        configurationMap.value( "errorSubtext" ).toString() );
}

bool
DreamerNetworkViewStep::isNextEnabled() const
{
    if ( !m_requireConnection )
    {
        return true;
    }
    return m_widget->isConnected();
}

bool
DreamerNetworkViewStep::isBackEnabled() const
{
    return true;
}

void
DreamerNetworkViewStep::onActivate()
{
    m_widget->startScanning();
}

void
DreamerNetworkViewStep::onLeave()
{
    // Persist the choice so later exec-time modules (networkcfg) know
    // whether to write a NetworkManager connection profile into the
    // target system, or leave it Ethernet-only / unconfigured.
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    gs->insert( "dreamerUsedEthernet", m_widget->usedEthernet() );
    gs->insert( "dreamerConnected", m_widget->isConnected() );
}

void
DreamerNetworkViewStep::onConnectivityChanged( bool connected )
{
    Q_UNUSED( connected )
    emit nextStatusChanged( isNextEnabled() );
}
