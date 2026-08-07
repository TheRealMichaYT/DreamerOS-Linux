// PartitionUtil.h
// Talks to real system tools (lsblk for reading, parted/mkfs for writing) —
// this is genuinely destructive code, guarded by the confirmation step in
// PartitionPage before anything here is ever called.

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct DiskInfo
{
    QString device;   // "/dev/sda"
    QString model;    // "Samsung SSD 970 EVO"
    qint64 sizeBytes = 0;
};

struct PartitionInfo
{
    QString device;    // "/dev/sda1"
    QString fsType;    // "ntfs", "ext4", "vfat", ...
    QString label;
    qint64 sizeBytes = 0;
    qint64 freeBytes = 0; // relevant for the "shrink" flow
};

namespace PartitionUtil
{
// Read-only: lists disks and their partitions via `lsblk --json`.
QVector<DiskInfo> listDisks();
QVector<PartitionInfo> listPartitions( const QString& diskDevice );

// --- destructive operations, all shell out to parted / mkfs ---

// "Erase Disk" mode: wipes the disk, creates a fresh GPT with an ESP + root.
bool eraseAndPartition( const QString& diskDevice, QString* espOut, QString* rootOut, QString* errorOut );

// "Shrink Partition" mode: shrinks an existing partition to make room, then
// creates the new root partition in the freed space.
bool shrinkAndPartition( const QString& partitionToShrink,
                          qint64 newSizeBytes,
                          QString* espOut,
                          QString* rootOut,
                          QString* errorOut );

bool formatPartition( const QString& device, const QString& fsType, QString* errorOut );
bool mount( const QString& device, const QString& mountPoint, QString* errorOut );
bool unmount( const QString& mountPoint, QString* errorOut );
}
