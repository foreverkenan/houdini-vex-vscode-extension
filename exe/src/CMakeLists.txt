cmake_minimum_required( VERSION 3.6 )
project( GenFunctionsTxtFromVexZip )

add_executable(GenFunctionsTxtFromVexZip 
  main.cpp
  miniz.c
  miniz.h
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../)

set_target_properties(GenFunctionsTxtFromVexZip PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXECUTABLE_OUTPUT_PATH}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXECUTABLE_OUTPUT_PATH}
)