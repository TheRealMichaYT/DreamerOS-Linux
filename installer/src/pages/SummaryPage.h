// SummaryPage.h — page08: Summary

#pragma once

#include "../InstallerPage.h"

class QLabel;

class SummaryPage : public InstallerPage
{
    Q_OBJECT
public:
    explicit SummaryPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Summary" ); }
    void onEnter() override;

private:
    QLabel* m_summaryLabel;
};
