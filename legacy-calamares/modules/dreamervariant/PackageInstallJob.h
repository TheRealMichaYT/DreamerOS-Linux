// PackageInstallJob.h
// Runs at exec-time (the "Installing..." screen, page10): pacstraps the
// package list chosen on page03/03b into the target system. Each package
// installed is reported back as a line for the live log list — that's the
// "no fake progress bar, a real live log" design from the original review.

#pragma once

#include <Job.h>
#include <QStringList>

class PackageInstallJob : public Calamares::Job
{
    Q_OBJECT
public:
    explicit PackageInstallJob( const QStringList& packages, QObject* parent = nullptr );

    QString prettyName() const override;
    Calamares::JobResult exec() override;

private:
    QStringList m_packages;
};
