cmake_minimum_required(VERSION 3.16)

project(f3d-stl-thumbnailer
    DESCRIPTION "STL file thumbnailer for KDE Dolphin, rendered with F3D"
    HOMEPAGE_URL "https://f3d.app/"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "6.6.2")
set(KF6_MIN_VERSION "6.0.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS KIO)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(stlthumbnail_SRCS stlcreator.cpp)

add_library(stlthumbnail MODULE ${stlthumbnail_SRCS})

target_link_libraries(stlthumbnail
    Qt6::Widgets
    KF6::KIOWidgets
)

install(TARGETS stlthumbnail DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/thumbcreator/)

# Test harness: instantiates the creator directly and asserts a non-blank render.
# NOTE: does not compile against Qt 6.12.0 — that release dropped `noexcept` from
# `QVariant::~QVariant`, which breaks `QVariantList` default construction
# (`static_assert(std::is_nothrow_destructible_v<QVariant>)` in QList). Enable on
# Qt < 6.12 or >= 6.12.1. Build with: cmake -DF3D_STL_BUILD_TESTING=ON ...
option(F3D_STL_BUILD_TESTING "Build the test harness" OFF)
if(F3D_STL_BUILD_TESTING)
    add_executable(test_stlcreator test/test_stlcreator.cpp ${stlthumbnail_SRCS})
    target_link_libraries(test_stlcreator
        Qt6::Widgets
        KF6::KIOWidgets
    )
endif()
