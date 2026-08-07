// FinishedPage.h — page11: Finished / reboot

#pragma once

#include "../InstallerPage.h"

class FinishedPage : public InstallerPage
{
    Q_OBJECT
public:
    explicit FinishedPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Installation Complete" ); }
    bool isBackEnabled() const override { return false; }

private Q_SLOTS:
    void onRebootClicked();
};
