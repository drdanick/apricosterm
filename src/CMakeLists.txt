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
