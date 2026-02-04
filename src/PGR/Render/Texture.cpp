#include "Texture.h"

namespace PGR {

    Texture::Texture(const std::string& path)
        : m_Path(path) {
        Init();
    }

    Texture::Texture(const float value) {
        m_Width = 1;
        m_Height = 1;
        m_Channels = 4;
        m_Data = new Vec4[1];
        m_Data[0] = Vec4(value);
    }

    Texture::Texture(const Vec4& value) {
        m_Width = 1;
        m_Height = 1;
        m_Channels = 4;
        m_Data = new Vec4[1];
        m_Data[0] = value;
    }

    Texture::Texture(const size_t w, const size_t h) {
        m_Width = (int)w;
        m_Height = (int)h;
        m_Channels = 4;
        m_Data = new Vec4[w * h];
        memset(m_Data, 0, w * h * sizeof(Vec4));
    }

    Texture::~Texture() {
        delete[] m_Data;
        m_Data = nullptr;
    }

    void Texture::Init() {

        std::ifstream file(m_Path.c_str());
        if (!file.is_open()) {
            m_Width = 1;
            m_Height = 1;
            m_Data = new Vec4[1];
            m_Data[0] = Vec4(0.0f);
            return;
        }

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        stbi_uc* data = nullptr;
        data = stbi_load(m_Path.c_str(), &width, &height, &channels, 0);

        if (!data) {
            m_Width = 1;
            m_Height = 1;
            m_Data = new Vec4[1];
            m_Data[0] = Vec4(0.0f);
            return;
        }

        m_Height = height;
        m_Width = width;
        int size = width * height;
        m_Data = new Vec4[size];

        const float inv255 = 1.0f / 255.0f;

        switch (channels) {
        case 4:
            for (int i = 0; i < size; i++) {
                const int idx = i * 4;
                m_Data[i] = Vec4(
                    data[idx] * inv255,
                    data[idx + 1] * inv255,
                    data[idx + 2] * inv255,
                    data[idx + 3] * inv255
                );
            }
            break;

        case 3:
            for (int i = 0; i < size; i++) {
                const int idx = i * 3;
                m_Data[i] = Vec4(
                    data[idx] * inv255,
                    data[idx + 1] * inv255,
                    data[idx + 2] * inv255,
                    1.0f
                );
            }
            break;

        case 2:
            for (int i = 0; i < size; i++) {
                const int idx = i * 2;
                m_Data[i] = Vec4(
                    data[idx] * inv255,
                    data[idx + 1] * inv255,
                    0.0f,
                    0.0f
                );
            }
            break;

        case 1:
            for (int i = 0; i < size; i++) {
                m_Data[i] = Vec4(
                    data[i] * inv255,
                    0.0f,
                    0.0f,
                    0.0f
                );
            }
            break;

        default:
            break;
        }

        stbi_image_free(data);
    }


    Texture* Texture::ClipBlockImg(int x0, int y0, int x1, int y1, bool reserve) {
        if (!this || x0 < 0 || y0 < 0 || x1 <= x0 || y1 <= y0 || x1 > this->GetWidth() || y1 > this->GetHeight()) {
            return new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
        }

        Texture* newTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
        newTexture->m_Width = x1 - x0;
        newTexture->m_Height = y1 - y0;
        newTexture->m_Channels = this->m_Channels;
        newTexture->m_Path = this->GetPath() + "_blockclipped";

        int newSize = newTexture->m_Width * newTexture->m_Height;
        newTexture->m_Data = new Vec4[newSize];

        for (int y = 0; y < newTexture->m_Height; y++) {
            for (int x = 0; x < newTexture->m_Width; x++) {
                Vec4 color = this->GetColor(x + x0, y + y0);
                newTexture->SetColor(x, y, color);
            }
        }

        if (!reserve)
            delete this;

        return newTexture;
    }

    Texture* Texture::ColorTexture(Vec4 color, bool reserve) {
        Texture* newTexture = new Texture(color);
        newTexture->m_Width = this->GetWidth();
        newTexture->m_Height = this->GetHeight();
        newTexture->m_Channels = this->m_Channels;
        newTexture->m_Path = this->GetPath();

        int newSize = newTexture->m_Width * newTexture->m_Height;
        delete[] newTexture->m_Data;
        newTexture->m_Data = new Vec4[newSize];

        for (int j = 0; j < this->GetHeight(); j++) {
            for (int i = 0; i < this->GetWidth(); i++) {
                newTexture->SetColor(i, j, color * this->GetColor(i, j));
            }
        }

        if (!reserve)
            delete this;

        return newTexture;
    }

    Texture* Texture::GetShaderImg(float radius, bool reserve) {
        if (!this || m_Data == nullptr || radius <= 0.0f)
            return new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));

        int w = this->GetWidth();
        int h = this->GetHeight();
        int channels = this->m_Channels;

        Texture* newTexture = new Texture(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
        newTexture->m_Width = w;
        newTexture->m_Height = h;
        newTexture->m_Channels = channels;
        newTexture->m_Path = this->GetPath();

        int size = w * h;
        delete[] newTexture->m_Data;
        newTexture->m_Data = new Vec4[size];

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                float d = sqrt(((x - w / 2.0f) / w * 2.0f) * ((x - w / 2.0f) / w * 2.0f) + ((y - h / 2.0f) / h * 2.0f) * ((y - h / 2.0f) / h * 2.0f));
                float a = 1.0f - (d / radius) * 0.2f;
                if (a < 0.8f) a = 0.8f;
                newTexture->SetColor(x, y, this->GetColor(x, y) * a);
            }
        }
        if (!reserve)
            delete this;

        return newTexture;
    }

}
