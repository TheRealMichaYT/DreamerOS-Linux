// WelcomePage.h — page01: Select Installer Language

#pragma once

#include "../InstallerPage.h"

class QComboBox;

class WelcomePage : public InstallerPage
{
    Q_OBJECT
public:
    explicit WelcomePage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Select Installer Language" ); }

private:
    QComboBox* m_languageCombo;
};
