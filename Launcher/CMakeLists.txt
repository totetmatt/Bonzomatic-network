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

set(BZC_PROJECT_SRCS
  ${CMAKE_SOURCE_DIR}/src/main.cpp
  ${CMAKE_SOURCE_DIR}/src/Network.h
  ${CMAKE_SOURCE_DIR}/src/Network.cpp
)

##############################################################################
# COMMON
add_executable(${BZC_EXE_NAME} ${BZC_PROJECT_SRCS})
target_include_directories(${BZC_EXE_NAME} PUBLIC ${BZC_PROJECT_INCLUDES})
target_link_libraries(${BZC_EXE_NAME} ${BZC_PROJECT_LIBS})