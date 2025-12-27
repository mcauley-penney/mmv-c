cmake_minimum_required(VERSION 3.5)

set(NAME mmv)

project(${NAME}-c C)

set(SRCDIR ${CMAKE_SOURCE_DIR}/src)

include_directories(./includes/)

file(GLOB SRCS ${SRCDIR}/*.c)

add_executable(${NAME} ${SRCS})


option(TESTING "Enable tests executables" OFF)

if(TESTING)
		message(STATUS "Enable testing")
		enable_testing()
		add_subdirectory(libs/Unity EXCLUDE_FROM_ALL)
		add_subdirectory(test)
endif()


include(GNUInstallDirs)

install(TARGETS ${NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_SOURCE_DIR}/man/${NAME}.1.gz DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
