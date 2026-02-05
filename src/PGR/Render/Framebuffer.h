#pragma once

#include "PGR/Base/Maths.h"
#include "PGR/Render/Texture.h"

#include <stb_image/stb_truetype.h>
#include <stb_image/stb_image_write.h>

#include <Windows.h>
#include <fstream>

namespace PGR {

    class Framebuffer {
    public:
        Framebuffer(const int width, const int height);
        Framebuffer(const Framebuffer& cpy);
        ~Framebuffer();

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }

        void SetColor(const int x, const int y, const Vec4& color);
        Vec3 GetColor(const int x, const int y) const;
        const Vec3* GetColorData() const { return m_ColorBuffer; }

        void Clear(const Vec3& color = Vec3(0.0f, 0.0f, 0.0f));
        void Clear(const Framebuffer& other);

        void LoadFontTTF(stbtt_fontinfo* fontInfo);
        void LoadFontTTF(const std::string& fontPath);

        void DrawCharTTF(int x, int y, wchar_t c, const Vec4& color, float fontSize);
        void DrawTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize, float xOffset = 0.0f, float yOffset = 0.0f);
        void DrawRotatedTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize, float rotation);
        void GetTextSize(const std::string& text, float fontSize, int* width, int* height, bool draw = false, bool baseLine = true);
        Texture* TextToTexture(const std::string& text, const Vec4& color, float fontSize);

        void DrawLine(int x0, int y0, int x1, int y1, float w, const Vec4& color);
        void FillRect(int x0, int y0, int x1, int y1, const Vec4& color);
        void FillSizeRect(int x, int y, int w, int h, const Vec4& color);

        void DrawTexture(int x, int y, const Texture* texture, int w = -1.0f, int h = -1.0f, float rotation = 0.0f, const float alpha = 1.0f);

        void ToPNG(const std::string& path);

        static Framebuffer* Create(const int width, const int height);

        stbtt_fontinfo* GetFontInfo() { return m_FontInfo; }
        stbtt_fontinfo* GetDFontInfo() { return m_DefaultFontInfo; }

    private:
        int GetPixelIndex(const int x, const int y) const { return (y * m_Width + x) * 3; }

    private:
        int m_Width;
        int m_Height;
        int m_PixelSize;
        Vec3* m_ColorBuffer;

        stbtt_fontinfo* m_FontInfo = nullptr;
        stbtt_fontinfo* m_DefaultFontInfo = nullptr;
        std::vector<unsigned char> m_FontBuffer;
        std::vector<unsigned char> m_DefaultFontBuffer;

    };

}
