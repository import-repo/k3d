FILE(GLOB ICO_FILES *.ico)
FILE(GLOB PNG_FILES *.png)

INSTALL(FILES ${ICO_FILES} ${PNG_FILES} DESTINATION share/k3d/icons)

