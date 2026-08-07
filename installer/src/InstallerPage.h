// InstallerPage.h
// Base class for every page widget (Welcome, Network, Locale, Variant,
// Users, Hostname, Desktop, Partition, Summary, Install, Finished).
//
// Pages that internally have multiple "sub-screens" (Variant has the card
// choice + software list; Desktop has DE choice + dark/light + wallpaper
// question + color picker) handle that themselves via next()/back()
// returning false to mean "consumed internally, don't advance the outer
// page stack yet" — mirroring how DreamerVariantViewStep/DreamerDesktopViewStep
// worked when this was still Calamares plugins.

#pragma once

#include <QWidget>

class InstallerState;

class InstallerPage : public QWidget
{
    Q_OBJECT
public:
    explicit InstallerPage( InstallerState* state, QWidget* parent = nullptr )
        : QWidget( parent )
        , m_state( state )
    {
    }

    virtual QString pageTitle() const = 0;

    // Called every time this page becomes the visible one (equivalent to
    // Calamares's onActivate()).
    virtual void onEnter() {}

    // Whether the page's own rules currently allow NEXT / BACK.
    virtual bool isNextEnabled() const { return true; }
    virtual bool isBackEnabled() const { return true; }

    // Whether this page should even appear in the flow at all — used by
    // DesktopPage to remove itself entirely when InstallerState::isScratch().
    virtual bool isVisible_() const { return true; }

    // Returns true if the page fully handled the NEXT/BACK press itself
    // (e.g. moved from its card sub-screen to its detail sub-screen) and
    // MainWindow should NOT advance to the next page in the outer stack.
    virtual bool handleNext() { return false; }
    virtual bool handleBack() { return false; }

Q_SIGNALS:
    void validityChanged();

protected:
    InstallerState* m_state;
};
