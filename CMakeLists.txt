cmake_minimum_required(VERSION 3.21)
project(DoubleSwing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# -------- Core library (no SFML) --------
add_library(doubleswing_core
        src/engine.cpp
        src/drag.cpp
)
target_include_directories(doubleswing_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

# ================================================================
# Web / WASM build (Emscripten)
# ================================================================
if (EMSCRIPTEN)
    add_executable(doubleswing_web
            web/bindings.cpp
    )
    target_link_libraries(doubleswing_web PRIVATE doubleswing_core)

    # Output name/path for web/app.js import
    set_target_properties(doubleswing_web PROPERTIES
            OUTPUT_NAME "doubleswing"
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/web"
    )

    # linker flags
    target_link_options(doubleswing_web PRIVATE
            "SHELL:-s MODULARIZE=1"
            "SHELL:-s EXPORT_ES6=1"
            "SHELL:-s ENVIRONMENT=web"
            "SHELL:-s ALLOW_MEMORY_GROWTH=1"
            # export JS funcs
            "SHELL:-s EXPORTED_FUNCTIONS=['_ds_create','_ds_destroy','_ds_step','_ds_step_drag_p1','_ds_update_positions','_ds_x1','_ds_y1','_ds_x2','_ds_y2','_ds_set_th1','_ds_set_th2','_ds_set_w1','_ds_set_w2','_ds_reset']"
            "SHELL:-s EXPORTED_RUNTIME_METHODS=['cwrap','ccall']"
            "SHELL:-s MALLOC=emmalloc"
    )

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        target_link_options(doubleswing_web PRIVATE "SHELL:-O3")
    endif()

else()
    # -------- SFML (only needed for the SFML frontend target) --------
    FetchContent_Declare(
            SFML
            GIT_REPOSITORY https://github.com/SFML/SFML.git
            GIT_TAG 2.6.1
    )
    FetchContent_MakeAvailable(SFML)

    # -------- SFML app --------
    add_executable(doubleswing_sfml
            apps/desktop/main.cpp
            apps/desktop/sfml_app.cpp
    )

    target_link_libraries(doubleswing_sfml PRIVATE
            doubleswing_core
            sfml-graphics
            sfml-window
            sfml-system
    )

    if (WIN32)
        add_custom_command(TARGET doubleswing_sfml POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:sfml-graphics>
                $<TARGET_FILE:sfml-window>
                $<TARGET_FILE:sfml-system>
                $<TARGET_FILE_DIR:doubleswing_sfml>
        )
    endif()
endif()
