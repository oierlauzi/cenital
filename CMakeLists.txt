cmake_minimum_required(VERSION 3.9)
include(GNUInstallDirs)

#Project info
project(cenital 
	VERSION 0.1.0 
	DESCRIPTION "An open source vision mixer"
)

#Subdirectories
add_subdirectory(${PROJECT_SOURCE_DIR}/shaders/)
#add_subdirectory(${PROJECT_SOURCE_DIR}/doc/doxygen/)

#Set compiler's options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

#Register all source and header files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE INLINE_SOURCES ${PROJECT_SOURCE_DIR}/include/*.inl)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} shaders)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/)
target_include_directories(${PROJECT_NAME} PRIVATE ${SHADER_INCLUDE_DIR}/)
target_link_libraries(
	${PROJECT_NAME} PUBLIC 
	zuazo zuazo-window zuazo-ffmpeg zuazo-compositor
	avutil avformat avcodec swscale
	glfw pthread dl
)

# Install the executable
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
