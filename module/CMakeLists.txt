add_library(AnnounceMod MODULE Main.cpp ConvertUTF.cpp)

if(FORCE_32BIT_BIN)
	set_target_properties(AnnounceMod PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	target_compile_definitions(AnnounceMod PRIVATE _SQ64)
endif()

set_target_properties(AnnounceMod PROPERTIES PREFIX "")

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
		set_target_properties(AnnounceMod PROPERTIES OUTPUT_NAME "announce-win64")
	else()
		set_target_properties(AnnounceMod PROPERTIES OUTPUT_NAME "announce-win32")
	endif()
else()
	if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT FORCE_32BIT_BIN)
		set_target_properties(AnnounceMod PROPERTIES OUTPUT_NAME "announce-linux64")
	else()
		set_target_properties(AnnounceMod PROPERTIES OUTPUT_NAME "announce-linux32")
	endif()
endif(WIN32)

target_include_directories(AnnounceMod PRIVATE ${CMAKE_CURRENT_LIST_DIR})
