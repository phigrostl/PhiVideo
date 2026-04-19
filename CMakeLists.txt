cmake_minimum_required(VERSION 3.18)
project(PhiVideo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_SYSTEM_VERSION 10.0.19041.0 CACHE STRING "" FORCE)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable (PhiVideo 
    
    "src/PhiVideo/stb/stb_image.cpp"
    "src/PhiVideo/stb/stb_truetype.cpp"
    "src/PhiVideo/stb/std_image_resize2.cpp"
    "src/PhiVideo/stb/stb_image_write.cpp"

    "src/cJSON/cJSON.c"

    "src/PhiVideo/Main.cpp"

    "src/PhiVideo/Base/Base.h"
    "src/PhiVideo/Base/Base.cpp"
    "src/PhiVideo/Base/Maths.h"
    "src/PhiVideo/Base/Maths.cpp"

    "src/PhiVideo/Render/Texture.h"
    "src/PhiVideo/Render/Texture.cpp"
    "src/PhiVideo/Render/Framebuffer.h"
    "src/PhiVideo/Render/Framebuffer.cpp"

    "src/PhiVideo/Log/Log.h"
    "src/PhiVideo/Log/Log.cpp"

    "src/PhiVideo/Application/Constant.h"
    "src/PhiVideo/Application/Chart.h"
    "src/PhiVideo/Application/Note.h"
    "src/PhiVideo/Application/Note.cpp"
    "src/PhiVideo/Application/Event.h"
    "src/PhiVideo/Application/Event.cpp"
    "src/PhiVideo/Application/HitFx.h"
    "src/PhiVideo/Application/HitFx.cpp"
    "src/PhiVideo/Application/JudgeLine.h"
    
    "src/PhiVideo/Application/Application.h"
    "src/PhiVideo/Application/Application.cpp"
    "src/PhiVideo/Application/LoadFiles.cpp"
    "src/PhiVideo/Application/Mix.cpp"
    "src/PhiVideo/Application/RenderVideo.cpp"
    "src/PhiVideo/Application/RenderFrame.cpp"
)
