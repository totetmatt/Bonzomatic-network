project(Launcher)

cmake_minimum_required(VERSION 3.0)

set(BZC_EXE_NAME "Launcher")

##############################################################################
# MONGOOSE

set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/../external/mongoose)
set(MONGOOSE_SRCS
	${CMAKE_SOURCE_DIR}/../external/mongoose/mongoose.c
)
add_library(bzc_mongoose STATIC ${MONGOOSE_SRCS})

target_include_directories(bzc_mongoose PUBLIC ${CMAKE_SOURCE_DIR}/../external/mongoose)
if (MSVC)
	target_compile_options(bzc_mongoose PUBLIC "$<$<CONFIG:Release>:/MT>")
endif ()	
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_mongoose)

##############################################################################
# JSONXX
set(JSONXX_SRCS
  ${CMAKE_SOURCE_DIR}/../external/jsonxx/jsonxx.cc
)
add_library(bzc_jsonxx STATIC ${JSONXX_SRCS})
target_include_directories(bzc_jsonxx PUBLIC ${CMAKE_SOURCE_DIR}/../external/jsonxx)
if (MSVC)
  target_compile_options(bzc_jsonxx PUBLIC "$<$<CONFIG:Release>:/MT>")
endif ()
set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/../external/jsonxx)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} bzc_jsonxx)

##############################################################################
# GLFW

set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/glfw/include)
#set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2017/glfw3.lib)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2017/glfw3dll.lib)

##############################################################################
# GLEW

set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/glew/include)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/glew/lib/Release/x64/glew32.lib)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} ${CMAKE_SOURCE_DIR}/external/glew/lib/Release/x64/glew32s.lib)
set(BZC_PROJECT_LIBS ${BZC_PROJECT_LIBS} opengl32.lib)

##############################################################################
# truetype

set(BZC_PROJECT_INCLUDES ${BZC_PROJECT_INCLUDES} ${CMAKE_SOURCE_DIR}/external/stb_truetype)

##############################################################################
# SRC

set(BZC_PROJECT_SRCS
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/Network.h
  ${CMAKE_SOURCE_DIR}/src/Network.cpp
  ${CMAKE_SOURCE_DIR}/src/ControlWindow.h
  ${CMAKE_SOURCE_DIR}/src/ControlWindow.cpp
  ${CMAKE_SOURCE_DIR}/src/Instances.h
  ${CMAKE_SOURCE_DIR}/src/Instances.cpp
)

##############################################################################
# COMMON
add_executable(${BZC_EXE_NAME} ${BZC_PROJECT_SRCS})
target_include_directories(${BZC_EXE_NAME} PUBLIC ${BZC_PROJECT_INCLUDES})
target_link_libraries(${BZC_EXE_NAME} ${BZC_PROJECT_LIBS})