cmake_minimum_required(VERSION 3.9)
project(Cloth_sim LANGUAGES C CXX)

message("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(SOURCEFILES src/main.cpp src/cloth.cpp src/mesh.cpp)
set(HEADERFILES include/cloth.h include/mesh.h)

add_executable(${PROJECT_NAME} ${SOURCEFILES} ${HEADERFILES})

add_subdirectory(extern/glfw)
add_subdirectory(extern/glm)
add_subdirectory(extern/glad)
add_subdirectory(extern/stbimage)
add_subdirectory(extern/tinyobjloader)

find_package(OpenGL REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} glad glfw glm stbimage tinyobjloader OpenGL::GL)

set(DATA_DIR_BUILD ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(DATA_DIR_INSTALL ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/data)

message(STATUS "Data dir (in build tree): ${DATA_DIR_BUILD}")
message(STATUS "Data dir (in install tree): ${DATA_DIR_INSTALL}")

# Configure a header file to pass some of the CMake settings to the source code
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

install(
    DIRECTORY data/
    DESTINATION ${DATA_DIR_INSTALL}
)