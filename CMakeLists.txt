cmake_minimum_required(VERSION 3.12)

set(NAME picovision-uc)

include(pimoroni_pico_import.cmake)
include(pico_sdk_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()


# Add your source files
add_executable(${NAME}
    main.cpp # <-- Add source files here!
    usb_host.cpp
)

# Don't forget to link the libraries you need!
target_link_libraries(${NAME}
    pico_stdlib
    pico_rand
    tinyusb_board
    tinyusb_host
)

target_include_directories(${NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_compile_definitions(${NAME} PRIVATE
    USB_HOST=1
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

pico_enable_stdio_uart(${NAME} 1)
pico_enable_stdio_usb(${NAME} 0)

target_link_options(${NAME} PRIVATE
    -Wl,--print-memory-usage
)

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
