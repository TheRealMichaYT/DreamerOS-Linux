// MainWindow.cpp

#include "MainWindow.h"
#include "InstallerPage.h"

#include "pages/WelcomePage.h"
#include "pages/NetworkPage.h"
#include "pages/LocalePage.h"
#include "pages/VariantPage.h"
#include "pages/UsersPage.h"
#include "pages/HostnamePage.h"
#include "pages/DesktopPage.h"
#include "pages/PartitionPage.h"
#include "pages/SummaryPage.h"
#include "pages/InstallPage.h"
#include "pages/FinishedPage.h"

#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow( QWidget* parent )
    : QMainWindow( parent )
    , m_stack( new QStackedWidget( this ) )
{
    setWindowTitle( "DreamerOS Installer" );

    auto* central = new QWidget( this );
    setCentralWidget( central );
    auto* rootLayout = new QVBoxLayout( central );

    // top-right logo, present on every page (matches the mockups)
    m_logoLabel = new QLabel( central );
    m_logoLabel->setObjectName( "dreamerLogo" );
    m_logoLabel->setPixmap( QPixmap( ":/dreameros-logo.png" ) );
    auto* logoRow = new QHBoxLayout();
    logoRow->addStretch();
    logoRow->addWidget( m_logoLabel );
    rootLayout->addLayout( logoRow );

    rootLayout->addWidget( m_stack, 1 );

    // bottom nav: About (bottom-left, page01 only) + Back / Next (bottom-right)
    auto* navRow = new QHBoxLayout();
    m_aboutButton = new QPushButton( tr( "\u2139  About DreamerOS" ), central );
    m_aboutButton->setObjectName( "dreamerAboutButton" );
    connect( m_aboutButton, &QPushButton::clicked, this, &MainWindow::showAboutModal );
    navRow->addWidget( m_aboutButton );
    navRow->addStretch();

    m_backButton = new QPushButton( tr( "BACK" ), central );
    m_nextButton = new QPushButton( tr( "NEXT" ), central );
    m_backButton->setObjectName( "dreamerBackButton" );
    m_nextButton->setObjectName( "dreamerNextButton" );
    connect( m_backButton, &QPushButton::clicked, this, &MainWindow::goBack );
    connect( m_nextButton, &QPushButton::clicked, this, &MainWindow::goNext );
    navRow->addWidget( m_backButton );
    navRow->addWidget( m_nextButton );

    rootLayout->addLayout( navRow );

    buildPages();
    updateNavButtons();
}

void
MainWindow::buildPages()
{
    // Fixed order matching the page numbering used throughout the whole
    // project: page01, 01b, 02, 03/03b, 04a-d, 05, 06-06d, 07, 08, 09/10, 11.
    m_pages = {
        new WelcomePage( &m_state, this ),
        new NetworkPage( &m_state, this ),
        new LocalePage( &m_state, this ),
        new VariantPage( &m_state, this ),
        new UsersPage( &m_state, this ),
        new HostnamePage( &m_state, this ),
        new DesktopPage( &m_state, this ),   // self-skips on Scratch, see isVisible_()
        new PartitionPage( &m_state, this ),
        new SummaryPage( &m_state, this ),
        new InstallPage( &m_state, this ),
        new FinishedPage( &m_state, this ),
    };

    for ( InstallerPage* page : m_pages )
    {
        m_stack->addWidget( page );
        connect( page, &InstallerPage::validityChanged, this, &MainWindow::updateNavButtons );
    }

    m_currentPageIndex = 0;
    m_stack->setCurrentWidget( m_pages.first() );
    m_pages.first()->onEnter();
}

void
MainWindow::goNext()
{
    InstallerPage* current = m_pages.at( m_currentPageIndex );

    // Let the page consume the click itself first (e.g. Variant card ->
    // software list, or Desktop's DE -> dark/light -> wallpaper? -> colors
    // sub-screen chain). Only advance the outer stack if it says "no, I'm
    // done, move on".
    if ( current->handleNext() )
    {
        updateNavButtons();
        return;
    }

    int nextIndex = m_currentPageIndex + 1;
    while ( nextIndex < m_pages.size() && !m_pages.at( nextIndex )->isVisible_() )
    {
        ++nextIndex; // skip DesktopPage entirely when InstallerState::isScratch()
    }

    if ( nextIndex >= m_pages.size() )
    {
        return; // already on FinishedPage, NEXT there triggers reboot (handled in that page itself)
    }

    m_currentPageIndex = nextIndex;
    m_stack->setCurrentWidget( m_pages.at( m_currentPageIndex ) );
    m_pages.at( m_currentPageIndex )->onEnter();
    updateNavButtons();
}

void
MainWindow::goBack()
{
    InstallerPage* current = m_pages.at( m_currentPageIndex );

    if ( current->handleBack() )
    {
        updateNavButtons();
        return;
    }

    int prevIndex = m_currentPageIndex - 1;
    while ( prevIndex >= 0 && !m_pages.at( prevIndex )->isVisible_() )
    {
        --prevIndex;
    }

    if ( prevIndex < 0 )
    {
        return;
    }

    m_currentPageIndex = prevIndex;
    m_stack->setCurrentWidget( m_pages.at( m_currentPageIndex ) );
    updateNavButtons();
}

void
MainWindow::updateNavButtons()
{
    InstallerPage* current = m_pages.at( m_currentPageIndex );
    m_nextButton->setEnabled( current->isNextEnabled() );
    m_backButton->setEnabled( current->isBackEnabled() && m_currentPageIndex > 0 );
    m_aboutButton->setVisible( m_currentPageIndex == 0 ); // page01 only, per the mockup
}

void
MainWindow::showAboutModal()
{
    // Content matches what was written into welcome.conf's aboutText during
    // the Calamares-plugin version of this project.
    QMessageBox box( this );
    box.setWindowTitle( tr( "What is DreamerOS?" ) );
    box.setText(
        tr( "DreamerOS is an operating system built on Linux, based on Arch Linux \u2014 "
            "so you get the power Arch gives you, but better. You're in the installer "
            "right now, but did you notice Arch doesn't normally have an installer? "
            "You don't. That's already something good about DreamerOS.\n\n"
            "There are three editions to choose from (pick one at page 06):\n\n"
            "Game \u2014 For gamers. Lightweight, but can't save an older computer.\n"
            "Develop \u2014 A full desktop with the tools builders need, ready to code "
            "from first boot.\n"
            "Scratch \u2014 Very lightweight. A stripped version of KDE. Runs on just "
            "4GB of RAM and 32GB of storage.\n\n"
            "Version 1.0 \u2014 \"Shamer\"" ) );
    box.exec();
}
