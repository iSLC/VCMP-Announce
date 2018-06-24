add_library(Mongoose STATIC mongoose.c)

if(WIN32)
	target_link_libraries(Mongoose Ws2_32)
endif()

target_include_directories(Mongoose PRIVATE ${CMAKE_CURRENT_LIST_DIR})

if (CMAKE_BUILD_TYPE EQUAL "Release")
    target_compile_definitions(Mongoose PRIVATE CS_NDEBUG=1)
endif()
if (ENABLE_MG_STDIO)
	target_compile_definitions(Mongoose PRIVATE CS_ENABLE_STDIO=1)
endif()

if(FORCE_32BIT_BIN)
	set_target_properties(Mongoose PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
