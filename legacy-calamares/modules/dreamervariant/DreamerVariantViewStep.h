// DreamerVariantViewStep.h
// page03 (Game/Develop/Scratch cards) + page03b (software selection for
// whichever variant got picked) live in the same view step: page03b is
// really just "page03, but showing more detail about the selection", so
// Calamares treats them as two widgets in a QStackedWidget inside one step.

#pragma once

#include <ViewStep.h>
#include <QVariantMap>

class DreamerVariantPage;

class DreamerVariantViewStep : public Calamares::ViewStep
{
    Q_OBJECT
public:
    explicit DreamerVariantViewStep( QObject* parent = nullptr );
    ~DreamerVariantViewStep() override;

    QString prettyName() const override { return tr( "Edition" ); }

    QWidget* widget() override;

    bool isNextEnabled() const override;
    bool isBackEnabled() const override { return true; }
    bool isAtBeginning() const override;
    bool isAtEnd() const override;

    void next() override;
    void back() override;

    void setConfigurationMap( const QVariantMap& configurationMap ) override;

    // At exec time: turns the chosen variant + optional packages into an
    // actual pacman package-install job, and (for Scratch) queues the
    // post-install strip job described by dreamervariant.conf's scratchStrip.
    Calamares::JobList jobs() const override;

private:
    DreamerVariantPage* m_widget;
    QVariantList m_variantsConfig;
};
