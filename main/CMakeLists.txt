
# File: main/CMakeLists.txt
# LilyGO THMI IDF Template Project
cmake_minimum_required(VERSION 3.16)

# Set usual component variables
set(
    app_sources
    "main.cpp"
    "display_port.c"
    "lvgl_port.c"
    "temp_sensor_cpu.cpp"
)

set(
    app_include_dirs
    "."
    ""
)

set(app_requires
    fmt
    littlefs
    lvgl
    filesystem-tdisplays3
    simple-cli-tdisplays3-v001
    )

set(
    app_priv_requires
    ${app_requires}
    esp_lcd
)


# Set my components
# These are components that are not part of the IDF 
# but are used in this project.
# Put this components in /lib after set 
# set(EXTRA_COMPONENT_DIRS lib) in root/CMakeLists.txt
set(
    my_components
)

idf_component_register(
    SRCS
    ${app_sources}
    INCLUDE_DIRS
    ${app_include_dirs}
    REQUIRES
    ${app_requires}
    ${my_components}
    PRIV_REQUIRES
    ${app_priv_requires}
    WHOLE_ARCHIVE
)

add_compile_options(
   -Wno-error
)


# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
# littlefs_create_partition_image(littlefs littlefs_data FLASH_IN_PROJECT)
# spiffs_create_partition_image(spiffs spiffs_data FLASH_IN_PROJECT)
