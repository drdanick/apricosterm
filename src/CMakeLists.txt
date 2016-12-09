include (FindPkgConfig)
pkg_search_module (SDL2 REQUIRED sdl2)

set (apricosterm_src
    main.c
    screen.c
    managedtextures.c
    )

include_directories (${SDL2_INCLUDE_DIRS})
add_executable (${PROJECT_NAME} ${apricosterm_src})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${SDL2_LIBRARIES})
