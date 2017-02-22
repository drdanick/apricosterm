include (FindPkgConfig)
pkg_search_module (SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

set (apricosterm
    main.c
    argparser.c
    screen.c
    managedtextures.c
    terminalemulator.c
    terminalrenderer.c
    serialtermio.c
    )

include_directories (${SDL2_INCLUDE_DIRS})
add_executable (${PROJECT_NAME} ${apricosterm})

install (TARGETS apricosterm DESTINATION bin)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
