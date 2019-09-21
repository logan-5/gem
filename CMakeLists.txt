cmake_minimum_required(VERSION 3.12)

project(gem)

find_package(PythonInterp 2.7 REQUIRED)

# TODO platform independent yada yada
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Weffc++ -Wconversion -g")

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

set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CONFIG_HEADER_PATH ${GENERATED_DIR}/${PROJECT_NAME}_config.hpp)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config_template.hpp.in
    ${CONFIG_HEADER_PATH}
    )
set(SRC ${SRC} ${CONFIG_HEADER_PATH})

set(OPCODE_SRC ${GENERATED_DIR}/opcodes.cpp)
add_custom_command(
    OUTPUT ${OPCODE_SRC}
    DEPENDS ${TOOLS_DIR}/gen_opcodes.py ${GENERATORS_DIR}/opcode.py
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/gen_opcodes.py ${GENERATORS_DIR}/opcode.py ${OPCODE_SRC})
set(SRC ${SRC} ${OPCODE_SRC})

set(BOOTSTRAP_SRC ${GENERATED_DIR}/bootstrap.cpp)
add_custom_command(
    OUTPUT ${BOOTSTRAP_SRC}
    DEPENDS ${TOOLS_DIR}/gen_bootstrap.py ${GENERATORS_DIR}/DMG_ROM.bin
    COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/gen_bootstrap.py ${GENERATORS_DIR}/DMG_ROM.bin ${BOOTSTRAP_SRC})
set(SRC ${SRC} ${BOOTSTRAP_SRC})

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

function(SET_SRC_HPP _NAME)
    set(SRC ${SRC} ${SRC_DIR}/${_NAME}.hpp PARENT_SCOPE)
endfunction()
function(SET_SRC_HPP_CPP _NAME)
    set(SRC ${SRC} ${SRC_DIR}/${_NAME}.cpp ${SRC_DIR}/${_NAME}.hpp PARENT_SCOPE)
endfunction()
function(SET_SRC_FILE _NAME)
    set(SRC ${SRC} ${SRC_DIR}/${_NAME} PARENT_SCOPE)
endfunction()

SET_SRC_HPP_CPP(alu)
SET_SRC_HPP_CPP(cpu)
SET_SRC_HPP_CPP(fs)
SET_SRC_HPP_CPP(gpu)
SET_SRC_HPP_CPP(io)
SET_SRC_HPP_CPP(mem)
SET_SRC_HPP_CPP(opcode)
SET_SRC_HPP_CPP(rom)
SET_SRC_HPP_CPP(screen)

SET_SRC_HPP(fwd)

SET_SRC_FILE(main)

## If you want to link SFML statically
# set(SFML_STATIC_LIBRARIES TRUE)

find_package(SFML 2.5 COMPONENTS graphics window REQUIRED)

set(INCLUDE_DIRS ${INCLUDE_DIRS} ${SRC_DIR})
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${GENERATED_DIR})
include_directories(${PROJECT_NAME} ${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
