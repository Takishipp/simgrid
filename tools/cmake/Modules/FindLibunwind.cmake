if(SIMGRID_PROCESSOR_x86_64)
  find_library(PATH_LIBUNWIND_LIB
    NAMES unwind-x86_64
    HINTS
    $ENV{SIMGRID_LIBUNWIND_LIBRARY_PATH}
    $ENV{LD_LIBRARY_PATH}
    $ENV{LIBUNWIND_LIBRARY_PATH}
    PATH_SUFFIXES lib/ GnuWin32/lib lib/system
    PATHS
    /opt
    /opt/local
    /opt/csw
    /sw
    /usr)
endif()

if(NOT PATH_LIBUNWIND_LIB)
  find_library(PATH_LIBUNWIND_LIB
    NAMES unwind
    HINTS
    $ENV{SIMGRID_LIBUNWIND_LIBRARY_PATH}
    $ENV{LD_LIBRARY_PATH}
    $ENV{LIBUNWIND_LIBRARY_PATH}
    PATH_SUFFIXES lib/ GnuWin32/lib lib/system
    PATHS
    /opt
    /opt/local
    /opt/csw
    /sw
    /usr
    /usr/lib/)
endif()

find_path(PATH_LIBUNWIND_H "libunwind.h"
  HINTS
  $ENV{SIMGRID_LIBUNWIND_LIBRARY_PATH}
  $ENV{LD_LIBRARY_PATH}
  $ENV{LIBUNWIND_LIBRARY_PATH}
  PATH_SUFFIXES include/ GnuWin32/include
  PATHS
  /opt
  /opt/local
  /opt/csw
  /sw
  /usr)

message(STATUS "Looking for libunwind.h")
if(PATH_LIBUNWIND_H)
  message(STATUS "Looking for libunwind.h - found")
else()
  message(STATUS "Looking for libunwind.h - not found")
endif()

message(STATUS "Looking for libunwind")
if(PATH_LIBUNWIND_LIB)
  message(STATUS "Looking for libunwind - found")
else()
  message(STATUS "Looking for libunwind - not found")
endif()

if(PATH_LIBUNWIND_LIB AND PATH_LIBUNWIND_H)
  string(REGEX REPLACE "/libunwind.*[.]${LIB_EXE}$" "" PATH_LIBUNWIND_LIB "${PATH_LIBUNWIND_LIB}")
  string(REGEX REPLACE "/libunwind.h"               "" PATH_LIBUNWIND_H   "${PATH_LIBUNWIND_H}")
      
  include_directories(${PATH_LIBUNWIND_H})
  link_directories(${PATH_LIBUNWIND_LIB})
  SET(HAVE_LIBUNWIND 1)
else()
  SET(HAVE_LIBUNWIND 0)
endif()

mark_as_advanced(PATH_LIBDW_H)
mark_as_advanced(PATH_LIBDW_LIB)
mark_as_advanced(PATH_LIBUNWIND_LIB)
mark_as_advanced(PATH_LIBUNWIND_H)
