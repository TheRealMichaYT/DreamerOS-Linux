// HostnamePage.h — page05: Hostname

#pragma once

#include "../InstallerPage.h"

class QLineEdit;

class HostnamePage : public InstallerPage
{
    Q_OBJECT
public:
    explicit HostnamePage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Name Your Computer" ); }
    bool isNextEnabled() const override;

private:
    QLineEdit* m_hostnameField;
};
