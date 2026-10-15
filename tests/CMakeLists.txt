set (src ${PROJECT_SOURCE_DIR}/src)

# Protocol tests: the emulator against a fake renderer
add_executable (test_emulator
    test_emulator.c
    testing.c
    fakerenderer.c
    ${src}/terminalemulator.c
    )
target_include_directories (test_emulator PRIVATE ${src})
target_link_libraries (test_emulator PkgConfig::SDL2)

# Rendering tests: the real renderer, drawing off screen
add_executable (test_renderer
    test_renderer.c
    testing.c
    ${src}/terminalemulator.c
    ${src}/terminalrenderer.c
    ${src}/screen.c
    ${src}/managedtextures.c
    )
target_include_directories (test_renderer PRIVATE ${src})
target_compile_definitions (test_renderer PRIVATE
    APRICOSTERM_USE_SOFTWARE_RENDERER
    FONT_PATH="${PROJECT_SOURCE_DIR}/resources/vga8x16.png"
    )
target_link_libraries (test_renderer PkgConfig::SDL2 PkgConfig::SDL2IMAGE)

add_test (NAME emulator COMMAND test_emulator)
add_test (NAME renderer COMMAND test_renderer)
