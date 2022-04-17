cmake_minimum_required(VERSION 3.16)

add_executable(helper)
mo2_configure_executable(helper
	ELEVATED
	WARNINGS OFF
	TRANSLATIONS OFF
	PRIVATE_DEPENDS Qt::Core)
mo2_install_target(helper)
