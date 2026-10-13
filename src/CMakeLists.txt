include (FindPkgConfig)
pkg_search_module (SDL2 REQUIRED IMPORTED_TARGET sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED IMPORTED_TARGET SDL2_image>=2.0.0)

set (apricosterm
    main.c
    argparser.c
    screen.c
    managedtextures.c
    terminalemulator.c
    terminalrenderer.c
    serialtermio.c
    resources.c
    )

add_executable (${PROJECT_NAME} ${apricosterm})

install (TARGETS apricosterm DESTINATION bin)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PkgConfig::SDL2 PkgConfig::SDL2IMAGE)
