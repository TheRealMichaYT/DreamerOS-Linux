// DreamerNetworkViewStep.h
// The "Connect to a Network" page (page01b).
//
// This is a Calamares *view step*: it owns one page in the installer, and
// tells Calamares whether NEXT is allowed to be pressed via isNextEnabled().
// The actual Wi-Fi scanning is delegated to NetworkManager over D-Bus
// (DreamerNetworkManagerProxy, in NetworkManagerProxy.h) so this class stays
// focused on the page logic, not the network stack itself.

#pragma once

#include <ViewStep.h>
#include <QVariantMap>

class DreamerNetworkPage;

class DreamerNetworkViewStep : public Calamares::ViewStep
{
    Q_OBJECT
public:
    explicit DreamerNetworkViewStep( QObject* parent = nullptr );
    ~DreamerNetworkViewStep() override;

    QString prettyName() const override { return tr( "Network" ); }

    QWidget* widget() override;

    bool isNextEnabled() const override;
    bool isBackEnabled() const override;
    bool isAtBeginning() const override { return true; }
    bool isAtEnd() const override { return true; }

    void onActivate() override;
    void onLeave() override;

    void setConfigurationMap( const QVariantMap& configurationMap ) override;

    Calamares::JobList jobs() const override { return {}; } // no install-time job, state is just written to globalStorage

private Q_SLOTS:
    void onConnectivityChanged( bool connected );

private:
    DreamerNetworkPage* m_widget;
    bool m_requireConnection = true;
    QString m_title;
    QString m_subtitle;
};
