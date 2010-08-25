!symbian:error(This test is for Symbian packaging purposes only.)

TEMPLATE = subdirs

include(../../staticconfig.pri)

BLD_INF_RULES.prj_exports += "./qtmobilitytests.iby $$CUSTOMER_VARIANT_APP_LAYER_IBY_EXPORT_PATH(qtmobilitytests.iby)"
