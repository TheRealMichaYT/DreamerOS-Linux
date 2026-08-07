// main.cpp

#include "MainWindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int
main( int argc, char** argv )
{
    QApplication app( argc, argv );
    app.setApplicationName( "DreamerOS Installer" );
    app.setOrganizationName( "DreamerOS" );

    // Load the gold-and-navy theme (dreameros.qss) matching the mockups —
    // every page's colors, card panels, and buttons come from this one
    // stylesheet rather than being hardcoded per widget.
    QFile styleFile( ":/dreameros.qss" );
    if ( styleFile.open( QFile::ReadOnly | QFile::Text ) )
    {
        QTextStream stream( &styleFile );
        app.setStyleSheet( stream.readAll() );
    }

    MainWindow window;
    window.resize( 1920, 1080 );
    window.show();

    return app.exec();
}
