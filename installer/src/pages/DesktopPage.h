// DesktopPage.h — page06 (DE) / 06b (dark-light) / 06c (wallpaper y/n) / 06d (colors)
// Entire page is skipped when InstallerState::isScratch() — see isVisible_().

#pragma once

#include "../InstallerPage.h"

class QStackedWidget;
class QButtonGroup;
class QAbstractButton;
class QComboBox;
class QLabel;

class DesktopPage : public InstallerPage
{
    Q_OBJECT
public:
    enum SubPage
    {
        DesktopEnvironment,
        DarkLight,
        WallpaperQuestion,
        ColorPicker
    };

    explicit DesktopPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Choose your desktop" ); }
    bool isVisible_() const override { return !m_state->isScratch(); }
    bool isNextEnabled() const override;
    bool handleNext() override;
    bool handleBack() override;

private Q_SLOTS:
    void onDeCardClicked( QAbstractButton* button );
    void onDarkLightClicked( QAbstractButton* button );
    void onWallpaperAnswer( QAbstractButton* button );
    void onColor1Changed( int index );
    void onColor2Changed( int index );

private:
    void rebuildColor2Choices();
    void updatePreview();

    SubPage m_subPage = DesktopEnvironment;
    QStackedWidget* m_stack;
    QWidget* m_dePage;
    QWidget* m_darkLightPage;
    QWidget* m_wallpaperQPage;
    QWidget* m_colorPage;

    QComboBox* m_color1Combo;
    QComboBox* m_color2Combo;
    QLabel* m_previewLabel;

    static const QStringList kColors;
};
