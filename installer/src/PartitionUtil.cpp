// PartitionUtil.cpp

#include "PartitionUtil.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>

namespace PartitionUtil
{

static QByteArray
runAndCapture( const QString& program, const QStringList& args )
{
    QProcess proc;
    proc.start( program, args );
    proc.waitForFinished( -1 );
    return proc.readAllStandardOutput();
}

static bool
runAndCheck( const QString& program, const QStringList& args, QString* errorOut )
{
    QProcess proc;
    proc.start( program, args );
    proc.waitForFinished( -1 );
    if ( proc.exitCode() != 0 )
    {
        if ( errorOut )
        {
            *errorOut = QString::fromUtf8( proc.readAllStandardError() );
        }
        return false;
    }
    return true;
}

QVector<DiskInfo>
listDisks()
{
    // lsblk -J -b -o NAME,TYPE,SIZE,MODEL — JSON output, sizes in bytes
    const QByteArray out = runAndCapture( "lsblk", { "-J", "-b", "-o", "NAME,TYPE,SIZE,MODEL" } );
    const QJsonDocument doc = QJsonDocument::fromJson( out );

    QVector<DiskInfo> disks;
    for ( const QJsonValue& v : doc.object().value( "blockdevices" ).toArray() )
    {
        QJsonObject obj = v.toObject();
        if ( obj.value( "type" ).toString() != "disk" )
        {
            continue;
        }
        DiskInfo info;
        info.device = "/dev/" + obj.value( "name" ).toString();
        info.model = obj.value( "model" ).toString();
        info.sizeBytes = obj.value( "size" ).toVariant().toLongLong();
        disks.append( info );
    }
    return disks;
}

QVector<PartitionInfo>
listPartitions( const QString& diskDevice )
{
    const QByteArray out
        = runAndCapture( "lsblk", { "-J", "-b", "-o", "NAME,FSTYPE,LABEL,SIZE,FSAVAIL", diskDevice } );
    const QJsonDocument doc = QJsonDocument::fromJson( out );

    QVector<PartitionInfo> parts;
    const QJsonArray devices = doc.object().value( "blockdevices" ).toArray();
    if ( devices.isEmpty() )
    {
        return parts;
    }
    for ( const QJsonValue& v : devices.first().toObject().value( "children" ).toArray() )
    {
        QJsonObject obj = v.toObject();
        PartitionInfo info;
        info.device = "/dev/" + obj.value( "name" ).toString();
        info.fsType = obj.value( "fstype" ).toString();
        info.label = obj.value( "label" ).toString();
        info.sizeBytes = obj.value( "size" ).toVariant().toLongLong();
        info.freeBytes = obj.value( "fsavail" ).toVariant().toLongLong();
        parts.append( info );
    }
    return parts;
}

bool
eraseAndPartition( const QString& diskDevice, QString* espOut, QString* rootOut, QString* errorOut )
{
    // GPT layout: 512MiB ESP (fat32, boot flag) + rest as root (ext4).
    // Mirrors what grubcfg/bootloader later expect to find mounted.
    QString err;
    if ( !runAndCheck( "parted", { "-s", diskDevice, "mklabel", "gpt" }, &err )
         || !runAndCheck( "parted", { "-s", diskDevice, "mkpart", "ESP", "fat32", "1MiB", "513MiB" }, &err )
         || !runAndCheck( "parted", { "-s", diskDevice, "set", "1", "esp", "on" }, &err )
         || !runAndCheck( "parted", { "-s", diskDevice, "mkpart", "root", "ext4", "513MiB", "100%" }, &err ) )
    {
        if ( errorOut )
        {
            *errorOut = err;
        }
        return false;
    }

    // NVMe devices use a "p" before the partition number (nvme0n1p1), SATA/
    // virtio devices don't (sda1) — handle both naming schemes.
    const bool needsP = diskDevice.contains( "nvme" ) || diskDevice.contains( "mmcblk" );
    const QString suffix1 = needsP ? "p1" : "1";
    const QString suffix2 = needsP ? "p2" : "2";

    if ( espOut )
    {
        *espOut = diskDevice + suffix1;
    }
    if ( rootOut )
    {
        *rootOut = diskDevice + suffix2;
    }

    if ( !formatPartition( diskDevice + suffix1, "vfat", &err ) || !formatPartition( diskDevice + suffix2, "ext4", &err ) )
    {
        if ( errorOut )
        {
            *errorOut = err;
        }
        return false;
    }

    return true;
}

bool
shrinkAndPartition( const QString& partitionToShrink,
                     qint64 newSizeBytes,
                     QString* espOut,
                     QString* rootOut,
                     QString* errorOut )
{
    QString err;

    // Resizing a live filesystem safely needs the right tool per fstype —
    // ntfsresize for Windows partitions is the common real-world case here
    // (matches the "Windows (NTFS) — 250GB" example from the mockup).
    const QString mb = QString::number( newSizeBytes / ( 1024 * 1024 ) ) + "M";
    if ( !runAndCheck( "ntfsresize", { "--force", "--size", mb, partitionToShrink }, &err ) )
    {
        if ( errorOut )
        {
            *errorOut = err;
        }
        return false;
    }

    // Figure out which disk this partition lives on and where the resized
    // partition now ends, so the new partition can start right after it.
    // (A full implementation reads this from `parted <disk> unit MiB print`;
    // simplified here to keep the reference implementation readable.)
    QString diskDevice = partitionToShrink;
    diskDevice.remove( QRegularExpression( "p?\\d+$" ) );

    if ( !runAndCheck( "parted", { "-s", diskDevice, "resizepart", "1", mb }, &err )
         || !runAndCheck( "parted", { "-s", diskDevice, "mkpart", "root", "ext4", mb, "100%" }, &err ) )
    {
        if ( errorOut )
        {
            *errorOut = err;
        }
        return false;
    }

    const bool needsP = diskDevice.contains( "nvme" ) || diskDevice.contains( "mmcblk" );
    if ( espOut )
    {
        *espOut = partitionToShrink; // reuse existing ESP if the disk already has one (dual-boot case)
    }
    if ( rootOut )
    {
        *rootOut = diskDevice + ( needsP ? "p2" : "2" );
    }

    return formatPartition( *rootOut, "ext4", errorOut );
}

bool
formatPartition( const QString& device, const QString& fsType, QString* errorOut )
{
    if ( fsType == "vfat" )
    {
        return runAndCheck( "mkfs.vfat", { "-F32", device }, errorOut );
    }
    if ( fsType == "ext4" )
    {
        return runAndCheck( "mkfs.ext4", { "-F", device }, errorOut );
    }
    if ( errorOut )
    {
        *errorOut = "Unsupported filesystem type: " + fsType;
    }
    return false;
}

bool
mount( const QString& device, const QString& mountPoint, QString* errorOut )
{
    QDir().mkpath( mountPoint );
    return runAndCheck( "mount", { device, mountPoint }, errorOut );
}

bool
unmount( const QString& mountPoint, QString* errorOut )
{
    return runAndCheck( "umount", { mountPoint }, errorOut );
}

} // namespace PartitionUtil
