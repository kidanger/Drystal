find_program(MAN_XSLTPROC xsltproc)
if (NOT MAN_XSLTPROC)
    message(FATAL_ERROR "Unable to find xsltproc")
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man1)

set(MAN_OUTPUTS "")
set(MAN_RESOURCES
	drystal.xsl
)

add_custom_target(man)
function(add_manpage volume title)
    list(APPEND MAN_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/man1/drystal.1")
	add_custom_command(TARGET man
		DEPENDS ${title}.${volume}.xml ${MAN_RESOURCES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND ${MAN_XSLTPROC} -nonet --xinclude -o "${CMAKE_CURRENT_BINARY_DIR}/man${volume}/" drystal.xsl ${title}.${volume}.xml
	)
endfunction()

add_manpage(1 drystal)

