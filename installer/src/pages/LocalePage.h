// LocalePage.h — page02: Region, Timezone, Keyboard + NTP toggle

#pragma once

#include "../InstallerPage.h"

class QComboBox;
class QLabel;
class QPushButton;

class LocalePage : public InstallerPage
{
    Q_OBJECT
public:
    explicit LocalePage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Region, Timezone & Keyboard" ); }

private:
    void paintWorldMap();     // draws the clickable world map used to set timezone
    void toggleNtp();

    QComboBox* m_keyboardCombo;
    QComboBox* m_languageCombo;
    QPushButton* m_ntpToggle;
    QLabel* m_mapLabel;
};
