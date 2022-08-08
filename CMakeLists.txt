cmake_minimum_required(VERSION 3.0)

project(GitLabHook)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include_directories(
    ${MAGIC}
    ${MAGIC}/Include
    ${PROJECT_BINARY_DIR}/Include
    ${PROJECT_SOURCE_DIR}/Include
    ${MAGIC}/ThirdParty/Asio/Include
    ${MAGIC}/ThirdParty/RapidJSON/Include
)
include(${MAGIC}/Magic.cmake)
link_directories(${MAGIC}/Lib)

set(MODULES
    ${MAGIC}/Modules/Magic.json
    ${MAGIC}/Modules/WebServer.json
    ${PROJECT_SOURCE_DIR}/Modules/Servlet.json
)

add_custom_target(Gen ALL)
add_custom_command(
    TARGET Gen
    COMMAND ${MAGIC}/Bin/Gen ${MODULES} ${PROJECT_BINARY_DIR}/Include/GitLabHook
)

add_executable(GitLabHook
    Source/main.cpp
    Source/Servlet/HookServlet.cpp
)

target_link_libraries(GitLabHook Magic ${MAGIC_DEPEND_LIBRARY})