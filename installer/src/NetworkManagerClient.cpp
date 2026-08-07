// NetworkManagerClient.cpp
//
// Talks to the live ISO's NetworkManager over the system D-Bus
// (org.freedesktop.NetworkManager), the same instance the desktop's network
// applet would use. It does NOT touch NetworkManager inside the
// not-yet-installed target system — that happens later, at install time,
// via InstallWorker::configureNetwork(), using the ssid/password this class
// connected with (stashed in InstallerState by NetworkPage).

#include "NetworkManagerClient.h"

#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QTimer>

static const char* NM_SERVICE = "org.freedesktop.NetworkManager";
static const char* NM_PATH = "/org/freedesktop/NetworkManager";
static const char* NM_IFACE = "org.freedesktop.NetworkManager";
static const char* NM_DEVICE_IFACE = "org.freedesktop.NetworkManager.Device";
static const char* NM_WIRELESS_IFACE = "org.freedesktop.NetworkManager.Device.Wireless";
static const char* NM_AP_IFACE = "org.freedesktop.NetworkManager.AccessPoint";

NetworkManagerClient::NetworkManagerClient( QObject* parent )
    : QObject( parent )
    , m_nmInterface( new QDBusInterface( NM_SERVICE, NM_PATH, NM_IFACE, QDBusConnection::systemBus(), this ) )
{
    m_wifiDevicePath = findWifiDevicePath();

    QDBusConnection::systemBus().connect(
        NM_SERVICE, NM_PATH, NM_IFACE, "StateChanged",
        this, SLOT( onNMStateChanged( uint ) ) );
}

QString
NetworkManagerClient::findWifiDevicePath() const
{
    QDBusReply<QList<QDBusObjectPath>> devices = m_nmInterface->call( "GetDevices" );
    if ( !devices.isValid() )
    {
        qWarning() << "dreamernetwork: could not enumerate NetworkManager devices:" << devices.error().message();
        return QString();
    }

    for ( const QDBusObjectPath& devicePath : devices.value() )
    {
        QDBusInterface devIface( NM_SERVICE, devicePath.path(), "org.freedesktop.DBus.Properties",
                                  QDBusConnection::systemBus() );
        QDBusReply<QVariant> deviceType = devIface.call( "Get", NM_DEVICE_IFACE, "DeviceType" );
        // NM_DEVICE_TYPE_WIFI == 2
        if ( deviceType.isValid() && deviceType.value().toUInt() == 2 )
        {
            return devicePath.path();
        }
    }
    qWarning() << "dreamernetwork: no Wi-Fi device found on this machine";
    return QString();
}

void
NetworkManagerClient::requestScan()
{
    if ( m_wifiDevicePath.isEmpty() )
    {
        emit networksUpdated( {}, {}, {} ); // no wifi hardware -> empty list, Ethernet card is still usable
        return;
    }

    QDBusInterface wifiIface( NM_SERVICE, m_wifiDevicePath, NM_WIRELESS_IFACE, QDBusConnection::systemBus() );
    wifiIface.call( "RequestScan", QVariantMap() );

    // Scan results land asynchronously; give the driver a moment then read
    // the access-point list. (A production build would connect to the
    // AccessPointAdded/Removed signals instead of polling once — kept
    // simple here since this is the reference implementation.)
    QTimer::singleShot( 3000, this, &NetworkManagerClient::onScanResultsReady );
}

void
NetworkManagerClient::onScanResultsReady()
{
    QDBusInterface wifiIface( NM_SERVICE, m_wifiDevicePath, "org.freedesktop.DBus.Properties",
                               QDBusConnection::systemBus() );
    QDBusReply<QVariant> apsReply = wifiIface.call( "Get", NM_WIRELESS_IFACE, "AccessPoints" );

    QStringList ssids;
    QVector<int> strengths;
    QVector<bool> secured;

    if ( apsReply.isValid() )
    {
        const auto apPaths = qdbus_cast<QList<QDBusObjectPath>>( apsReply.value() );
        for ( const QDBusObjectPath& apPath : apPaths )
        {
            QDBusInterface apIface( NM_SERVICE, apPath.path(), "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus() );
            const QString ssid = QString::fromUtf8(
                apIface.call( "Get", NM_AP_IFACE, "Ssid" ).arguments().value( 0 ).toByteArray() );
            if ( ssid.isEmpty() )
            {
                continue; // hidden network, skip in the list like the mockup shows
            }
            const uint strength = apIface.call( "Get", NM_AP_IFACE, "Strength" ).arguments().value( 0 ).toUInt();
            const uint wpaFlags = apIface.call( "Get", NM_AP_IFACE, "WpaFlags" ).arguments().value( 0 ).toUInt();
            const uint rsnFlags = apIface.call( "Get", NM_AP_IFACE, "RsnFlags" ).arguments().value( 0 ).toUInt();

            if ( !ssids.contains( ssid ) ) // de-dupe repeated APs (mesh / multiple bands)
            {
                ssids << ssid;
                strengths << static_cast<int>( strength );
                secured << ( wpaFlags != 0 || rsnFlags != 0 );
            }
        }
    }

    emit networksUpdated( ssids, strengths, secured );
}

void
NetworkManagerClient::connectToOpenNetwork( const QString& ssid )
{
    connectToSecuredNetwork( ssid, QString() );
}

void
NetworkManagerClient::connectToSecuredNetwork( const QString& ssid, const QString& password )
{
    if ( m_wifiDevicePath.isEmpty() )
    {
        return;
    }

    QVariantMap connection;
    connection[ "id" ] = ssid;
    connection[ "type" ] = "802-11-wireless";

    QVariantMap wifi;
    wifi[ "ssid" ] = ssid.toUtf8();
    wifi[ "mode" ] = "infrastructure";

    QVariantMap security;
    if ( !password.isEmpty() )
    {
        security[ "key-mgmt" ] = "wpa-psk";
        security[ "psk" ] = password;
    }

    QVariantMap settings;
    settings[ "connection" ] = connection;
    settings[ "802-11-wireless" ] = wifi;
    if ( !password.isEmpty() )
    {
        settings[ "802-11-wireless-security" ] = security;
    }

    QDBusInterface addAndActivate( NM_SERVICE, NM_PATH, NM_IFACE, QDBusConnection::systemBus() );
    addAndActivate.call( "AddAndActivateConnection", settings,
                          QVariant::fromValue( QDBusObjectPath( m_wifiDevicePath ) ),
                          QVariant::fromValue( QDBusObjectPath( "/" ) ) );
    // Result of the connection attempt comes back via the StateChanged
    // signal we're already listening to -> onNMStateChanged().
}

void
NetworkManagerClient::onNMStateChanged( uint newState )
{
    // NM_STATE_CONNECTED_GLOBAL == 70 (fully connected, has internet route)
    const bool connected = ( newState == 70 );
    emit connectionStateChanged( connected );
}
