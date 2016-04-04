
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
find_package( OpenCV REQUIRED )
add_executable( codebook main.cpp codebook.cpp )
target_link_libraries( codebook ${OpenCV_LIBS} )