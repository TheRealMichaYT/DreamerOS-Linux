// NetworkManagerClient.h
// Same D-Bus approach as the earlier Calamares-plugin version, just without
// any Calamares types — a plain QObject any page can use directly.

#pragma once

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QDBusInterface>

class NetworkManagerClient : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManagerClient( QObject* parent = nullptr );

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
