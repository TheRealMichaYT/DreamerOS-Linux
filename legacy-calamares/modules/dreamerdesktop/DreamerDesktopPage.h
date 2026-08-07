// DreamerDesktopPage.h

#pragma once

#include <QWidget>
#include <QVariantMap>
#include <QStringList>

class QStackedWidget;
class QLabel;
class QComboBox;
class QPushButton;
class QButtonGroup;

class DreamerDesktopPage : public QWidget
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

    explicit DreamerDesktopPage( QWidget* parent = nullptr );

    void configure( const QVariantMap& config );

    void showDesktopEnvironment();
    void showDarkLight();
    void showWallpaperQuestion();
    void showColorPicker();
    SubPage currentSubPage() const { return m_currentSubPage; }

    QString selectedDesktopId() const { return m_desktopId; }
    bool isDarkMode() const { return m_darkMode; }
    bool wantsCustomWallpaper() const { return m_customWallpaper; }
    QString selectedColor1() const { return m_color1; }
    QString selectedColor2() const { return m_color2; }

Q_SIGNALS:
    void choiceMade();

private Q_SLOTS:
    void onDesktopCardClicked( QAbstractButton* button );
    void onDarkLightClicked( QAbstractButton* button );
    void onWallpaperAnswer( QAbstractButton* button );
    void onColor1Changed( int index );
    void updatePreview();

private:
    void rebuildColor2Choices(); // excludes whatever Color1 currently is

    QVariantMap m_config;
    QStackedWidget* m_stack;

    QWidget* m_deWidgetPage;
    QWidget* m_darkLightPage;
    QWidget* m_wallpaperQuestionPage;
    QWidget* m_colorPickerPage;

    QComboBox* m_color1Combo;
    QComboBox* m_color2Combo;
    QLabel* m_previewLabel;

    SubPage m_currentSubPage = DesktopEnvironment;
    QString m_desktopId;
    bool m_darkMode = true;
    bool m_customWallpaper = false;
    QString m_color1;
    QString m_color2;
};
