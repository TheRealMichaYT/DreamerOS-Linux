// PartitionPage.h — page07: Partitioning (Erase Disk / Shrink Partition / Manual)

#pragma once

#include "../InstallerPage.h"

class QButtonGroup;
class QAbstractButton;
class QComboBox;
class QLabel;
class QSlider;
class QStackedWidget;

class PartitionPage : public InstallerPage
{
    Q_OBJECT
public:
    explicit PartitionPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Partitioning" ); }
    void onEnter() override;
    bool isNextEnabled() const override;

private Q_SLOTS:
    void onModeClicked( QAbstractButton* button );
    void onDiskChanged( int index );
    void onShrinkSliderMoved( int value );

private:
    void refreshDiskList();
    void refreshShrinkPanel();

    QButtonGroup* m_modeGroup;
    QComboBox* m_diskCombo;
    QStackedWidget* m_detailStack;
    QWidget* m_shrinkPanel;
    QSlider* m_shrinkSlider;
    QLabel* m_shrinkLabel;
    QLabel* m_warningLabel;

    QString m_targetPartitionForShrink;
    qint64 m_partitionTotalBytes = 0;
};
