calamares_add_plugin( dreamervariant
    TYPE viewmodule
    EXPORT_MACRO PLUGINDLLEXPORT_PRO
    SOURCES
        DreamerVariantViewStep.cpp
        DreamerVariantPage.cpp
        PackageInstallJob.cpp
        ScratchStripJob.cpp
    LINK_PRIVATE_LIBRARIES
        calamares
        Qt5::Widgets
    SHARED_LIB
)
