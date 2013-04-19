# Install script for directory: /Users/hbridges/Downloads/Chipmunk-6.1.4/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/src/libchipmunk.6.1.3.dylib"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/src/libchipmunk.dylib"
    )
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libchipmunk.6.1.3.dylib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libchipmunk.dylib"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      EXECUTE_PROCESS(COMMAND "/usr/bin/install_name_tool"
        -id "libchipmunk.6.1.3.dylib"
        "${file}")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "${file}")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF()
  ENDFOREACH()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/hbridges/Downloads/Chipmunk-6.1.4/src/libchipmunk.a")
  IF(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libchipmunk.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libchipmunk.a")
    EXECUTE_PROCESS(COMMAND "/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libchipmunk.a")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/chipmunk" TYPE FILE FILES
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/chipmunk.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/chipmunk_ffi.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/chipmunk_private.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/chipmunk_types.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/chipmunk_unsafe.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpArbiter.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpBB.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpBody.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpPolyShape.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpShape.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpSpace.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpSpatialIndex.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/cpVect.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/chipmunk/constraints" TYPE FILE FILES
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpConstraint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpDampedRotarySpring.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpDampedSpring.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpGearJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpGrooveJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpPinJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpPivotJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpRatchetJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpRotaryLimitJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpSimpleMotor.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/cpSlideJoint.h"
    "/Users/hbridges/Downloads/Chipmunk-6.1.4/include/chipmunk/constraints/util.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

