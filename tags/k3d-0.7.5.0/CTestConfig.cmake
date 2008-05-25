SET(CTEST_PROJECT_NAME "K-3D")
SET(CTEST_NIGHTLY_START_TIME "02:00:00 MST")

IF(NOT DEFINED CTEST_DROP_METHOD)
  SET(CTEST_DROP_METHOD "http")
ENDIF(NOT DEFINED CTEST_DROP_METHOD)

IF(CTEST_DROP_METHOD STREQUAL "http")
  SET(CTEST_DROP_SITE "www.k-3d.org")
  SET(CTEST_DROP_LOCATION "/cdash/submit.php?project=K-3D")
  SET(CTEST_TRIGGER_SITE "")
ENDIF(CTEST_DROP_METHOD STREQUAL "http")
