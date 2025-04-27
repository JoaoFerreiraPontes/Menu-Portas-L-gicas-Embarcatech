# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == Configuração da Extensão VS Code (NÃO EDITAR) ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

# ATUALIZE TODAS AS VERSÕES PARA 2.1.1
set(sdkVersion 2.1.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.1)

set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# =======================================================

set(PICO_BOARD pico CACHE STRING "Board type")

# Importação do SDK com caminho explícito
set(PICO_SDK_PATH "${USERHOME}/.pico-sdk/sdk/2.1.1")  # Caminho absoluto corrigido
include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(Menu_PortasLogicas_JoaoPontes C CXX ASM)

# Inicialização do SDK
pico_sdk_init()

# Arquivos fonte
add_executable(Menu_PortasLogicas_JoaoPontes 
    Menu_PortasLogicas_JoaoPontes.c 
    libs/ssd1306.c
)

# Configurações do programa
pico_set_program_name(Menu_PortasLogicas_JoaoPontes "Portas_Logicas")
pico_set_program_version(Menu_PortasLogicas_JoaoPontes "1.0")

# Configurações de I/O
pico_enable_stdio_uart(Menu_PortasLogicas_JoaoPontes 0)
pico_enable_stdio_usb(Menu_PortasLogicas_JoaoPontes 1)

# Bibliotecas necessárias
target_link_libraries(Menu_PortasLogicas_JoaoPontes
    pico_stdlib
    hardware_i2c
    hardware_adc
    hardware_gpio
    hardware_pwm
)

# Diretórios de include
target_include_directories(Menu_PortasLogicas_JoaoPontes PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/libs
)

# Saídas adicionais
pico_add_extra_outputs(Menu_PortasLogicas_JoaoPontes)