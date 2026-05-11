cmake_minimum_required(VERSION 3.20)

project(ParkingLotOpenGL VERSION 0.1.0 LANGUAGES C CXX)



set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



option(PARKING_LOT_BUILD_TESTS "Build tests" OFF)



include(FetchContent)



# GLFW — window, context, input

FetchContent_Declare(

  glfw

  GIT_REPOSITORY https://github.com/glfw/glfw.git

  GIT_TAG 3.4

)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)



# GLM — math

FetchContent_Declare(

  glm

  GIT_REPOSITORY https://github.com/g-truc/glm.git

  GIT_TAG 1.0.1

)

set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm)



# STB — obrazy (tekstury z glTF)

FetchContent_Declare(

  stb

  GIT_REPOSITORY https://github.com/nothings/stb.git

  GIT_TAG master

)

FetchContent_MakeAvailable(stb)

# Assimp — wczytywanie glTF/glB

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

FetchContent_Declare(

  assimp

  GIT_REPOSITORY https://github.com/assimp/assimp.git

  GIT_TAG v5.3.1

)

FetchContent_MakeAvailable(assimp)



# GLAD 2 (generated OpenGL 3.3 Core loader, committed under third_party/glad)

add_library(glad STATIC third_party/glad/src/gl.c)

target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include)



add_executable(ParkingLot

  src/main.cpp

  src/app/Application.cpp

  src/core/Window.cpp

  src/gl/GlContext.cpp

  src/rendering/Shader.cpp

  src/rendering/Camera.cpp

  src/rendering/Renderer.cpp

  src/scene/ParkingGenerator.cpp

  src/scene/ParkingScene.cpp

  src/scene/ScreenRay.cpp

  src/models/GltfModel.cpp

  src/models/GrassBlades.cpp

  src/input/Input.cpp

  src/lighting/Lighting.cpp

)



target_include_directories(ParkingLot PRIVATE

  ${CMAKE_CURRENT_SOURCE_DIR}/src

  ${stb_SOURCE_DIR}

)



target_compile_definitions(ParkingLot PRIVATE

  "PARKING_ASSETS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\""

)



target_link_libraries(ParkingLot PRIVATE

  glad

  glfw

  glm::glm

  assimp::assimp

)



if(WIN32)

  target_link_libraries(ParkingLot PRIVATE opengl32)

elseif(APPLE)

  find_library(PARKING_OPENGL_FRAMEWORK OpenGL REQUIRED)

  target_link_libraries(ParkingLot PRIVATE ${PARKING_OPENGL_FRAMEWORK})

else()

  find_package(OpenGL REQUIRED)

  target_link_libraries(ParkingLot PRIVATE OpenGL::GL)

endif()



if(MSVC)

  target_compile_options(ParkingLot PRIVATE /W4 /permissive-)

else()

  target_compile_options(ParkingLot PRIVATE -Wall -Wextra -Wpedantic)

endif()


