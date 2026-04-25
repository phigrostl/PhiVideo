#pragma once

#include "PhiVideo/Application/Chart.h"
#include "PhiVideo/Render/Framebuffer.h"
#include "PhiVideo/Log/Log.h"

#include <fstream>
#include <filesystem>
#include <mutex>
#include <thread>

#include <climits>
#include <csignal>
#include <cstdlib>

#include <direct.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <tchar.h>

#include "cJSON/cJSON.h"
#include "CLI/CLI11.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

namespace PhiVideo {

    struct UI {
        UI() = default;
        UI(std::string& str);

        Vec2 hitFx;
        Vec2 holdAtlas;
        Vec2 holdAtlasMH;

        float noteDelays[4] = { 0 };

        bool RenderJudgeLines = true;
        bool RenderNotes = true;
        bool RenderEffects = true;
        bool RenderUI = true;
        bool RenderMainInfo = true;
        bool RenderSubInfo = true;
        bool RenderDebugInfo = true;
        bool RenderBack = true;

        std::string title;
        std::string title2;
        std::string combo;
        std::string info;
        std::vector<std::string> SYM;

        int ParticleNum = 4;
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
        std::string TempDir;
        std::string InfoPath;
        std::string OutPath;

        std::string CoverTitle;

        bool RenderVideo = true;
        bool RenderCover = true;
        bool RenderPic = false;
        bool NormalChart = false;
        float PicTime = 0.0f;
        float startTime = 0.0f;
        float endTime = -1;
        float NormalY = 0.0f;
        float NormalSpeed = 2.25f;
        int GPUNum = 8;
        int CPUNum = 4;
        int FPS = 60;
        float size = 1.0f;
        float notesVolume = 0.5f;
        float musicVolume = 1.0f;
        bool overwrite = false;
        unsigned int aas = 1;
        float bitrate = 10.0f;
    };

    struct NoteSoundInfo {
        std::string soundFile;
        float delayTime = 0.0f;
    };

    class Application {
    public:
        Application(
            int argc, char** argv,
            std::string WorkDir, std::string ResDir
        );
        Application() {
            argc = 0;
            argv = nullptr;
            m_Framebuffer = nullptr;
        }

        ~Application();

        void Run();

        bool DEBUG = false;

    public:

        std::string GetDir() const {
            char dir[MAX_PATH];
            if (!_getcwd(dir, MAX_PATH)) Exit("Failed to get Working Directory", 1);
            return dir;
        }

        void ToDir(std::string dir) const {
            char Dir[MAX_PATH];
            strcpy(Dir, dir.c_str());
            if (_chdir(Dir)) Exit("Failed to change directory to %s", 1, gbk2utf8(Dir).c_str());
        }

        std::string GetFileName(const std::string path) const {
            size_t pos = path.find_last_of("/\\");
            std::string tmp = path.substr(pos + 1);
            pos = tmp.find_last_of(".");
            return tmp.substr(0, pos);
        }

        std::string GetFilePath(const std::string& path) const {
            std::string result = path;
            size_t pos = result.find_last_of("/\\");
            if (pos != std::string::npos) result = result.substr(0, pos + 1);
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

        void RenderVideo();

        void Render(float t, Framebuffer* fb, bool drawBack = true);
        void RenderCover(Framebuffer* fb) const;
        void RenderBack(Framebuffer* fb) const;

        void RenderPrepare(
            float t,
            std::vector<EventsValue>& evs, std::vector<float>& beats, std::vector<float>& fps,
            std::vector<float>& sins, std::vector<float>& coss, int& combo
        );

        void RenderJudgeLines(
            float t, Framebuffer* fb,
            std::vector<float>& beats, std::vector<EventsValue>& evs, std::vector<float>& fps,
            std::vector<float>& sins, std::vector<float>& coss
        );

        void RenderHoldNotes(
            float t, Framebuffer* fb,
            const std::vector<float>& beats,
            const std::vector<EventsValue>& evs, const std::vector<float>& fps,
            const std::vector<float>& sins, const std::vector<float>& coss, float viewFp
        );

        void RenderTapNotes(
            float t, Framebuffer* fb,
            const std::vector<float>& beats,
            const std::vector<EventsValue>& evs, const std::vector<float>& fps,
            const std::vector<float>& sins, const std::vector<float>& coss, float viewFp
        );

        void RenderEffects(float t, Framebuffer* fb, float noteW, float size);
        void RenderUI(float t, Framebuffer* fb, int combo, float size) const;
        void RenderMainInfo(float t, Framebuffer* fb) const;
        void RenderSubInfo(float t, Framebuffer* fb) const;
        void RenderDebugInfo(
            float t, int& combo, Framebuffer* fb,
            const std::vector<float>& beats,
            const std::vector<EventsValue>& evs, const std::vector<float>& fps,
            const std::vector<float>& sins, const std::vector<float>& coss,
            float viewFp, float size
        );

        void MixMusic();
        void GenerateEmptyAudio(const std::string& outputFile, float duration);
        std::vector<NoteSoundInfo> Application::CollectAllNotes();
        void ProcessNoteBatch(
            int batchIdx, const std::vector<NoteSoundInfo>& allNotes,
            std::vector<std::string>& tempAudioFiles, std::mutex& filesMutex,
            std::atomic<int>& completedBatches, int totalBatches
        );
        void MergeAudioFiles(
            const std::vector<std::string>& inputFiles, const std::string& outputFile
        );
        void MixFinalAudio(const std::string& musicFile) const;

        bool Overwritea(const std::string& path, const char* file, int line, const char* func);
        void Removea(const char* path, const char* file, int line, const char* func) const;

    private:
        int argc;
        char** argv;

        int m_Width = 1920, m_Height = 1080;
        Framebuffer* m_Framebuffer;

        UI m_UI;
        RenderInfo m_Info;
        bool m_Inited = false;

        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    };

}

#define Overwrite(path) Overwritea(path, __FILE__, __LINE__, __func__)
#define Remove(path) Removea(path, __FILE__, __LINE__, __func__)