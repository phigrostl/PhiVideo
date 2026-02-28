#pragma once

#include "PGR/Application/Chart.h"
#include "PGR/Render/Framebuffer.h"
#include "PGR/Log/Log.h"

#include <map>
#include <chrono>
#include <fstream>
#include <thread>
#include <direct.h>
#include <mmsystem.h>
#include <codecvt>
#include <locale>
#include <mutex>
#include <strsafe.h>
#include <tchar.h>
#include <filesystem>
#include <shlwapi.h>

#include "cJSON/cJSON.h"
#include "CLI/CLI11.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

namespace PGR {

    struct UI {
        UI() = default;
        UI(std::string& str);

        Vec2 hitFx;
        Vec2 holdAtlas;
        Vec2 holdAtlasMH;

        float noteDelays[4] = { 0 };

        std::string title;
        std::string title2;
        std::string combo;
        std::string info;
    };

    struct RenderInfo {
        RenderInfo() = default;

        NoteImgs noteImgs;
        std::vector<Texture*> hitFxImgs;
        Chart chart;
        Texture* noteHeadImgs[4][2] = { 0 };
        Texture* holdBodyImgs[2] = { 0 };
        Texture* holdTailImgs[2] = { 0 };

        std::string ChartDir;
        std::string WorkDir;
        std::string ResDir;
        std::string InfoPath;
        std::string OutPath;

        bool RenderVideo = true;
        bool RenderCover = true;
        bool RenderPic = false;
        float PicTime = 0.0f;
        float startTime = 0.0f;
        float endTime = -1.0f;
        int CPUNum = 4;
        int FPS = 60;
        bool showWindow = true;
        float size = 1.0f;
        float notesVolume = 0.5f;
        float musicVolume = 1.0f;
        bool overwrite = false;
        int aas = 1.0f;
        float bitrate = 10.0f;
    };

    class Application {
    public:
        Application(
            int argc, char** argv,
            std::string WorkDir, std::string ResDir
        );

        ~Application();

        void Run();

        bool DEBUG = false;

    public:

        std::string GetDir() {
            char dir[MAX_PATH];
            if (!_getcwd(dir, MAX_PATH))
                Exit("Failed to get Working Directory", 1);
            return dir;
        }

        void ToDir(const std::string& dir) {
            if (_chdir(dir.c_str())) {
                Exit("Failed to change directory to " + dir, 1);
            }
        }

        std::string GetFileName(const std::string path) {
            size_t pos = path.find_last_of("/\\");
            std::string tmp = path.substr(pos + 1);
            pos = tmp.find_last_of(".");
            return tmp.substr(0, pos);
        }

        std::string GetFilePath(const std::string& path) {
            std::string result = path;
            size_t pos = result.find_last_of("/\\");
            if (pos != std::string::npos)
                result = result.substr(0, pos + 1);
            return result;
        }

    private:
        void Init();
        void Terminate();

        void LoadFiles();
        void LoadJsons();
        void LoadChartJson();
        void LoadImgs();
        void LoadFxImgs();
        void LoadMusics();

        void Render(float t, Framebuffer* fb, bool drawBack = true, bool cover = false);
        void RenderBack(Framebuffer* fb);

        void RenderVideo();
        void MixMusic();

        bool Overwritea(const std::string& path, const char* file, int line, const char* func);
        void Removea(const char* path, const char* file, int line, const char* func);

    private:
        int argc;
        char** argv;

        int m_Width = 1920, m_Height = 1080;
        Framebuffer* m_Framebuffer;

        UI m_UI;
        RenderInfo m_Info;

        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    };

}

#define Overwrite(path) Overwritea(path, __FILE__, __LINE__, __func__)

#define Remove(path) Removea(path, __FILE__, __LINE__, __func__)
