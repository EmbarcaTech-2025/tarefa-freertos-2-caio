cmake_minimum_required(VERSION 3.12)

# Detectar home do usuário
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

# Config VSCode Extension
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# Board
set(PICO_BOARD pico_w CACHE STRING "Board type")

# FreeRTOS
if (DEFINED ENV{FREERTOS_PATH})
  set(FREERTOS_PATH $ENV{FREERTOS_PATH})
else()
  set(FREERTOS_PATH ${CMAKE_CURRENT_LIST_DIR}/FreeRTOS)
endif()
message("FreeRTOS Kernel located in ${FREERTOS_PATH}")

# Import SDK e FreeRTOS
include(pico_sdk_import.cmake)
include(${FREERTOS_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Projeto
project(embarcatech-tarefa-freertos-2 C CXX ASM)
pico_sdk_init()

# Arquivos
add_executable(embarcatech-tarefa-freertos-2
    main.c
    src/mqtt_comm.c
    src/wifi_conn.c
    src/xor_cipher.c
    src/ssd1306_i2c.c
)

# Includes
target_include_directories(embarcatech-tarefa-freertos-2 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
)
pico_enable_stdio_uart(embarcatech-tarefa-freertos-2 0)
pico_enable_stdio_usb(embarcatech-tarefa-freertos-2 1)

# Bibliotecas
target_link_libraries(embarcatech-tarefa-freertos-2
    pico_stdlib
    hardware_i2c
    hardware_adc
    pico_cyw43_driver
    pico_lwip
    pico_lwip_mqtt
    pico_cyw43_arch_lwip_threadsafe_background
    FreeRTOS-Kernel-Heap4
)

# Geração de UF2, bin, etc.
pico_add_extra_outputs(embarcatech-tarefa-freertos-2)
