TEMPLATE = subdirs

isEmpty(QT_LIBINFIX):symbian {
    include(../../staticconfig.pri)
    load(data_caging_paths)
    include($$QT_MOBILITY_BUILD_TREE/config.pri)

    SUBDIRS =
    TARGET = "QtMobility"
    TARGET.UID3 = 0x2002AC89

    VERSION = 1.0.3

    vendorinfo = \
        "; Localised Vendor name" \
        "%{\"Nokia\"}" \
        " " \
        "; Unique Vendor name" \
        ":\"Nokia\"" \
        " "
    qtmobilitydeployment.pkg_prerules += vendorinfo

    epoc31 = $$(EPOCROOT31)
    epoc32 = $$(EPOCROOT32)
    epoc50 = $$(EPOCROOT50)

    # default to EPOCROOT if EPOCROOTxy not defined
    isEmpty(epoc31) {
        EPOCROOT31 = $${EPOCROOT}
    } else {
        EPOCROOT31 = $$(EPOCROOT31)
    }
    isEmpty(epoc32) {
        EPOCROOT32 = $${EPOCROOT}
    }else {
        EPOCROOT32 = $$(EPOCROOT32)
    }
    isEmpty(epoc50) {
        EPOCROOT50 = $${EPOCROOT}
    } else {
        EPOCROOT50 = $$(EPOCROOT50)
    }

    #Symbian^3 and beyond requires special package flags
    #we cannot use S60_VERSION == 5.2 as Qt 4.6.x does not define it yet
    #see $QTDIR/mkspecs/common/symbian/symbian.conf for details
    exists($${EPOCROOT}epoc32/release/winscw/udeb/z/system/install/series60v5.2.sis)|exists($${EPOCROOT}epoc32/data/z/system/install/series60v5.2.sis)|exists($${EPOCROOT}epoc32/release/armv5/lib/libstdcppv5.dso) {
        pkg_version = $$replace(VERSION,"\.",",")
        qtmobilitydeployment.pkg_prerules += "$${LITERAL_HASH}{\"QtMobility\"},(0x2002AC89),$${pkg_version},TYPE=SA,RU,NR"
    }

    contains(mobility_modules, messaging): qtmobilitydeployment.sources += \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtMessaging.dll
	
    contains(mobility_modules, serviceframework) { 
        qtmobilitydeployment.sources += \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtServiceFramework.dll \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qsfwdatabasemanagerserver.exe
        contains(QT_CONFIG, declarative): {
            qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/declarative_serviceframework.dll
            pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\serviceframework\\qmakepluginstubs\\declarative_serviceframework.qtplugin\"  - \"!:\\resource\\qt\\imports\\QtMobility\\serviceframework\\declarative_serviceframework.qtplugin\""
            qmldirs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\serviceframework\\qmldir\"  - \"!:\\resource\\qt\\imports\\QtMobility\\serviceframework\\qmldir\""
        }
    }

    contains(mobility_modules, location): qtmobilitydeployment.sources += \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtLocation.dll

    contains(mobility_modules, systeminfo): qtmobilitydeployment.sources += \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtSystemInfo.dll

    contains(mobility_modules, publishsubscribe) {
        qtmobilitydeployment.sources += \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtPublishSubscribe.dll \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qpspathmapperserver.exe
        contains(QT_CONFIG, declarative) {
            qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/declarative_publishsubscribe.dll
            pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\publishsubscribe\\qmakepluginstubs\\declarative_publishsubscribe.qtplugin\"  - \"!:\\resource\\qt\\imports\\QtMobility\\publishsubscribe\\declarative_publishsubscribe.qtplugin\""
            qmldirs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\publishsubscribe\\qmldir\"  - \"!:\\resource\\qt\\imports\\QtMobility\\publishsubscribe\\qmldir\""
        }
    }

    contains(mobility_modules, versit): qtmobilitydeployment.sources += \
        $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtVersit.dll


    contains(mobility_modules, bearer) {
        bearer = \
            "IF package(0x1028315F)" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtBearer.dll\" - \"!:\\sys\\bin\\QtBearer.dll\"" \
            "ELSEIF package(0x102752AE)" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtBearer.dll\" - \"!:\\sys\\bin\\QtBearer.dll\"" \
            "ELSEIF package(0x102032BE)" \
            "   \"$${EPOCROOT31}epoc32/release/$(PLATFORM)/$(TARGET)/QtBearer.dll\" - \"!:\\sys\\bin\\QtBearer.dll\"" \
            "ELSE" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtBearer.dll\" - \"!:\\sys\\bin\\QtBearer.dll\"" \
            "ENDIF"

        qtmobilitydeployment.pkg_postrules += bearer
    }

    contains(mobility_modules, contacts) {

        qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtContacts.dll

        contacts = \
            "IF package(0x1028315F)" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbian.dll\" - \"!:\\sys\\bin\\qtcontacts_symbian.dll\"" \
            "ELSEIF package(0x102752AE)" \
            "   \"$${EPOCROOT32}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbian.dll\" - \"!:\\sys\\bin\\qtcontacts_symbian.dll\"" \
            "ELSEIF package(0x102032BE)" \
            "   \"$${EPOCROOT31}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbian.dll\" - \"!:\\sys\\bin\\qtcontacts_symbian.dll\"" \
            "ELSE" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbian.dll\" - \"!:\\sys\\bin\\qtcontacts_symbian.dll\"" \
            "ENDIF"

        qtmobilitydeployment.pkg_postrules += contacts

        pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE/plugins/contacts/symbian/qmakepluginstubs/qtcontacts_symbian.qtplugin\"  - \"!:\\resource\\qt\\plugins\\contacts\\qtcontacts_symbian.qtplugin\""

        contains(symbiancntsim_enabled, yes) {
            pluginstubs += \
                "\"$$QT_MOBILITY_BUILD_TREE/plugins/contacts/symbiansim/qmakepluginstubs/qtcontacts_symbiansim.qtplugin\"  - \"!:\\resource\\qt\\plugins\\contacts\\qtcontacts_symbiansim.qtplugin\""

            symbiancntsim = \
                "IF package(0x1028315F)" \
                "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbiansim.dll\" - \"!:\\sys\\bin\\qtcontacts_symbiansim.dll\"" \
                "ELSEIF package(0x102752AE)" \
                "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbiansim.dll\" - \"!:\\sys\\bin\\qtcontacts_symbiansim.dll\"" \
                "ELSEIF package(0x102032BE)" \
                "   \"$${EPOCROOT31}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbiansim.dll\" - \"!:\\sys\\bin\\qtcontacts_symbiansim.dll\"" \
                "ELSE" \
                "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtcontacts_symbiansim.dll\" - \"!:\\sys\\bin\\qtcontacts_symbiansim.dll\"" \
                "ENDIF"

            qtmobilitydeployment.pkg_postrules += symbiancntsim
        }
	 contains(QT_CONFIG, declarative): {
            qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/declarative_contacts.dll
            pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\contacts\\qmakepluginstubs\\declarative_contacts.qtplugin\"  - \"!:\\resource\\qt\\imports\\QtMobility\\contacts\\declarative_contacts.qtplugin\""
            qmldirs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\contacts\\qmldir\"  - \"!:\\resource\\qt\\imports\\QtMobility\\contacts\\qmldir\""
        }
    }

    contains(mobility_modules, multimedia) {

        qtmobilitydeployment.sources += \
            $$(EPOCROOT50)epoc32/release/$(PLATFORM)/$(TARGET)/QtMultimediaKit.dll \
            $$(EPOCROOT50)epoc32/release/$(PLATFORM)/$(TARGET)/qtmultimediakit_m3u.dll

        multimedia = \
            "IF package(0x1028315F)" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtmultimediakit_mmfengine.dll\" - \"!:\\sys\\bin\\qtmultimediakit_mmfengine.dll\"" \
            "ELSEIF package(0x102752AE)" \
            "   \"$${EPOCROOT32}epoc32/release/$(PLATFORM)/$(TARGET)/qtmultimediakit_mmfengine.dll\" - \"!:\\sys\\bin\\qtmultimediakit_mmfengine.dll\"" \
            "ELSEIF package(0x102032BE)" \
            "   \"$${EPOCROOT31}epoc32/release/$(PLATFORM)/$(TARGET)/qtmultimediakit_mmfengine.dll\" - \"!:\\sys\\bin\\qtmultimediakit_mmfengine.dll\"" \
            "ELSE" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtmultimediakit_mmfengine.dll\" - \"!:\\sys\\bin\\qtmultimediakit_mmfengine.dll\"" \
            "ENDIF"


        qtmobilitydeployment.pkg_postrules += multimedia

        contains(openmaxal_symbian_enabled, yes) {
            openmax = \
                "IF package(0x20022E6D)" \
                "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtmultimediakit_xarecordservice.dll\" - \"!:\\sys\\bin\\qtmultimediakit_xarecordservice.dll\"" \
                "ENDIF"

            qtmobilitydeployment.pkg_postrules += openmax

            pluginstubs += \
                "IF package(0x20022E6D)" \
                    "\"$$QT_MOBILITY_BUILD_TREE/plugins/multimedia/symbian/openmaxal/mediarecorder/qmakepluginstubs/qtmultimediakit_xarecordservice.qtplugin\" - \"!:\\resource\\qt\\plugins\\mediaservice\\qtmultimediakit_xarecordservice.qtplugin\"" \
                "ENDIF"
        }

        pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE/plugins/multimedia/symbian/mmf/qmakepluginstubs/qtmultimediakit_mmfengine.qtplugin\" - \"!:\\resource\\qt\\plugins\\mediaservice\\qtmultimediakit_mmfengine.qtplugin\"" \
            "\"$$QT_MOBILITY_BUILD_TREE/plugins/multimedia/m3u/qmakepluginstubs/qtmultimediakit_m3u.qtplugin\"     - \"!:\\resource\\qt\\plugins\\playlistformats\\qtmultimediakit_m3u.qtplugin\"" \
	    
	contains(QT_CONFIG, declarative): {
            qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/declarative_multimedia.dll
            pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\multimedia\\qmakepluginstubs\\declarative_multimedia.qtplugin\"  - \"!:\\resource\\qt\\imports\\Qt\\multimedia\\declarative_multimedia.qtplugin\""
            qmldirs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\multimedia\\qmldir\"  - \"!:\\resource\\qt\\imports\\Qt\\multimedia\\qmldir\""
        }
    }

    contains(mobility_modules, sensors) {

        equals(sensors_symbian_enabled,yes) {
            sensorplugin=symbian
        } else:equals(sensors_s60_31_enabled,yes) {
            sensorplugin=s60_sensor_api
        } else {
            error("Must have a Symbian sensor backend available")
        }

        qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/QtSensors.dll

        sensors = \
            "IF package(0x1028315F)" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_sym.dll\" - \"!:\\sys\\bin\\qtsensors_sym.dll\"" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_generic.dll\" - \"!:\\sys\\bin\\qtsensors_generic.dll\"" \
            "ELSEIF package(0x102752AE)" \
            "   \"$${EPOCROOT32}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_sym.dll\" - \"!:\\sys\\bin\\qtsensors_sym.dll\"" \
            "   \"$${EPOCROOT32}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_generic.dll\" - \"!:\\sys\\bin\\qtsensors_generic.dll\"" \
            "ELSEIF package(0x102032BE)" \
            "   \"$${EPOCROOT31}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_sym.dll\" - \"!:\\sys\\bin\\qtsensors_sym.dll\"" \
            "   \"$${EPOCROOT31}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_generic.dll\" - \"!:\\sys\\bin\\qtsensors_generic.dll\"" \
            "ELSE" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_sym.dll\" - \"!:\\sys\\bin\\qtsensors_sym.dll\"" \
            "   \"$${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/qtsensors_generic.dll\" - \"!:\\sys\\bin\\qtsensors_generic.dll\"" \
            "ENDIF"

        pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE/plugins/sensors/$$sensorplugin/qmakepluginstubs/qtsensors_sym.qtplugin\"  - \"!:\\resource\\qt\\plugins\\sensors\\qtsensors_sym.qtplugin\"" \
            "\"$$QT_MOBILITY_BUILD_TREE/plugins/sensors/generic/qmakepluginstubs/qtsensors_generic.qtplugin\"  - \"!:\\resource\\qt\\plugins\\sensors\\qtsensors_generic.qtplugin\""

        !isEmpty(sensors):qtmobilitydeployment.pkg_postrules += sensors
        contains(QT_CONFIG, declarative): {
            qtmobilitydeployment.sources += \
            $${EPOCROOT50}epoc32/release/$(PLATFORM)/$(TARGET)/declarative_sensors.dll
            pluginstubs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\sensors\\qmakepluginstubs\\declarative_sensors.qtplugin\"  - \"!:\\resource\\qt\\imports\\QtMobility\\sensors\\declarative_sensors.qtplugin\""
            qmldirs += \
            "\"$$QT_MOBILITY_BUILD_TREE\\plugins\\declarative\\sensors\\qmldir\"  - \"!:\\resource\\qt\\imports\\QtMobility\\sensors\\qmldir\""
        }
    }

    !isEmpty(pluginstubs):qtmobilitydeployment.pkg_postrules += pluginstubs
    !isEmpty(qmldirs):qtmobilitydeployment.pkg_postrules += qmldirs

    qtmobilitydeployment.path = /sys/bin

    # Support backup and restore for QtMobility libraries and applications
    mobilitybackup.sources = backup_registration.xml
    mobilitybackup.path = c:/private/10202D56/import/packages/$$replace(TARGET.UID3, 0x,)

    DEPLOYMENT += qtmobilitydeployment\
                mobilitybackup

    BLD_INF_RULES.prj_exports += "./qtmobility.iby           $$CORE_MW_LAYER_IBY_EXPORT_PATH(qtmobility.iby)"
    
} else {
    message(Deployment of infixed library names not supported)
}
