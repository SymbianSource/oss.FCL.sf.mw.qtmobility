include(../staticconfig.pri)

TEMPLATE = subdirs

contains(mobility_modules,contacts): SUBDIRS += contacts
contains(mobility_modules,multimedia): SUBDIRS += multimedia
contains(mobility_modules,sensors): SUBDIRS += sensors
contains(mobility_modules,versit): SUBDIRS += versit

#contains(QT_CONFIG,declarative): SUBDIRS += declarative
