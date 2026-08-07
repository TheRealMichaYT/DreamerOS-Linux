// PartitionPage.cpp

#include "PartitionPage.h"
#include "../InstallerState.h"
#include "../PartitionUtil.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QStackedWidget>

PartitionPage::PartitionPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    root->addWidget( title );

    auto* modeRow = new QHBoxLayout();
    m_modeGroup = new QButtonGroup( this );
    for ( const auto& pair :
          QList<QPair<QString, QString>> { { "erase", "Erase Disk\nAutomatic, wipes selected disk" },
                                            { "shrink", "Shrink Partition\nResize an existing partition" },
                                            { "manual", "Manual\nAdd mountpoints yourself" } } )
    {
        auto* card = new QPushButton( pair.second, this );
        card->setObjectName( "dreamerPartitionModeCard" );
        card->setCheckable( true );
        card->setProperty( "mode", pair.first );
        m_modeGroup->addButton( card );
        modeRow->addWidget( card );
    }
    connect( m_modeGroup, &QButtonGroup::buttonClicked, this, &PartitionPage::onModeClicked );
    root->addLayout( modeRow );

    m_diskCombo = new QComboBox( this );
    connect( m_diskCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &PartitionPage::onDiskChanged );
    root->addWidget( m_diskCombo );

    m_detailStack = new QStackedWidget( this );
    root->addWidget( m_detailStack );

    m_shrinkPanel = new QWidget( m_detailStack );
    auto* shrinkLayout = new QVBoxLayout( m_shrinkPanel );
    m_shrinkLabel = new QLabel( this );
    m_shrinkSlider = new QSlider( Qt::Horizontal, m_shrinkPanel );
    connect( m_shrinkSlider, &QSlider::valueChanged, this, &PartitionPage::onShrinkSliderMoved );
    shrinkLayout->addWidget( m_shrinkLabel );
    shrinkLayout->addWidget( m_shrinkSlider );
    m_detailStack->addWidget( m_shrinkPanel );

    m_warningLabel = new QLabel( this );
    m_warningLabel->setObjectName( "dreamerWarningText" );
    root->addWidget( m_warningLabel );
}

void
PartitionPage::onEnter()
{
    refreshDiskList();
}

void
PartitionPage::refreshDiskList()
{
    m_diskCombo->clear();
    for ( const DiskInfo& disk : PartitionUtil::listDisks() )
    {
        const double gib = disk.sizeBytes / 1073741824.0;
        m_diskCombo->addItem( QStringLiteral( "%1 \u2014 %2 (%3 GB)" ).arg( disk.device, disk.model ).arg( gib, 0, 'f', 0 ),
                               disk.device );
    }
}

void
PartitionPage::onModeClicked( QAbstractButton* button )
{
    m_state->partitioning.mode = button->property( "mode" ).toString();

    if ( m_state->partitioning.mode == "shrink" )
    {
        refreshShrinkPanel();
        m_detailStack->setCurrentWidget( m_shrinkPanel );
    }
    emit validityChanged();
}

void
PartitionPage::onDiskChanged( int index )
{
    Q_UNUSED( index )
    m_state->partitioning.diskDevice = m_diskCombo->currentData().toString();
    if ( m_state->partitioning.mode == "shrink" )
    {
        refreshShrinkPanel();
    }
    emit validityChanged();
}

void
PartitionPage::refreshShrinkPanel()
{
    const auto partitions = PartitionUtil::listPartitions( m_diskCombo->currentData().toString() );
    if ( partitions.isEmpty() )
    {
        return;
    }
    // Simplification: shrink the first non-ESP partition found (in the
    // mockup this was the "Windows (NTFS) — 250GB" bar) — a full build
    // lets the user click which partition in the bar to shrink.
    const PartitionInfo& target = partitions.first();
    m_targetPartitionForShrink = target.device;
    m_partitionTotalBytes = target.sizeBytes;

    m_shrinkSlider->setRange( 0, 100 );
    m_shrinkSlider->setValue( 50 );
    onShrinkSliderMoved( 50 );
}

void
PartitionPage::onShrinkSliderMoved( int value )
{
    const qint64 newSizeForExisting = m_partitionTotalBytes * value / 100;
    const qint64 freedForDreamerOS = m_partitionTotalBytes - newSizeForExisting;
    m_state->partitioning.shrinkToBytes = newSizeForExisting;

    m_shrinkLabel->setText( tr( "%1 GB for existing system \u2014 %2 GB freed for DreamerOS" )
                                 .arg( newSizeForExisting / 1073741824.0, 0, 'f', 1 )
                                 .arg( freedForDreamerOS / 1073741824.0, 0, 'f', 1 ) );

    // "Next disabled until /boot and / are assigned" — here that just means
    // a sane minimum size was chosen (mirrors the mockup's red warning text).
    const bool tooSmall = freedForDreamerOS < 8LL * 1024 * 1024 * 1024; // 8GB floor
    m_warningLabel->setText( tooSmall ? tr( "Not enough space freed for DreamerOS \u2014 need at least 8GB" )
                                       : QString() );
    emit validityChanged();
}

bool
PartitionPage::isNextEnabled() const
{
    if ( m_state->partitioning.mode.isEmpty() || m_state->partitioning.diskDevice.isEmpty() )
    {
        return false;
    }
    if ( m_state->partitioning.mode == "shrink" )
    {
        const qint64 freed = m_partitionTotalBytes - m_state->partitioning.shrinkToBytes;
        return freed >= 8LL * 1024 * 1024 * 1024;
    }
    return true; // erase / manual: disk chosen is enough at this stage
}
