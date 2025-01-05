cmake_minimum_required(VERSION 3.30)
project(MaraTON)

set(CMAKE_CXX_STANDARD 17)

function(setup_target target_name source_file)
    add_executable(${target_name} ${source_file} utils.h compressor.hpp huffman.hpp serializator.hpp dictionary.hpp)
    target_include_directories(${target_name} PRIVATE
            include include/crypto include/tdutils include/ton include/common
    )
    set_target_properties(${target_name} PROPERTIES
            LINK_FLAGS "-L${CMAKE_CURRENT_SOURCE_DIR}"
            INSTALL_RPATH ${CMAKE_CURRENT_SOURCE_DIR}
            BUILD_RPATH ${CMAKE_CURRENT_SOURCE_DIR}
    )
    target_link_libraries(${target_name} PRIVATE ton_crypto_lib)
endfunction()

setup_target(main main.cpp)
setup_target(analyse analyse.cpp)
setup_target(Cells "code-examples/01 - Cells.cpp")
setup_target(TLB "code-examples/03 - TLB.cpp")
setup_target(TLB2 "code-examples/04 - TLB 2.cpp")
setup_target(Base64 "code-examples/02 - Base64, serialization, lz4.cpp")
setup_target(BuildHuffman "build_huffman.cpp")
setup_target(BuildDict "build_dict.cpp")
