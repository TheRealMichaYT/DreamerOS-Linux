// DreamerNetworkPage.h
// The actual widget for page01b: Wi-Fi list on the left, password field
// below it once a secured network is picked, "I use Ethernet" card on the
// right, and (when nothing is connected) a red blocking error banner.

#pragma once

#include <QWidget>
#include <QString>

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QCheckBox;
class QLabel;
class DreamerNetworkManagerProxy;

class DreamerNetworkPage : public QWidget
{
    Q_OBJECT
public:
    explicit DreamerNetworkPage( QWidget* parent = nullptr );

    void setTexts( const QString& title,
                   const QString& subtitle,
                   const QString& ethernetLabel,
                   const QString& ethernetSubtext,
                   const QString& errorText,
                   const QString& errorSubtext );

    void startScanning();
    bool isConnected() const;
    bool usedEthernet() const;

Q_SIGNALS:
    void connectivityChanged( bool connected );

private Q_SLOTS:
    void onNetworksUpdated( const QStringList& ssids, const QVector<int>& signalStrengths, const QVector<bool>& secured );
    void onNetworkSelected( QListWidgetItem* item );
    void onConnectClicked();
    void onEthernetToggled( bool checked );
    void refreshErrorBanner();

private:
    DreamerNetworkManagerProxy* m_nm;

    QLabel* m_titleLabel;
    QLabel* m_subtitleLabel;
    QListWidget* m_networkList;
    QLineEdit* m_passwordField;
    QLabel* m_passwordLabel;
    QCheckBox* m_ethernetCheck;
    QLabel* m_ethernetSubtextLabel;
    QLabel* m_errorBanner;

    bool m_connected = false;
    bool m_ethernetUsed = false;
    QString m_selectedSsid;
};
