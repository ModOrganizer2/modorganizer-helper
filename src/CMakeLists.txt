cmake_minimum_required(VERSION 3.16)

add_executable(helper main.cpp privileges.cpp privileges.h)
source_group(src FILES REGULAR_EXPRESSION ".*")
target_compile_definitions(helper PRIVATE UNICODE)
set_target_properties(helper PROPERTIES
	CXX_STANDARD 20
	LINK_FLAGS
		"/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\""
)
target_link_options(helper PRIVATE
	$<$<CONFIG:RelWithDebInfo>:/LTCG /INCREMENTAL:NO /OPT:REF /OPT:ICF>
)

install(FILES $<TARGET_FILE:helper> DESTINATION bin)
install(FILES $<TARGET_PDB_FILE:helper> DESTINATION pdb)
