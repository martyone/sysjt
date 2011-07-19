TEMPLATE = app
CONFIG += qdbus
TARGET = sysjt

HEADERS += job.h
SOURCES += job.cpp
DBUS_ADAPTORS += jobview.xml

HEADERS += joblistview.h
SOURCES += joblistview.cpp

HEADERS += jobmanager.h
SOURCES += jobmanager.cpp
DBUS_ADAPTORS += jobmanager.xml

HEADERS += singleton.h

SOURCES += sysjt.cpp

HEADERS += systemtrayicon.h
SOURCES += systemtrayicon.cpp
