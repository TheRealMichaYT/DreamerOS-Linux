// DreamerDesktopPage.cpp

#include "DreamerDesktopPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QPixmap>

DreamerDesktopPage::DreamerDesktopPage( QWidget* parent )
    : QWidget( parent )
{
    auto* root = new QVBoxLayout( this );
    m_stack = new QStackedWidget( this );
    root->addWidget( m_stack );

    m_deWidgetPage = new QWidget( m_stack );
    m_darkLightPage = new QWidget( m_stack );
    m_wallpaperQuestionPage = new QWidget( m_stack );
    m_colorPickerPage = new QWidget( m_stack );

    m_stack->addWidget( m_deWidgetPage );
    m_stack->addWidget( m_darkLightPage );
    m_stack->addWidget( m_wallpaperQuestionPage );
    m_stack->addWidget( m_colorPickerPage );
}

void
DreamerDesktopPage::configure( const QVariantMap& config )
{
    m_config = config;

    // --- page06: desktop environment cards ---
    auto* deLayout = new QHBoxLayout( m_deWidgetPage );
    auto* deGroup = new QButtonGroup( this );
    for ( const QVariant& v : config.value( "desktopEnvironments" ).toList() )
    {
        QVariantMap m = v.toMap();
        auto* card = new QPushButton( m.value( "name" ).toString(), m_deWidgetPage );
        card->setObjectName( "dreamerDesktopCard" );
        card->setCheckable( true );
        card->setProperty( "deId", m.value( "id" ).toString() );
        deGroup->addButton( card );
        deLayout->addWidget( card );
    }
    connect( deGroup, &QButtonGroup::buttonClicked, this, &DreamerDesktopPage::onDesktopCardClicked );

    // --- page06b: dark / light ---
    auto* dlLayout = new QHBoxLayout( m_darkLightPage );
    auto* dlGroup = new QButtonGroup( this );
    auto* darkBtn = new QPushButton( tr( "Dark" ), m_darkLightPage );
    auto* lightBtn = new QPushButton( tr( "Light" ), m_darkLightPage );
    darkBtn->setObjectName( "dreamerDesktopCard" );
    lightBtn->setObjectName( "dreamerDesktopCard" );
    darkBtn->setCheckable( true );
    lightBtn->setCheckable( true );
    darkBtn->setChecked( true ); // default ON per mockup
    darkBtn->setProperty( "isDark", true );
    lightBtn->setProperty( "isDark", false );
    dlGroup->addButton( darkBtn );
    dlGroup->addButton( lightBtn );
    dlLayout->addWidget( darkBtn );
    dlLayout->addWidget( lightBtn );
    connect( dlGroup, &QButtonGroup::buttonClicked, this, &DreamerDesktopPage::onDarkLightClicked );

    // --- page06c: custom wallpaper yes/no ---
    auto* wqLayout = new QVBoxLayout( m_wallpaperQuestionPage );
    auto* wqTitle = new QLabel( config.value( "wallpaperQuestionTitle" ).toString(), m_wallpaperQuestionPage );
    wqTitle->setObjectName( "dreamerPageTitle" );
    auto* wqSubtitle = new QLabel( config.value( "wallpaperQuestionSubtitle" ).toString(), m_wallpaperQuestionPage );
    wqLayout->addWidget( wqTitle );
    wqLayout->addWidget( wqSubtitle );

    auto* wqCardsLayout = new QHBoxLayout();
    wqLayout->addLayout( wqCardsLayout );
    auto* wqGroup = new QButtonGroup( this );
    auto* yesBtn = new QPushButton( tr( "YES" ), m_wallpaperQuestionPage );
    auto* noBtn = new QPushButton( tr( "NO" ), m_wallpaperQuestionPage );
    yesBtn->setObjectName( "dreamerDesktopCard" );
    noBtn->setObjectName( "dreamerDesktopCard" );
    yesBtn->setCheckable( true );
    noBtn->setCheckable( true );
    noBtn->setChecked( true ); // default: use the fixed default wallpaper, no picker shown
    yesBtn->setProperty( "wantsCustom", true );
    noBtn->setProperty( "wantsCustom", false );
    wqGroup->addButton( yesBtn );
    wqGroup->addButton( noBtn );
    wqCardsLayout->addWidget( yesBtn );
    wqCardsLayout->addWidget( noBtn );
    connect( wqGroup, &QButtonGroup::buttonClicked, this, &DreamerDesktopPage::onWallpaperAnswer );

    // --- page06d: color1 / color2 + live preview ---
    auto* cpLayout = new QHBoxLayout( m_colorPickerPage );
    auto* leftCol = new QVBoxLayout();
    cpLayout->addLayout( leftCol, 1 );

    auto* c1Label = new QLabel( tr( "Color 1 (main)" ), m_colorPickerPage );
    m_color1Combo = new QComboBox( m_colorPickerPage );
    m_color1Combo->addItems( config.value( "wallpaperColors" ).toStringList() );
    leftCol->addWidget( c1Label );
    leftCol->addWidget( m_color1Combo );

    auto* c2Label = new QLabel( tr( "Color 2 (details)" ), m_colorPickerPage );
    m_color2Combo = new QComboBox( m_colorPickerPage );
    leftCol->addWidget( c2Label );
    leftCol->addWidget( m_color2Combo );

    auto* helpLabel = new QLabel( tr( "Color 2 can't match Color 1." ), m_colorPickerPage );
    helpLabel->setObjectName( "dreamerSubtext" );
    leftCol->addWidget( helpLabel );
    leftCol->addStretch();

    m_previewLabel = new QLabel( m_colorPickerPage );
    m_previewLabel->setObjectName( "dreamerWallpaperPreview" );
    m_previewLabel->setMinimumSize( 400, 250 );
    m_previewLabel->setScaledContents( true );
    cpLayout->addWidget( m_previewLabel, 2 );

    connect( m_color1Combo, QOverload<int>::of( &QComboBox::currentIndexChanged ),
             this, &DreamerDesktopPage::onColor1Changed );
    connect( m_color2Combo, QOverload<int>::of( &QComboBox::currentIndexChanged ),
             this, [this]( int ) { m_color2 = m_color2Combo->currentText(); updatePreview(); emit choiceMade(); } );

    rebuildColor2Choices();
    updatePreview();
}

void
DreamerDesktopPage::onDesktopCardClicked( QAbstractButton* button )
{
    m_desktopId = button->property( "deId" ).toString();
    emit choiceMade();
}

void
DreamerDesktopPage::onDarkLightClicked( QAbstractButton* button )
{
    m_darkMode = button->property( "isDark" ).toBool();
    emit choiceMade();
}

void
DreamerDesktopPage::onWallpaperAnswer( QAbstractButton* button )
{
    m_customWallpaper = button->property( "wantsCustom" ).toBool();
    emit choiceMade();
}

void
DreamerDesktopPage::onColor1Changed( int index )
{
    Q_UNUSED( index )
    m_color1 = m_color1Combo->currentText();
    rebuildColor2Choices(); // Color1's pick is excluded from Color2's list
    updatePreview();
    emit choiceMade();
}

void
DreamerDesktopPage::rebuildColor2Choices()
{
    const QString keepOut = m_color1Combo->currentText();
    m_color2Combo->clear();
    for ( const QString& color : m_config.value( "wallpaperColors" ).toStringList() )
    {
        if ( color != keepOut )
        {
            m_color2Combo->addItem( color );
        }
    }
    m_color2 = m_color2Combo->currentText();
}

void
DreamerDesktopPage::updatePreview()
{
    if ( m_color1.isEmpty() || m_color2.isEmpty() )
    {
        return;
    }
    const QString pattern = m_config.value( "wallpaperAssetPattern" ).toString();
    const QString path = pattern.arg( m_color1, m_color2 );
    QPixmap pix( path );
    if ( !pix.isNull() )
    {
        m_previewLabel->setPixmap( pix );
    }
}

void
DreamerDesktopPage::showDesktopEnvironment()
{
    m_currentSubPage = DesktopEnvironment;
    m_stack->setCurrentWidget( m_deWidgetPage );
}

void
DreamerDesktopPage::showDarkLight()
{
    m_currentSubPage = DarkLight;
    m_stack->setCurrentWidget( m_darkLightPage );
}

void
DreamerDesktopPage::showWallpaperQuestion()
{
    m_currentSubPage = WallpaperQuestion;
    m_stack->setCurrentWidget( m_wallpaperQuestionPage );
}

void
DreamerDesktopPage::showColorPicker()
{
    m_currentSubPage = ColorPicker;
    m_stack->setCurrentWidget( m_colorPickerPage );
}
