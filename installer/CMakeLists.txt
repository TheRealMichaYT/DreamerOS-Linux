cmake_minimum_required( VERSION 3.16 )
project( dreameros-installer LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

find_package( Qt6 COMPONENTS Widgets DBus REQUIRED )

add_executable( dreameros-installer
    src/main.cpp
    src/MainWindow.cpp
    src/InstallerState.cpp
    src/InstallWorker.cpp
    src/PartitionUtil.cpp
    src/NetworkManagerClient.cpp
    src/pages/WelcomePage.cpp
    src/pages/NetworkPage.cpp
    src/pages/LocalePage.cpp
    src/pages/VariantPage.cpp
    src/pages/UsersPage.cpp
    src/pages/HostnamePage.cpp
    src/pages/DesktopPage.cpp
    src/pages/PartitionPage.cpp
    src/pages/SummaryPage.cpp
    src/pages/InstallPage.cpp
    src/pages/FinishedPage.cpp
    resources/resources.qrc
)

target_include_directories( dreameros-installer PRIVATE src )
target_link_libraries( dreameros-installer PRIVATE Qt6::Widgets Qt6::DBus )

install( TARGETS dreameros-installer RUNTIME DESTINATION bin )
install( FILES resources/dreameros.qss DESTINATION share/dreameros-installer )
