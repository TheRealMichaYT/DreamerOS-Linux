// NetworkPage.h — page01b: Connect to a Network

#pragma once

#include "../InstallerPage.h"

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QCheckBox;
class QLabel;
class NetworkManagerClient;

class NetworkPage : public InstallerPage
{
    Q_OBJECT
public:
    explicit NetworkPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Connect to a Network" ); }
    void onEnter() override;
    bool isNextEnabled() const override;

private Q_SLOTS:
    void onNetworksUpdated( const QStringList& ssids, const QVector<int>& signalStrengths, const QVector<bool>& secured );
    void onNetworkSelected( QListWidgetItem* item );
    void onConnectClicked();
    void onEthernetToggled( bool checked );
    void refreshErrorBanner();

private:
    NetworkManagerClient* m_nm;
    QListWidget* m_networkList;
    QLineEdit* m_passwordField;
    QLabel* m_passwordLabel;
    QCheckBox* m_ethernetCheck;
    QLabel* m_errorBanner;
    QString m_selectedSsid;
};
