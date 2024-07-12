cmake_minimum_required(VERSION 3.16)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core)

add_executable(helper main.cpp privileges.cpp privileges.h)
set_target_properties(helper PROPERTIES
	WIN32_EXECUTABLE TRUE
	CXX_STANDARD 20
	LINK_FLAGS
		"/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)
target_link_libraries(helper PRIVATE Qt6::Core)

install(FILES $<TARGET_FILE:helper> DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:helper> DESTINATION pdb)
