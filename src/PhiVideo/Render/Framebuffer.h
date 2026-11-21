#pragma once

#include "PhiVideo/Base/Maths.h"
#include "PhiVideo/Render/Texture.h"

#include <stb_image/stb_image_write.h>
#include <stb_image/stb_truetype.h>

#include <fstream>

#include <Windows.h>

namespace PhiVideo {

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

        void SetAlphaMode(bool mode) { m_AlphaMode = mode; }
        bool GetAlphaMode() const { return m_AlphaMode; }

        void Clear(const Vec3& color = Vec3(0.0f, 0.0f, 0.0f));
        void Clear(const Framebuffer& other);

        void LoadFontTTF(stbtt_fontinfo* fontInfo);
        void LoadFontTTF(const std::string& fontPath);

        void DrawCharTTF(float x, float y, wchar_t c, const Vec4& color, float fontSize);
        void DrawTextTTF(float x, float y, const std::string& text, const Vec4& color, float fontSize, float xOffset = 0.0f, float yOffset = 0.0f);
        void DrawRotatedTextTTF(float x, float y, const std::string& text, const Vec4& color, float fontSize, float rotation);
        void GetTextSize(const std::string& text, float fontSize, int* width, int* height) const;
        Texture* TextToTexture(const std::string& text, const Vec4& color, float fontSize);

        void DrawLine(float x0, float y0, float x1, float y1, float w, const Vec4& color);
        void DrawRect(Vec2 pos, Vec2 size, float rotation, Vec4 color);
        void FillRect(float x0, float y0, float x1, float y1, const Vec4& color);
        void FillSizeRect(float x, float y, float w, float h, const Vec4& color);

        int GetSampleNum() const;
        void SetSampleNum(int num);
        void DrawTexture(float x, float y, const Texture* texture, float w = -1.0f, float h = -1.0f, float rotation = 0.0f, const float alpha = 1.0f);
        void DrawTexture(float x, float y, const Texture* textures[3], float w, const float h[3], float rotation = 0.0f, const float alpha = 1.0f);

        void ToPNG(const std::string& path);

        static Framebuffer* Create(const int width, const int height);

        stbtt_fontinfo* GetFontInfo() { return m_FontInfo; }
        stbtt_fontinfo* GetDFontInfo() { return m_DefaultFontInfo; }

    private:
        int GetPixelIndex(const int x, const int y) const { return (y * m_Width + x) * 3; }

    private:
        int m_Width, m_Height;
        int m_PixelSize;
        bool m_AlphaMode;
        Vec3* m_ColorBuffer;

        stbtt_fontinfo* m_FontInfo = nullptr;
        stbtt_fontinfo* m_DefaultFontInfo = nullptr;
        std::vector<unsigned char> m_FontBuffer;
        std::vector<unsigned char> m_DefaultFontBuffer;

    };

}
