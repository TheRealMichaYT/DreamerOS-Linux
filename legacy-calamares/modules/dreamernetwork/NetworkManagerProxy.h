// NetworkManagerProxy.h
// Thin wrapper around NetworkManager's D-Bus API (org.freedesktop.NetworkManager)
// so the page widget never has to know about D-Bus directly. This is what
// actually powers the Wi-Fi list and the "connected / not connected" state
// that unblocks NEXT.

#pragma once

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QDBusInterface>

class DreamerNetworkManagerProxy : public QObject
{
    Q_OBJECT
public:
    explicit DreamerNetworkManagerProxy( QObject* parent = nullptr );

    // Kicks off an async Wi-Fi scan; results arrive via networksUpdated().
    void requestScan();

    void connectToOpenNetwork( const QString& ssid );
    void connectToSecuredNetwork( const QString& ssid, const QString& password );

Q_SIGNALS:
    void networksUpdated( const QStringList& ssids, const QVector<int>& signalStrengths, const QVector<bool>& secured );
    void connectionStateChanged( bool connected );

private Q_SLOTS:
    void onScanResultsReady();
    void onNMStateChanged( uint newState );

private:
    QDBusInterface* m_nmInterface;
    QString m_wifiDevicePath;

    QString findWifiDevicePath() const;
};
