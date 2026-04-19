#include "Framebuffer.h"

namespace PhiVideo {

    Framebuffer::Framebuffer(const int width, const int height)
        : m_Width(width), m_Height(height) {
        m_PixelSize = m_Width * m_Height;
        m_ColorBuffer = new Vec3[m_PixelSize]();
        m_FontInfo = new stbtt_fontinfo();
        m_DefaultFontInfo = new stbtt_fontinfo();
        std::ifstream file("C:\\Windows\\Fonts\\arial.ttf", std::ios::binary);
        m_DefaultFontBuffer = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        stbtt_InitFont(m_DefaultFontInfo, m_DefaultFontBuffer.data(), 0);
        Clear();
    }

    Framebuffer::Framebuffer(const Framebuffer& cpy) {
        m_Width = cpy.m_Width;
        m_Height = cpy.m_Height;
        m_PixelSize = cpy.m_PixelSize;
        m_ColorBuffer = new Vec3[m_PixelSize]();
        m_FontInfo = cpy.m_FontInfo;
        m_DefaultFontInfo = cpy.m_DefaultFontInfo;
        m_FontBuffer = cpy.m_FontBuffer;
        m_DefaultFontBuffer = cpy.m_DefaultFontBuffer;
        memcpy(m_ColorBuffer, cpy.m_ColorBuffer, sizeof(Vec3) * m_PixelSize);
    }

    Framebuffer::~Framebuffer() {
        delete[] m_ColorBuffer;
        m_ColorBuffer = nullptr;
    }


    void Framebuffer::SetColor(const int x, const int y, const Vec4& color) {
        if (x < 0 || y < 0 || x >= m_Width || y >= m_Height || color.W <= 0.0f) {
            return;
        }

        const int index = x + y * m_Width;
        Vec3& target = m_ColorBuffer[index];

        if (m_AlphaMode) {
            float srcAlpha = color.W;
            float targetAlpha = target.X;
            float resultAlpha = Clamp(srcAlpha + targetAlpha * (1.0f - srcAlpha), 0.0f, 1.0f);
            target = Vec3(resultAlpha);
        }
        else {
            Vec3 srcColor = Vec3(color);
            float alpha = color.W;
            float invAlpha = 1.0f - alpha;

            target.X = Clamp(target.X * invAlpha + srcColor.X * alpha, 0.0f, 1.0f);
            target.Y = Clamp(target.Y * invAlpha + srcColor.Y * alpha, 0.0f, 1.0f);
            target.Z = Clamp(target.Z * invAlpha + srcColor.Z * alpha, 0.0f, 1.0f);
        }
    }

    Vec3 Framebuffer::GetColor(const int x, const int y) const {
        if (x >= 0 && x < m_Width && y >= 0 && y < m_Height) {
            return m_ColorBuffer[x + y * m_Width];
        }
        return Vec3(0.0f, 0.0f, 0.0f);
    }


    void Framebuffer::Clear(const Vec3& color) {
        Vec3 clearColor = Clamp(color, 0.0f, 1.0f);
        unsigned int clearValue =
            ((unsigned char)(clearColor.Z * 255.0f) << 16) |
            ((unsigned char)(clearColor.Y * 255.0f) << 8) |
            ((unsigned char)(clearColor.X * 255.0f));

        for (int i = 0; i < m_PixelSize; i++) {
            m_ColorBuffer[i] = clearColor;
        }
    }

    void Framebuffer::Clear(const Framebuffer& other) {
        memcpy(m_ColorBuffer, other.m_ColorBuffer, sizeof(Vec3) * m_PixelSize);
    }


    void Framebuffer::LoadFontTTF(stbtt_fontinfo* fontInfo) {
        m_FontInfo = fontInfo;
    }

    void Framebuffer::LoadFontTTF(const std::string& fontPath) {
        std::ifstream file(fontPath, std::ios::binary);
        if (!file) {
            file.open("C:\\Windows\\Fonts\\" + fontPath + ".ttf", std::ios::binary);
            if (!file) {
                file.open("C:\\Users\\Administrator\\AppData\\Local\\Microsoft\\Windows\\Fonts" + fontPath + ".ttf", std::ios::binary);
                if (!file) {
                    file.open(fontPath + ".ttf", std::ios::binary);
                    if (!file) {
                        file.open("C:\\Windows\\Fonts\\arial.ttf", std::ios::binary);
                        LogError("Font file not found: %s", fontPath.c_str());
                    } else Exit("Font file not found", 0);
                }
            }
        }
        m_FontBuffer = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        stbtt_InitFont(m_FontInfo, m_FontBuffer.data(), 0);
    }


    void Framebuffer::DrawCharTTF(int x, int y, wchar_t c, const Vec4& color, float fontSize) {
        if (fontSize <= 0.0f) return;

        int glyphIndex = stbtt_FindGlyphIndex(m_FontInfo, c);
        bool charInMainFont = (glyphIndex != 0);
        stbtt_fontinfo* fontToUse = m_FontInfo;

        if (!charInMainFont && m_DefaultFontInfo != nullptr) fontToUse = m_DefaultFontInfo;

        int w, h, xoff, yoff;
        float scale = stbtt_ScaleForPixelHeight(fontToUse, fontSize);

        unsigned char* bitmap;
        bitmap = stbtt_GetCodepointBitmap(fontToUse, 0, scale, c, &w, &h, &xoff, &yoff);

        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                float alpha = (float)bitmap[i + j * w] / 255.0f;
                int px = x + i + xoff;
                int py = y + j + yoff;
                if (px >= 0 && px < m_Width && py >= 0 && py < m_Height) {
                    SetColor(px, py, Vec4(color, alpha * color.W));
                }
            }
        }
        stbtt_FreeBitmap(bitmap, nullptr);
    }

    void Framebuffer::DrawTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize, float xOffset, float yOffset) {
        std::wstring wstr = str2wstr(text);

        float scale = stbtt_ScaleForPixelHeight(m_FontInfo, fontSize);
        float DScale = stbtt_ScaleForPixelHeight(m_DefaultFontInfo, fontSize);
        int xpos = x;
        int ascent, descent, lineGap, Dascent;
        stbtt_GetFontVMetrics(m_FontInfo, &ascent, &descent, &lineGap);
        stbtt_GetFontVMetrics(m_DefaultFontInfo, &Dascent, &descent, &lineGap);
        int baseline = int(ascent * scale);
        int Dbaseline = int(Dascent * DScale);

        int w, h;
        GetTextSize(text, fontSize, &w, &h);
        int ox = (int)(xOffset * w);
        int oy = (int)(yOffset * h);

        for (wchar_t c : wstr) {
            if (c == L'\0') break;

            int glyphIndex = stbtt_FindGlyphIndex(m_FontInfo, c);
            int ax, lsb;

            if (glyphIndex == 0) {
                stbtt_GetCodepointHMetrics(m_DefaultFontInfo, c, &ax, &lsb);
                DrawCharTTF(xpos - ox, y - oy + Dbaseline, c, color, fontSize);
                int kern = stbtt_GetCodepointKernAdvance(m_DefaultFontInfo, 0, c);
                xpos += int(ax * DScale) + kern;
            } else {
                stbtt_GetCodepointHMetrics(m_FontInfo, c, &ax, &lsb);
                DrawCharTTF(xpos - ox, y - oy + baseline, c, color, fontSize);
                int kern = stbtt_GetCodepointKernAdvance(m_FontInfo, 0, c);
                xpos += int(ax * scale) + kern;
            }
        }
    }

    void Framebuffer::DrawRotatedTextTTF(int x, int y, const std::string& text, const Vec4& color, float fontSize, float rotation) {
        Texture* texture = TextToTexture(text, color, fontSize);

        int w = texture->GetWidth();
        int h = texture->GetHeight();

        float c = cos(rotation * PI_OVER_180);
        float s = sin(rotation * PI_OVER_180);

        x = (int)(x - w / 2.0f * c - h / 2.0f * s);
        y = (int)(y - h / 2.0f * c + w / 2.0f * s);

        DrawTexture(x, y, texture, -1, -1, rotation);
        delete texture;
    }

    void Framebuffer::GetTextSize(const std::string& text, float fontSize, int* width, int* height) const {
        if (fontSize <= 0.0f || text.empty()) {
            if (width) *width = 0;
            if (height) *height = 0;
            return;
        }

        float MinX = FLT_MAX, MinY = FLT_MAX, MaxX = -FLT_MAX, MaxY = -FLT_MAX;
        std::wstring wstr = str2wstr(text);
        float scale = stbtt_ScaleForPixelHeight(m_FontInfo, fontSize);
        float DScale = stbtt_ScaleForPixelHeight(m_DefaultFontInfo, fontSize);
        int xpos = 0;
        int ascent, descent, lineGap, Dascent;
        stbtt_GetFontVMetrics(m_FontInfo, &ascent, &descent, &lineGap);
        stbtt_GetFontVMetrics(m_DefaultFontInfo, &Dascent, &descent, &lineGap);
        int baseline = int(ascent * scale);
        int Dbaseline = int(Dascent * DScale);

        for (wchar_t c : wstr) {
            if (c == L'\0') break;

            int glyphIndex = stbtt_FindGlyphIndex(m_FontInfo, c);

            if (glyphIndex == 0) {
                int ax, lsb;
                stbtt_GetCodepointHMetrics(m_DefaultFontInfo, c, &ax, &lsb);

                int w, h, xoff, yoff;
                unsigned char* bitmap = stbtt_GetCodepointBitmap(m_DefaultFontInfo, 0, scale, c, &w, &h, &xoff, &yoff);

                int px = xpos + w + xoff;
                int py = h + yoff + baseline;

                if (px > MaxX) MaxX = (float)px;
                if (py > MaxY) MaxY = (float)py;
                if (px < MinX) MinX = (float)px;
                if (py < MinY) MinY = (float)py;

                px = xpos + xoff, py = yoff + baseline;
                if (px > MaxX) MaxX = (float)px;
                if (py > MaxY) MaxY = (float)py;
                if (px < MinX) MinX = (float)px;
                if (py < MinY) MinY = (float)py;

                int kern = stbtt_GetCodepointKernAdvance(m_DefaultFontInfo, 0, c);
                xpos += int(ax * DScale) + kern;

                delete[] bitmap;
            } else {
                int ax, lsb;
                stbtt_GetCodepointHMetrics(m_FontInfo, c, &ax, &lsb);

                int w, h, xoff, yoff;
                unsigned char* bitmap = stbtt_GetCodepointBitmap(m_FontInfo, 0, scale, c, &w, &h, &xoff, &yoff);

                int px = xpos + w + xoff;
                int py = h + yoff + baseline;

                if (px > MaxX) MaxX = (float)px;
                if (py > MaxY) MaxY = (float)py;
                if (px < MinX) MinX = (float)px;
                if (py < MinY) MinY = (float)py;

                px = xpos + xoff, py = yoff + baseline;
                if (px > MaxX) MaxX = (float)px;
                if (py > MaxY) MaxY = (float)py;
                if (px < MinX) MinX = (float)px;
                if (py < MinY) MinY = (float)py;

                int kern = stbtt_GetCodepointKernAdvance(m_FontInfo, 0, c);
                xpos += int(ax * scale) + kern;

                delete[] bitmap;
            }
        }

        if (MinX > 0) MinX = 0;
        if (MinY > 0) MinY = 0;

        if (width) *width = (int)(MaxX - MinX);
        if (height) *height = (int)(MaxY - MinY);
    }

    Texture* Framebuffer::TextToTexture(const std::string& text, const Vec4& color, float fontSize) {
        if (text.empty() || fontSize <= 0.0f) return new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));

        int textWidth, textHeight;
        GetTextSize(text, fontSize, &textWidth, &textHeight);

        if (textWidth < 1) textWidth = 1;
        if (textHeight < 1) textHeight = 1;

        Texture* texture = new Texture(textWidth, textHeight);

        std::wstring wstr = str2wstr(text);
        float scale = stbtt_ScaleForPixelHeight(m_FontInfo, fontSize);
        float DScale = stbtt_ScaleForPixelHeight(m_DefaultFontInfo, fontSize);
        int xpos = 0;
        int ascent, descent, lineGap, Dascent;
        stbtt_GetFontVMetrics(m_FontInfo, &ascent, &descent, &lineGap);
        stbtt_GetFontVMetrics(m_DefaultFontInfo, &Dascent, &descent, &lineGap);
        int baseline = int(ascent * scale);
        int Dbaseline = int(Dascent * DScale);

        for (wchar_t c : wstr) {
            if (c == L'\0') break;

            int glyphIndex = stbtt_FindGlyphIndex(m_FontInfo, c);
            stbtt_fontinfo* fontToUse = m_FontInfo;
            float currentScale = scale;
            int currentBaseline = baseline;

            if (glyphIndex == 0) {
                fontToUse = m_DefaultFontInfo;
                currentScale = DScale;
                currentBaseline = Dbaseline;
            }

            int w, h, xoff, yoff;
            unsigned char* bitmap = stbtt_GetCodepointBitmap(fontToUse, 0, currentScale, c, &w, &h, &xoff, &yoff);

            if (bitmap != nullptr) {
                for (int j = 0; j < h; j++) {
                    for (int i = 0; i < w; i++) {
                        float alpha = (float)bitmap[i + j * w] / 255.0f;
                        int px = xpos + i + xoff;
                        int py = currentBaseline + j + yoff;
                        if (px >= 0 && px < textWidth && py >= 0 && py < textHeight) {
                            Vec4 texColor(color.X, color.Y, color.Z, alpha * color.W);
                            texture->SetColor(px, texture->GetHeight() - py - 1, texColor);
                        }
                    }
                }
                stbtt_FreeBitmap(bitmap, nullptr);
            }

            int ax, lsb;
            stbtt_GetCodepointHMetrics(fontToUse, c, &ax, &lsb);
            int kern = stbtt_GetCodepointKernAdvance(fontToUse, 0, c);
            xpos += int(ax * currentScale) + kern;
        }

        return texture;
    }


    void Framebuffer::DrawLine(int x0, int y0, int x1, int y1, float w, const Vec4& color) {
        float dx = (float)(x1 - x0);
        float dy = (float)(y1 - y0);

        float length = sqrt(dx * dx + dy * dy);
        if (length == 0) return;

        float ux = dx / length;
        float uy = dy / length;

        float nx = -uy;
        float ny = ux;

        float halfWidth = w / 2.0f;
        float aaWidth = 1.0f;
        float outerRadius = halfWidth + aaWidth;

        int minX = (int)(floor(std::min<int>(x0, x1) - outerRadius));
        int maxX = (int)(ceil(std::max<int>(x0, x1) + outerRadius));
        int minY = (int)(floor(std::min<int>(y0, y1) - outerRadius));
        int maxY = (int)(ceil(std::max<int>(y0, y1) + outerRadius));

        minX = std::max<int>(minX, 0);
        maxX = std::min<int>(maxX, m_Width - 1);
        minY = std::max<int>(minY, 0);
        maxY = std::min<int>(maxY, m_Height - 1);

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                float px = (float)(x - x0) + 0.5f;
                float py = (float)(y - y0) + 0.5f;

                float t = px * ux + py * uy;

                if (t < -aaWidth || t > length + aaWidth) continue;

                float n = fabs(px * nx + py * ny);

                if (n > outerRadius) continue;

                float coverage = 1.0f;
                if (n > halfWidth) {
                    coverage = 1.0f - (n - halfWidth) / aaWidth;
                    coverage = Clamp(coverage, 0.0f, 1.0f);
                }

                if (t < 0) {
                    float dist = sqrt(px * px + py * py);
                    if (dist > outerRadius) continue;
                    if (dist > halfWidth) {
                        float endCoverage = 1.0f - (dist - halfWidth) / aaWidth;
                        coverage = Min(coverage, endCoverage);
                    }
                } else if (t > length) {
                    float ex = px - dx;
                    float ey = py - dy;
                    float dist = sqrt(ex * ex + ey * ey);
                    if (dist > outerRadius) continue;
                    if (dist > halfWidth) {
                        float endCoverage = 1.0f - (dist - halfWidth) / aaWidth;
                        coverage = Min(coverage, endCoverage);
                    }
                }

                if (coverage > 0.0f) {
                    Vec4 finalColor(color.X, color.Y, color.Z, color.W * coverage);
                    SetColor(x, y, finalColor);
                }
            }
        }
    }

    void Framebuffer::FillRect(int x0, int y0, int x1, int y1, const Vec4& color) {
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);

        for (int y = y0; y <= y1; ++y) {
            if (y < 0 || y >= m_Height) continue;
            for (int x = x0; x <= x1; ++x) {
                if (x < 0 || x >= m_Width) continue;
                SetColor(x, y, color);
            }
        }
    }

    void Framebuffer::FillSizeRect(int x, int y, int w, int h, const Vec4& color) {
        FillRect(x - w / 2, y - h / 2, x + w / 2, y + h / 2, color);
    }

    int sampleNum = 1;
    static float* subPixelOffsetsX = nullptr;
    static float* subPixelOffsetsY = nullptr;
    static int currentSampleNum = 0;

    int Framebuffer::GetSampleNum() const {
        return sampleNum;
    }

    void Framebuffer::SetSampleNum(int num) {
        sampleNum = num;
        
        if (num != currentSampleNum) {
            delete[] subPixelOffsetsX;
            delete[] subPixelOffsetsY;
            
            subPixelOffsetsX = new float[num];
            subPixelOffsetsY = new float[num];
            const float invNum = 1.0f / num;
            for (int i = 0; i < num; ++i) {
                subPixelOffsetsX[i] = (i + 0.5f) * invNum - 0.5f;
                subPixelOffsetsY[i] = (i + 0.5f) * invNum - 0.5f;
            }
            currentSampleNum = num;
        }
    }

    void Framebuffer::DrawTexture(int x, int y, const Texture* texture, int w, int h, float rotation, const float alpha) {
        if (!texture || alpha <= 0.0f) return;

        if (w == -1) w = (int)(texture->GetWidth());
        if (h == -1) h = (int)(w * texture->GetHeight() / texture->GetWidth());

        const float sx = (float)w / texture->GetWidth();
        const float sy = (float)h / texture->GetHeight();

        const int texWidth = texture->GetWidth();
        const int texHeight = texture->GetHeight();
        const float srcW = (float)texWidth;
        const float srcH = (float)texHeight;

        const float rad = -rotation * PI_OVER_180;
        float cosA = cosf(rad);
        float sinA = sinf(rad);

        if (cosA > 0.999f) cosA = 1.0f;
        else if (cosA < -0.999f) cosA = -1.0f;
        else if (cosA < 0.001f && cosA > -0.001f) cosA = 0.0f;
        if (sinA > 0.999f) sinA = 1.0f;
        else if (sinA < -0.999f) sinA = -1.0f;
        else if (sinA < 0.001f && sinA > -0.001f) sinA = 0.0f;

        float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
        int corners[4][2] = { {0, 0}, {w, 0}, {0, h}, {w, h} };
        for (int k = 0; k < 4; ++k) {
            float rx = (float)corners[k][0] * cosA - (float)corners[k][1] * sinA;
            float ry = (float)corners[k][0] * sinA + (float)corners[k][1] * cosA;
            minX = Min(minX, rx);
            minY = Min(minY, ry);
            maxX = Max(maxX, rx);
            maxY = Max(maxY, ry);
        }

        int startX = (int)(std::floor(minX - 1));
        int endX = (int)(std::ceil(maxX + 1));
        int startY = (int)(std::floor(minY - 1));
        int endY = (int)(std::ceil(maxY + 1));

        if (startX > endX || startY > endY) return;

        const int windowWidth = m_Width;
        const int windowHeight = m_Height;
        const Vec4* texData = texture->GetData();

        const float invSx = 1.0f / sx;
        const float invSy = 1.0f / sy;
        const float cosAInvSx = cosA * invSx;
        const float sinAInvSx = sinA * invSx;
        const float negSinAInvSy = -sinA * invSy;
        const float cosAInvSy = cosA * invSy;

        const int yStart = std::clamp(y + startY, 0, windowHeight - 1);
        const int yEnd = std::clamp(y + endY, 0, windowHeight - 1);
        const int xStart = std::clamp(x + startX, 0, windowWidth - 1);
        const int xEnd = std::clamp(x + endX, 0, windowWidth - 1);

        const int numSamples = Max(1, sampleNum);
        const int totalSamples = numSamples * numSamples;
        const float invTotalSamples = 1.0f / totalSamples;
        const float tyBase = texHeight - 1;

        for (int dstY = yStart; dstY <= yEnd; ++dstY) {
            const int dstYOffset = dstY - y;

            for (int dstX = xStart; dstX <= xEnd; ++dstX) {
                const int dstXOffset = dstX - x;

                float totalR = 0.0f, totalG = 0.0f, totalB = 0.0f, totalA = 0.0f;

                for (int sy = 0; sy < numSamples; ++sy) {
                    const float j = (float)dstYOffset + subPixelOffsetsY[sy];
                    const float baseTx = j * sinAInvSx;
                    const float baseTy = j * cosAInvSy;

                    for (int sx = 0; sx < numSamples; ++sx) {
                        const float i = (float)dstXOffset + subPixelOffsetsX[sx];

                        float tx = i * cosAInvSx + baseTx;
                        float ty = tyBase - (i * negSinAInvSy + baseTy);

                        if (tx >= 0.0f && tx < srcW && ty >= 0.0f && ty < srcH) {
                            int texX = (int)(tx + 0.5f);
                            int texY = (int)(ty - 0.5f);
                            
                            texX = Clamp(texX, 0, texWidth - 1);
                            texY = Clamp(texY, 0, texHeight - 1);

                            const Vec4& texColor = texData[texX + texY * texWidth];
                            const float texAlpha = texColor.W * alpha;
                            totalR += texColor.X * texAlpha;
                            totalG += texColor.Y * texAlpha;
                            totalB += texColor.Z * texAlpha;
                            totalA += texAlpha;
                        }
                    }
                }

                if (totalA > 0.0f) {
                    const float finalA = totalA * invTotalSamples;
                    const float invA = 1.0f / finalA;
                    Vec4 finalColor(
                        totalR * invTotalSamples * invA,
                        totalG * invTotalSamples * invA,
                        totalB * invTotalSamples * invA,
                        finalA
                    );
                    SetColor(dstX, dstY, finalColor);
                }
            }
        }
    }

    void Framebuffer::ToPNG(const std::string& path) {
        const size_t totalSize = (size_t)m_Width * (size_t)m_Height * 3;
        unsigned char* data = new unsigned char[totalSize];

        const int width = m_Width;
        const int height = m_Height;
        const Vec3* colorBuffer = m_ColorBuffer;

        for (int y = 0; y < height; ++y) {
            const Vec3* srcRow = &colorBuffer[y * width];
            unsigned char* dstRow = &data[y * width * 3];

            for (int x = 0; x < width; ++x) {
                const Vec3& color = srcRow[x];
                int dstIndex = x * 3;
                dstRow[dstIndex] = (unsigned char)(color.X * 255.0f);
                dstRow[dstIndex + 1] = (unsigned char)(color.Y * 255.0f);
                dstRow[dstIndex + 2] = (unsigned char)(color.Z * 255.0f);
            }
        }

        stbi_write_png(path.c_str(), m_Width, m_Height, 3, data, 0);

        delete[] data;
    }

    Framebuffer* Framebuffer::Create(const int width, const int height) {
        return new Framebuffer(width, height);
    }

}
