# -*- CMakeLists.txt generated by CodeLite IDE. Do not edit by hand -*-

cmake_minimum_required(VERSION 3.0)


#{{{{ User Code 01
# Place your code here
#}}}}

enable_language(CXX C ASM)
# Project name
project(simple_pool_without_metric)



#{{{{ User Code 02
# Place your code here
#}}}}

# This setting is useful for providing JSON file used by CodeLite for code completion
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CONFIGURATION_NAME "Release")

set(CL_WORKSPACE_DIRECTORY ../..)
# Set default locations
set(CL_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/${CL_WORKSPACE_DIRECTORY}/cmake-build-${CONFIGURATION_NAME}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CL_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CL_OUTPUT_DIRECTORY})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CL_OUTPUT_DIRECTORY})

# Projects


# Top project
# Define some variables
set(PROJECT_simple_pool_without_metric_PATH "${CMAKE_CURRENT_LIST_DIR}")
set(WORKSPACE_PATH "${CMAKE_CURRENT_LIST_DIR}/../..")



#{{{{ User Code 1
# Place your code here
#}}}}

include_directories(
    .
    .

)


# Compiler options
add_definitions(-O2)
add_definitions(-Wall)
add_definitions(
    -DNDEBUG
)


# Linker options


if(WIN32)
    # Resource options
endif(WIN32)

# Library path
link_directories(
    .
)

# Define the CXX sources
set ( CXX_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS 
    " -O2 -Wall")

if(WIN32)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> ${RC_OPTIONS} -O coff -i <SOURCE> -o <OBJECT>")
endif(WIN32)



#{{{{ User Code 2
# Place your code here
#}}}}

add_executable(simple_pool_without_metric ${RC_SRCS} ${CXX_SRCS} ${C_SRCS} ${ASM_SRCS})
target_link_libraries(simple_pool_without_metric ${LINK_OPTIONS})



#{{{{ User Code 3
# Place your code here
#}}}}

