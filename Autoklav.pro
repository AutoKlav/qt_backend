QT -= gui
QT += sql serialport concurrent

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        autoklav.grpc.pb.cc \
        autoklav.pb.cc \
        dbmanager.cpp \
        globalerrors.cpp \
        globals.cpp \
        grpcserver.cpp \
        logger.cpp \
        main.cpp \
        master.cpp \
        mockserial.cpp \
        processlog.cpp \
        sensor.cpp \
        serial.cpp \
        statemachine.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    autoklav.grpc.pb.h \
    autoklav.pb.h \
    dbmanager.h \
    globalerrors.h \
    globals.h \
    grpcserver.h \
    logger.h \
    master.h \
    mockserial.h \
    processlog.h \
    sensor.h \
    serial.h \
    statemachine.h


INCLUDEPATH += /opt/vcpkg/installed/x64-linux/include

# Generate .proto files with (myb reusable from linux)
# $ protoc --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` autoklav.proto
# $ protoc --cpp_out=. autoklav.proto
# Add generated files to project

# Install grpc via vcpkg:
# vcpkg install grpc
# Current version of grpc on vcpkg is too old (1.51.1) for protoc version 24.2
# Install protobuf & protobuf-c /w vcpkg
# Eg $ /opt/vcpkg/packages/protobuf_x64-linux/tools/protobuf/protoc --grpc_out=. --plugin=protoc-gen-grpc="/opt/vcpkg/packages/grpc_x64-linux/tools/grpc/grpc_cpp_plugin" autoklav.proto
CONFIG(debug, debug|release) {
    LIBS += `PKG_CONFIG_PATH=/opt/vcpkg/installed/x64-linux/debug/lib/pkgconfig pkg-config --libs protobuf grpc++`
} else {
    LIBS += `PKG_CONFIG_PATH=/opt/vcpkg/installed/x64-linux/lib/pkgconfig pkg-config --libs protobuf grpc++`
}

DISTFILES += \
    autoklav.proto \
    init.db


