remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(smartdatatest.cpp TEST_NAME smartdatatest
    LINK_LIBRARIES Qt5::Test statickdedsmart)

ecm_add_test(smartmonitortest.cpp TEST_NAME smartmonitortest
    LINK_LIBRARIES Qt5::Test statickdedsmart)
