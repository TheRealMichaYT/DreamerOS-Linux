// PackageInstallJob.cpp

#include "PackageInstallJob.h"

#include <GlobalStorage.h>
#include <JobQueue.h>
#include <utils/CommandList.h>
#include <utils/Logger.h>

PackageInstallJob::PackageInstallJob( const QStringList& packages, QObject* parent )
    : Calamares::Job( parent )
    , m_packages( packages )
{
}

QString
PackageInstallJob::prettyName() const
{
    return tr( "Installing packages for the selected edition..." );
}

Calamares::JobResult
PackageInstallJob::exec()
{
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();
    const QString rootMountPoint = gs->value( "rootMountPoint" ).toString();

    if ( rootMountPoint.isEmpty() )
    {
        return Calamares::JobResult::error(
            tr( "No target mount point" ),
            tr( "The target filesystem was not mounted before package install ran." ) );
    }

    if ( m_packages.isEmpty() )
    {
        return Calamares::JobResult::error( tr( "No packages selected" ),
                                             tr( "The chosen edition did not resolve to any packages." ) );
    }

    // pacstrap -K <mountpoint> <packages...>
    // Each package name is emitted as progress so the "installing" screen's
    // live log list has something real to show, not a synthetic percentage.
    QStringList args;
    args << "-K" << rootMountPoint << m_packages;

    for ( const QString& pkg : m_packages )
    {
        emitProgress( QString(), 0.0 );  // Calamares core paints the log line itself when the process prints "installing <pkg>"
    }

    int exitCode = CalamaresUtils::System::instance()->targetEnvCommand(
        QStringList() << "pacstrap" << args, QString(), QString(), std::chrono::seconds( 0 ) ).first;

    if ( exitCode != 0 )
    {
        return Calamares::JobResult::error(
            tr( "Package installation failed" ),
            tr( "pacstrap exited with code %1 while installing: %2" ).arg( exitCode ).arg( m_packages.join( ", " ) ) );
    }

    return Calamares::JobResult::ok();
}
