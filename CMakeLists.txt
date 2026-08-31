cmake_minimum_required(VERSION 3.20)
project(DX11GameFramework CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
    message(WARNING
        "이 프로젝트는 Windows + DirectX 11 전용입니다. "
        "Windows(Visual Studio 2019 이상, Windows SDK 설치)에서 구성/빌드하세요.")
endif()

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h")

add_executable(DX11GameFramework WIN32 ${ENGINE_SOURCES})

target_include_directories(DX11GameFramework PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/third_party")

target_compile_definitions(DX11GameFramework PRIVATE UNICODE _UNICODE)

if (WIN32)
    target_link_libraries(DX11GameFramework PRIVATE
        d3d11.lib
        dxgi.lib
        d3dcompiler.lib
        windowscodecs.lib)
endif()

# 실행 파일과 같은 폴더에서 Shaders / Assets 를 상대 경로로 찾으므로,
# 빌드 결과물 옆으로 복사해준다.
add_custom_command(TARGET DX11GameFramework POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Shaders" "$<TARGET_FILE_DIR:DX11GameFramework>/Shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:DX11GameFramework>/Assets"
)
