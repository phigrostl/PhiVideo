cmake_minimum_required(VERSION 3.18)
project(PhiVideo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_SYSTEM_VERSION 10.0.19041.0 CACHE STRING "" FORCE)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable (PhiVideo 
    
    "src/PGR/stb/stb_image.cpp"
    "src/PGR/stb/stb_truetype.cpp"
    "src/PGR/stb/std_image_resize2.cpp"
    "src/PGR/stb/stb_image_write.cpp"

    "src/PGR/Main.cpp"

    "src/cJSON/cJSON.c"

    "src/PGR/Base/Base.cpp"
    "src/PGR/Base/Maths.cpp"
 
    "src/PGR/Render/Texture.cpp"
    "src/PGR/Render/Framebuffer.cpp"

    "src/PGR/Log/Log.cpp"

    "src/PGR/Application/Note.cpp"
    "src/PGR/Application/Event.cpp"
    "src/PGR/Application/HitFx.cpp"
    
    "src/PGR/Application/Application.cpp"
    "src/PGR/Application/LoadFiles.cpp"
    "src/PGR/Application/Mix.cpp"
    "src/PGR/Application/Render.cpp"
)
