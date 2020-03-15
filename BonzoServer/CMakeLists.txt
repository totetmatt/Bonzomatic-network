project(BonzoServer)

cmake_minimum_required(VERSION 3.0)

set(BZC_EXE_NAME "BonzoServer")

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

set(BZC_PROJECT_SRCS
  ${CMAKE_SOURCE_DIR}/src/main.cpp
)

add_executable(${BZC_EXE_NAME} ${BZC_PROJECT_SRCS})
target_include_directories(${BZC_EXE_NAME} PUBLIC ${BZC_PROJECT_INCLUDES})
target_link_libraries(${BZC_EXE_NAME} ${BZC_PROJECT_LIBS})