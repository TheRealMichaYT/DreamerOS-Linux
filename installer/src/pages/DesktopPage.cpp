// DesktopPage.cpp

#include "DesktopPage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>

const QStringList DesktopPage::kColors = { "blue", "gold", "green", "purple", "red", "pink" };

DesktopPage::DesktopPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    m_stack = new QStackedWidget( this );
    root->addWidget( m_stack );

    // --- page06: desktop environment ---
    m_dePage = new QWidget( m_stack );
    auto* deLayout = new QHBoxLayout( m_dePage );
    auto* deGroup = new QButtonGroup( this );
    for ( const auto& pair : QList<QPair<QString, QString>> { { "kde", "KDE Plasma" }, { "gnome", "GNOME" } } )
    {
        auto* card = new QPushButton( pair.second, m_dePage );
        card->setObjectName( "dreamerDesktopCard" );
        card->setCheckable( true );
        card->setProperty( "deId", pair.first );
        deGroup->addButton( card );
        deLayout->addWidget( card );
    }
    connect( deGroup, &QButtonGroup::buttonClicked, this, &DesktopPage::onDeCardClicked );
    m_stack->addWidget( m_dePage );

    // --- page06b: dark / light ---
    m_darkLightPage = new QWidget( m_stack );
    auto* dlLayout = new QHBoxLayout( m_darkLightPage );
    auto* dlGroup = new QButtonGroup( this );
    auto* darkBtn = new QPushButton( tr( "Dark" ), m_darkLightPage );
    auto* lightBtn = new QPushButton( tr( "Light" ), m_darkLightPage );
    darkBtn->setObjectName( "dreamerDesktopCard" );
    lightBtn->setObjectName( "dreamerDesktopCard" );
    darkBtn->setCheckable( true );
    lightBtn->setCheckable( true );
    darkBtn->setChecked( true );
    darkBtn->setProperty( "isDark", true );
    lightBtn->setProperty( "isDark", false );
    dlGroup->addButton( darkBtn );
    dlGroup->addButton( lightBtn );
    dlLayout->addWidget( darkBtn );
    dlLayout->addWidget( lightBtn );
    connect( dlGroup, &QButtonGroup::buttonClicked, this, &DesktopPage::onDarkLightClicked );
    m_stack->addWidget( m_darkLightPage );

    // --- page06c: custom wallpaper? ---
    m_wallpaperQPage = new QWidget( m_stack );
    auto* wqLayout = new QVBoxLayout( m_wallpaperQPage );
    auto* wqTitle = new QLabel( tr( "Custom Wallpaper?" ), m_wallpaperQPage );
    wqTitle->setObjectName( "dreamerPageTitle" );
    auto* wqSubtitle = new QLabel( tr( "Would you like to choose your own two-color wallpaper?" ), m_wallpaperQPage );
    wqLayout->addWidget( wqTitle );
    wqLayout->addWidget( wqSubtitle );

    auto* wqCardsRow = new QHBoxLayout();
    auto* wqGroup = new QButtonGroup( this );
    auto* yesBtn = new QPushButton( tr( "YES" ), m_wallpaperQPage );
    auto* noBtn = new QPushButton( tr( "NO" ), m_wallpaperQPage );
    yesBtn->setObjectName( "dreamerDesktopCard" );
    noBtn->setObjectName( "dreamerDesktopCard" );
    yesBtn->setCheckable( true );
    noBtn->setCheckable( true );
    noBtn->setChecked( true );
    yesBtn->setProperty( "wantsCustom", true );
    noBtn->setProperty( "wantsCustom", false );
    wqGroup->addButton( yesBtn );
    wqGroup->addButton( noBtn );
    wqCardsRow->addWidget( yesBtn );
    wqCardsRow->addWidget( noBtn );
    wqLayout->addLayout( wqCardsRow );
    connect( wqGroup, &QButtonGroup::buttonClicked, this, &DesktopPage::onWallpaperAnswer );
    m_stack->addWidget( m_wallpaperQPage );

    // --- page06d: color1 / color2 + preview ---
    m_colorPage = new QWidget( m_stack );
    auto* cpLayout = new QHBoxLayout( m_colorPage );
    auto* leftCol = new QVBoxLayout();
    cpLayout->addLayout( leftCol, 1 );

    leftCol->addWidget( new QLabel( tr( "Color 1 (main)" ), m_colorPage ) );
    m_color1Combo = new QComboBox( m_colorPage );
    m_color1Combo->addItems( kColors );
    leftCol->addWidget( m_color1Combo );

    leftCol->addWidget( new QLabel( tr( "Color 2 (details)" ), m_colorPage ) );
    m_color2Combo = new QComboBox( m_colorPage );
    leftCol->addWidget( m_color2Combo );

    auto* helpLabel = new QLabel( tr( "Color 2 can't match Color 1." ), m_colorPage );
    helpLabel->setObjectName( "dreamerSubtext" );
    leftCol->addWidget( helpLabel );
    leftCol->addStretch();

    m_previewLabel = new QLabel( m_colorPage );
    m_previewLabel->setObjectName( "dreamerWallpaperPreview" );
    m_previewLabel->setMinimumSize( 400, 250 );
    m_previewLabel->setScaledContents( true );
    cpLayout->addWidget( m_previewLabel, 2 );

    connect( m_color1Combo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &DesktopPage::onColor1Changed );
    connect( m_color2Combo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &DesktopPage::onColor2Changed );

    m_stack->addWidget( m_colorPage );

    rebuildColor2Choices();
    m_stack->setCurrentWidget( m_dePage );
}

void
DesktopPage::onDeCardClicked( QAbstractButton* button )
{
    m_state->desktopEnvironment = button->property( "deId" ).toString();
    emit validityChanged();
}

void
DesktopPage::onDarkLightClicked( QAbstractButton* button )
{
    m_state->darkMode = button->property( "isDark" ).toBool();
    emit validityChanged();
}

void
DesktopPage::onWallpaperAnswer( QAbstractButton* button )
{
    m_state->customWallpaper = button->property( "wantsCustom" ).toBool();
    emit validityChanged();
}

void
DesktopPage::onColor1Changed( int index )
{
    Q_UNUSED( index )
    m_state->wallpaperColor1 = m_color1Combo->currentText();
    rebuildColor2Choices();
    updatePreview();
    emit validityChanged();
}

void
DesktopPage::onColor2Changed( int index )
{
    Q_UNUSED( index )
    m_state->wallpaperColor2 = m_color2Combo->currentText();
    updatePreview();
    emit validityChanged();
}

void
DesktopPage::rebuildColor2Choices()
{
    const QString keepOut = m_color1Combo->currentText();
    m_color2Combo->clear();
    for ( const QString& color : kColors )
    {
        if ( color != keepOut )
        {
            m_color2Combo->addItem( color );
        }
    }
    m_state->wallpaperColor1 = keepOut;
    m_state->wallpaperColor2 = m_color2Combo->currentText();
}

void
DesktopPage::updatePreview()
{
    if ( m_state->wallpaperColor1.isEmpty() || m_state->wallpaperColor2.isEmpty() )
    {
        return;
    }
    const QString path = QStringLiteral( "/usr/share/dreameros/wallpapers/%1-%2-wallpaper.png" )
                              .arg( m_state->wallpaperColor1, m_state->wallpaperColor2 );
    QPixmap pix( path );
    if ( !pix.isNull() )
    {
        m_previewLabel->setPixmap( pix );
    }
}

bool
DesktopPage::isNextEnabled() const
{
    switch ( m_subPage )
    {
    case DesktopEnvironment:
        return !m_state->desktopEnvironment.isEmpty();
    case DarkLight:
    case WallpaperQuestion:
        return true;
    case ColorPicker:
        return !m_state->wallpaperColor1.isEmpty() && !m_state->wallpaperColor2.isEmpty();
    }
    return true;
}

bool
DesktopPage::handleNext()
{
    switch ( m_subPage )
    {
    case DesktopEnvironment:
        m_subPage = DarkLight;
        m_stack->setCurrentWidget( m_darkLightPage );
        emit validityChanged();
        return true;
    case DarkLight:
        m_subPage = WallpaperQuestion;
        m_stack->setCurrentWidget( m_wallpaperQPage );
        emit validityChanged();
        return true;
    case WallpaperQuestion:
        if ( m_state->customWallpaper )
        {
            m_subPage = ColorPicker;
            m_stack->setCurrentWidget( m_colorPage );
            emit validityChanged();
            return true;
        }
        return false; // "NO" -> skip straight to page07, let MainWindow advance
    case ColorPicker:
        return false; // done, let MainWindow advance to PartitionPage
    }
    return false;
}

bool
DesktopPage::handleBack()
{
    switch ( m_subPage )
    {
    case DarkLight:
        m_subPage = DesktopEnvironment;
        m_stack->setCurrentWidget( m_dePage );
        emit validityChanged();
        return true;
    case WallpaperQuestion:
        m_subPage = DarkLight;
        m_stack->setCurrentWidget( m_darkLightPage );
        emit validityChanged();
        return true;
    case ColorPicker:
        m_subPage = WallpaperQuestion;
        m_stack->setCurrentWidget( m_wallpaperQPage );
        emit validityChanged();
        return true;
    default:
        return false;
    }
}
