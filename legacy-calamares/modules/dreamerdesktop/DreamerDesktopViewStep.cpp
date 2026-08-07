// DreamerDesktopViewStep.cpp

#include "DreamerDesktopViewStep.h"
#include "DreamerDesktopPage.h"
#include "ApplyDesktopJob.h"

#include <GlobalStorage.h>
#include <JobQueue.h>

CALAMARES_PLUGIN_FACTORY_DEFINITION( DreamerDesktopViewStepFactory, registerPlugin<DreamerDesktopViewStep>(); )

DreamerDesktopViewStep::DreamerDesktopViewStep( QObject* parent )
    : Calamares::ViewStep( parent )
    , m_widget( new DreamerDesktopPage() )
{
    connect( m_widget, &DreamerDesktopPage::choiceMade,
             this, [this]() { emit nextStatusChanged( isNextEnabled() ); } );
}

DreamerDesktopViewStep::~DreamerDesktopViewStep()
{
    if ( m_widget && m_widget->parent() == nullptr )
    {
        m_widget->deleteLater();
    }
}

QWidget*
DreamerDesktopViewStep::widget()
{
    return m_widget;
}

void
DreamerDesktopViewStep::setConfigurationMap( const QVariantMap& configurationMap )
{
    m_widget->configure( configurationMap );
}

bool
DreamerDesktopViewStep::isVisible() const
{
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    return gs->value( "dreamerVariant" ).toString() != "scratch";
}

bool
DreamerDesktopViewStep::isNextEnabled() const
{
    switch ( m_widget->currentSubPage() )
    {
    case DreamerDesktopPage::DesktopEnvironment:
        return !m_widget->selectedDesktopId().isEmpty();
    case DreamerDesktopPage::DarkLight:
        return true; // has a default (Dark), always valid
    case DreamerDesktopPage::WallpaperQuestion:
        return true; // Yes/No, both valid, defaults to "No"
    case DreamerDesktopPage::ColorPicker:
        // Color2 can't equal Color1 (that combination doesn't exist as an
        // asset) — page03d's dropdown already excludes it, so reaching here
        // at all means a valid pair is selected.
        return !m_widget->selectedColor1().isEmpty() && !m_widget->selectedColor2().isEmpty();
    }
    return true;
}

bool
DreamerDesktopViewStep::isAtBeginning() const
{
    return m_widget->currentSubPage() == DreamerDesktopPage::DesktopEnvironment;
}

bool
DreamerDesktopViewStep::isAtEnd() const
{
    if ( m_widget->currentSubPage() == DreamerDesktopPage::ColorPicker )
    {
        return true;
    }
    // Also "at end" on the wallpaper question if the answer is No — NEXT
    // there skips straight past the color picker, matching the mockup's
    // "NO -> skips straight to page07" behaviour.
    if ( m_widget->currentSubPage() == DreamerDesktopPage::WallpaperQuestion && !m_widget->wantsCustomWallpaper() )
    {
        return true;
    }
    return false;
}

void
DreamerDesktopViewStep::next()
{
    switch ( m_widget->currentSubPage() )
    {
    case DreamerDesktopPage::DesktopEnvironment:
        m_widget->showDarkLight();
        break;
    case DreamerDesktopPage::DarkLight:
        m_widget->showWallpaperQuestion();
        break;
    case DreamerDesktopPage::WallpaperQuestion:
        if ( m_widget->wantsCustomWallpaper() )
        {
            m_widget->showColorPicker();
        }
        // else: isAtEnd() already returned true, Calamares moves to page07 itself
        break;
    case DreamerDesktopPage::ColorPicker:
        break; // isAtEnd(), Calamares moves on
    }
    emit nextStatusChanged( isNextEnabled() );
}

void
DreamerDesktopViewStep::back()
{
    switch ( m_widget->currentSubPage() )
    {
    case DreamerDesktopPage::DarkLight:
        m_widget->showDesktopEnvironment();
        break;
    case DreamerDesktopPage::WallpaperQuestion:
        m_widget->showDarkLight();
        break;
    case DreamerDesktopPage::ColorPicker:
        m_widget->showWallpaperQuestion();
        break;
    default:
        break;
    }
}

Calamares::JobList
DreamerDesktopViewStep::jobs() const
{
    Calamares::JobList list;
    list.append( Calamares::job_ptr( new ApplyDesktopJob(
        m_widget->selectedDesktopId(),
        m_widget->isDarkMode(),
        m_widget->wantsCustomWallpaper(),
        m_widget->selectedColor1(),
        m_widget->selectedColor2() ) ) );
    return list;
}
