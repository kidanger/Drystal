include(GNUInstallDirs)

find_program(MAN_XSLTPROC xsltproc)
if (NOT MAN_XSLTPROC)
    message(FATAL_ERROR "Unable to find xsltproc")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man1)

set(MAN_RESOURCES
	drystal.xsl
)

add_custom_target(man)
install(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_MAKE_PROGRAM} man)")
function(add_manpage volume title)
	add_custom_command(TARGET man
		DEPENDS ${title}.${volume}.xml ${MAN_RESOURCES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${MAN_XSLTPROC} -nonet --xinclude -o "${CMAKE_CURRENT_BINARY_DIR}/man${volume}/" drystal.xsl ${title}.${volume}.xml
	)
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/man${volume}/${title}.${volume}" DESTINATION "${CMAKE_INSTALL_MANDIR}/man${volume}")
endfunction()

add_manpage(1 drystal)

