
if(NOT APPLE)
    # Configure the .desktop entries with the arch suffix
    configure_file(
    misc/linux/org.openmoh.openmohaa.desktop.in
    ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaa.desktop
    @ONLY
    )
    configure_file(
    misc/linux/org.openmoh.openmohaab.desktop.in
    ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaab.desktop
    @ONLY
    )
    configure_file(
    misc/linux/org.openmoh.openmohaas.desktop.in
    ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaas.desktop
    @ONLY
    )

    # Install .desktop entries
    install(FILES ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaa.desktop  DESTINATION ${INSTALL_DATADIR_FULL}/applications)
    install(FILES ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaab.desktop DESTINATION ${INSTALL_DATADIR_FULL}/applications)
    install(FILES ${CMAKE_BINARY_DIR}/misc/linux/org.openmoh.openmohaas.desktop DESTINATION ${INSTALL_DATADIR_FULL}/applications)

    install(FILES misc/linux/org.openmoh.openmohaa.metainfo.xml DESTINATION ${INSTALL_DATADIR_FULL}/metainfo)

    install(FILES misc/openmohaa.svg DESTINATION ${INSTALL_DATADIR_FULL}/icons/hicolor/symbolic/apps/ RENAME org.openmoh.openmohaa.svg)
endif()
