function(detect_project_version)
    if(NOT DEFINED SKBUILD_PROJECT_VERSION OR SKBUILD_PROJECT_VERSION STREQUAL "")
        find_package(Git REQUIRED)
        # Get the latest tag
        execute_process(
                COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_TAG
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE GIT_RESULT
        )

        # Get the number of commit since the last tag
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-list ${GIT_TAG}..HEAD --count
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_COMMITS
                OUTPUT_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE GIT_COMMITS_RESULT
        )

        # Parse version components (expects format vX.Y.Z or vX.Y.Z-NUMCOMMITS-SHA)
        string(REGEX MATCH "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)" SEMVER_MATCHES "${GIT_TAG}")
        if(SEMVER_MATCHES)
            # Set the semver values
            set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
            set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
            set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
            # We also advance minor version by 1 in the patch version
            math(EXPR PROJECT_VERSION_PATCH "${PROJECT_VERSION_PATCH} + 1")
            # Of there are any git commit ahead the last tag, we add them as a "tweak" field
            if(${GIT_COMMITS})
                set(PROJECT_VERSION_TWEAK "${GIT_COMMITS}")
                set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.${PROJECT_VERSION_TWEAK}" PARENT_SCOPE)
            else()
                set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" PARENT_SCOPE)
            endif()
        else()
            message(WARNING "Could not parse version from git tag: ${GIT_TAG}: using '0.0.0.1'")
            set(PROJECT_VERSION "0.0.0.1" PARENT_SCOPE)
        endif()
    else()
        # The version was managed by skbuild (python wheel build)
        set(PROJECT_VERSION ${SKBUILD_PROJECT_VERSION} PARENT_SCOPE)
    endif()
endfunction()