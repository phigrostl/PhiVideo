#pragma once
#include "PGR/Base/Maths.h"

#include <stb_image/stb_image.h>
#include <stb_image/stb_image_resize2.h>

#include <fstream>
#include <string>

namespace PGR {

    class Texture {
    public:
        Texture(const std::string& path);
        Texture(const float value);
        Texture(const Vec4& value);
        Texture(const size_t width, const size_t height);
        ~Texture();

        Vec4 GetColor(int x, int y) const { return m_Data[x + y * m_Width]; }
        void SetColor(int x, int y, Vec4 color) { m_Data[x + y * m_Width] = color; }

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        std::string GetPath() const { return m_Path; }
        const Vec4* GetData() const { return m_Data; }

        Texture* ClipBlockImg(int x0, int y0, int x1, int y1, bool reserve = true);
        Texture* ColorTexture(Vec4 color, bool reserve = true);
        Texture* GetShaderImg(float radius, bool reserve = true);

    private:
        void Init();

    private:
        int m_Width, m_Height, m_Channels;
        std::string m_Path;
        Vec4* m_Data;
    };

}
