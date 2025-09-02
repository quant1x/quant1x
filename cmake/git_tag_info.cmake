# =============================
# 通过git tag获取版本号, 作者, 邮箱, 日期
# =============================
function(get_latest_git_tag_info OUT_VERSION OUT_LATEST_TAG OUT_TAG_COMMIT_HASH OUT_AUTHOR OUT_EMAIL OUT_DATE)
    find_package(Git REQUIRED)

    execute_process(
        COMMAND git rev-list --tags --max-count=1
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE TAG_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (TAG_COMMIT_HASH)
        message(STATUS "Found latest commit hash: ${TAG_COMMIT_HASH}")
    else ()
        message(WARNING "No commit found in Git repository")
    endif ()
    execute_process(
        COMMAND git describe --tags ${TAG_COMMIT_HASH}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE LATEST_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (LATEST_TAG)
        message(STATUS "Found latest Git tag: ${LATEST_TAG}")
    else ()
        message(WARNING "No version tags found in Git repository")
    endif ()

    if (LATEST_TAG AND LATEST_TAG MATCHES "v?([0-9]+(\\.[0-9]+)*(\\.[0-9]+)?(-[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*)?(\\+[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*)?$)")
        set(VERSION_FROM_GIT "${CMAKE_MATCH_0}")
        if (VERSION_FROM_GIT MATCHES "^v(.*)")
            set(VERSION_FROM_GIT "${CMAKE_MATCH_1}")
        endif ()
    else ()
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        set(VERSION_FROM_GIT "0.0.0-dev+${GIT_COMMIT_HASH}")
    endif ()
    message(STATUS "Version: ${VERSION_FROM_GIT}")

    execute_process(
        COMMAND git log -1 ${LATEST_TAG} --pretty=format:%an||%ae||%ai
        OUTPUT_VARIABLE GIT_INFO
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    string(REPLACE "||" ";" GIT_INFO_LIST ${GIT_INFO})
    list(GET GIT_INFO_LIST 0 GIT_AUTHOR_NAME)
    list(GET GIT_INFO_LIST 1 GIT_AUTHOR_EMAIL)
    list(GET GIT_INFO_LIST 2 GIT_AUTHOR_DATE)

    message(STATUS " Author: ${GIT_AUTHOR_NAME}")
    message(STATUS "  Email: ${GIT_AUTHOR_EMAIL}")
    message(STATUS "   Date: ${GIT_AUTHOR_DATE}")

    set(${OUT_LATEST_TAG} "${LATEST_TAG}" PARENT_SCOPE)
    set(${OUT_TAG_COMMIT_HASH} "${TAG_COMMIT_HASH}" PARENT_SCOPE)
    set(${OUT_VERSION} "${VERSION_FROM_GIT}" PARENT_SCOPE)
    set(${OUT_AUTHOR} "${GIT_AUTHOR_NAME}" PARENT_SCOPE)
    set(${OUT_EMAIL} "${GIT_AUTHOR_EMAIL}" PARENT_SCOPE)
    set(${OUT_DATE} "${GIT_AUTHOR_DATE}" PARENT_SCOPE)
endfunction()
