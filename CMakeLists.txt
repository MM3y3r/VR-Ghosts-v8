cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake/Modules)

include(AddProject)

project(opensg)

set(OPENSG_MODULES OSGBase OSGSystem OSGWindow OSGWindowGLUT OSGDrawable OSGUtil OSGGroup OSGFileIO OSGImageFileIO)

add_project(OpenSG
	SOURCES "src/*.*"
	CONFIGS "config/*.*"
	MODELS "models/*.*"
	IMAGES "images/*.*"
	DOCS "snippets/*.*"
	OPENSG_MODULES ${OPENSG_MODULES}
)

add_project(OpenSGFinal
	SOURCES "final/src/*.*"
	CONFIGS "final/config/*.*"
	MODELS "models/*.*"
	IMAGES "images/*.*"
	DOCS "snippets/*.*"
	OPENSG_MODULES ${OPENSG_MODULES}
)
