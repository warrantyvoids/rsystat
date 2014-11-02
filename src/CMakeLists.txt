cmake_minimum_required( VERSION 2.8.12 )

find_package( PkgConfig REQUIRED )
pkg_check_modules(SSH REQUIRED libssh )
pkg_check_modules(CURSES REQUIRED ncursesw )

add_executable( rsystat main.cpp Connection.cpp Configuration.cpp OperatingSystems.cpp
			SSHStream.cpp )
include_directories(${SSH_INCLUDE_DIRS} )
include_directories(${CURSES_INCLUDE_DIRS} )
target_link_libraries( rsystat ${SSH_LIBRARIES} ${CURSES_LIBRARIES} )
