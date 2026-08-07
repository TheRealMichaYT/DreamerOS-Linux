calamares_add_plugin( dreamerdesktop
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        DreamerDesktopViewStep.cpp
        DreamerDesktopPage.cpp
        ApplyDesktopJob.cpp
    LINK_PRIVATE_LIBRARIES
        calamares
        Qt5::Widgets
    SHARED_LIB
)
