// DreamerDesktopViewStep.h
// page06 + 06b + 06c + 06d as four stacked sub-pages of one step:
//   1. Desktop environment (KDE / GNOME)
//   2. Dark / Light mode
//   3. Custom wallpaper? yes/no  -- skipped straight through on "no"
//   4. Color1 / Color2 picker with live preview -- only reached on "yes"
//
// The entire step is skipped when the variant chosen on page03 is Scratch:
// Scratch has no wallpaper picker, no theme choice beyond dark/light — see
// isVisible() below, which is what actually removes the step from the
// installer's page flow (not just hides widgets on it).

#pragma once

#include <ViewStep.h>
#include <QVariantMap>

class DreamerDesktopPage;

class DreamerDesktopViewStep : public Calamares::ViewStep
{
    Q_OBJECT
public:
    explicit DreamerDesktopViewStep( QObject* parent = nullptr );
    ~DreamerDesktopViewStep() override;

    QString prettyName() const override { return tr( "Desktop" ); }

    QWidget* widget() override;

    bool isNextEnabled() const override;
    bool isBackEnabled() const override { return true; }
    bool isAtBeginning() const override;
    bool isAtEnd() const override;

    void next() override;
    void back() override;

    void setConfigurationMap( const QVariantMap& configurationMap ) override;

    Calamares::JobList jobs() const override;

    // Called by Calamares before showing this step: if the variant picked
    // on page03 was Scratch, this step is removed from the flow entirely
    // and partitioning (page07) comes right after users/hostname instead.
    bool isVisible() const;

private:
    DreamerDesktopPage* m_widget;
};
