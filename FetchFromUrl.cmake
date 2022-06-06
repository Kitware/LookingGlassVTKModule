include(FetchContent)

# Before calling this, the following variables must be defined:
# FETCH_FROM_URL_PROJECT_NAME - a unique name for the project
# FETCH_FROM_URL_URL - the URL from which to download the project
# FETCH_FROM_URL_INSTALL_LOCATION - the location to unpack the project

FetchContent_Populate(${FETCH_FROM_URL_PROJECT_NAME}
  URL         ${FETCH_FROM_URL_URL}
  SOURCE_DIR  ${FETCH_FROM_URL_INSTALL_LOCATION}
  QUIET
  )

message(STATUS "Remote - FETCH_FROM_URL ${FETCH_FROM_URL_PROJECT_NAME} [OK]")
