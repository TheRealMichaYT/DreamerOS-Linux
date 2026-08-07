calamares_add_plugin( dreamernetwork
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        DreamerNetworkViewStep.cpp
        DreamerNetworkPage.cpp
        NetworkManagerProxy.cpp
    LINK_PRIVATE_LIBRARIES
        calamares
        Qt5::DBus
        Qt5::Widgets
    SHARED_LIB
)
