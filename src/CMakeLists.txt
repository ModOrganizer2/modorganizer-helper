cmake_minimum_required(VERSION 3.16)

find_package(Qt5 COMPONENTS Core REQUIRED)
mo2_find_libraries(loot cpptoml)

add_executable(helper)
mo2_configure_executable(helper
	ELEVATED
	WARNINGS OFF
	PRIVATE_DEPENDS Qt::Core)
mo2_install_target(helper)
