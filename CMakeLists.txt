cmake_minimum_required(VERSION 3.12)

project(gem)

find_package(PythonInterp 2.7 REQUIRED)

# TODO platform independent yada yada
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Weffc++ -Werror -Wconversion")

option(ASAN "ASAN" OFF)
if (ASAN)
    message("-- enabling address sanitizer")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    else (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message("address sanitizer not supported for this compiler")
    endif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
else (ASAN)
    message("-- not enabling address sanitizer")
endif (ASAN)

set(GENERATORS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generators)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(OPCODE_SRC ${GENERATED_DIR}/opcodes.cpp)
add_custom_command(
    OUTPUT ${OPCODE_SRC}
    DEPENDS ${TOOLS_DIR}/gen_opcodes.py ${GENERATORS_DIR}/opcode.py
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/gen_opcodes.py ${GENERATORS_DIR}/opcode.py ${OPCODE_SRC})
set_source_files_properties(${OPCODE_SRC} PROPERTIES OBJECT_DEPENDS ${GENERATED_DIR}/opcodes.cache)
set(SRC ${SRC} ${OPCODE_SRC})

set(BOOTSTRAP_SRC ${GENERATED_DIR}/bootstrap.cpp)
add_custom_command(
    OUTPUT ${BOOTSTRAP_SRC}
    DEPENDS ${TOOLS_DIR}/gen_bootstrap.py ${GENERATORS_DIR}/DMG_ROM.bin
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/gen_bootstrap.py ${GENERATORS_DIR}/DMG_ROM.bin ${BOOTSTRAP_SRC})
set_source_files_properties(${BOOTSTRAP_SRC} PROPERTIES OBJECT_DEPENDS ${GENERATED_DIR}/bootstrap.cache)
set(SRC ${SRC} ${BOOTSTRAP_SRC})

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

set(SRC ${SRC} ${SRC_DIR}/alu.cpp)
set(SRC ${SRC} ${SRC_DIR}/cpu.cpp)
set(SRC ${SRC} ${SRC_DIR}/mem.cpp)
set(SRC ${SRC} ${SRC_DIR}/main.cpp)
set(SRC ${SRC} ${SRC_DIR}/opcode.cpp)

set(INCLUDE_DIRS ${INCLUDE_DIRS} ${SRC_DIR})
include_directories(${PROJECT_NAME} ${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
