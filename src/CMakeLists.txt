set(CMAKE_CXX_STANDARD 20)
add_subdirectory(ThreadPool)

set(SOURCE_EXE main.cpp)
add_executable(myapp ${SOURCE_EXE})

set_target_properties(myapp PROPERTIES
        CXX_STANDARD 20
        CXX_EXTENSIONS OFF)

target_link_libraries(myapp PRIVATE
        ThreadPool
)

