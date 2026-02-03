#include "Application.h"

namespace PGR {

    Application::Application(
        int argc, char** argv,
        std::string WorkDir, std::string ResDir) : argc(argc), argv(argv) {
        m_Info.WorkDir = WorkDir;
        m_Info.ResDir = ResDir;
        puts(("Working Directory: " + m_Info.WorkDir).c_str());
        puts(("Resource Directory: " + m_Info.ResDir).c_str());
        Init();
    }

    Application::~Application() {
        Terminate();
    }

    void Application::Init() {
        CLI::App app("PhiVideo", "PhiVideo");
        app.add_option("File", m_Info.InfoPath, "The Path of the Chart file")->required(false);
        app.add_flag("-d,--debug", DEBUG, "Debug Mode");
        app.add_option("-v,--video", m_Info.RenderVideo, "Render Video");
        app.add_option("-c,--cover", m_Info.RenderCover, "Render Cover");
        app.add_option("-o,--output", m_Info.OutPath, "Output Name");
        app.add_option("-p,--picTime", m_Info.PicTime, "Render a Picture at the Time");
        app.add_option("-s, --startTime", m_Info.startTime, "Render from the Time");
        app.add_option("-e, --endTime", m_Info.endTime, "Render to the Time");
        app.add_option("-z,--zoom", m_Info.size, "Zoom")->check(CLI::PositiveNumber);
        app.add_option("-m,--musicVolume", m_Info.musicVolume, "Music Volume");
        app.add_option("-n,--notesVolume", m_Info.notesVolume, "Notes Volume");
        app.add_option("-W,--width", m_Width, "Width");
        app.add_option("-H,--height", m_Height, "Height");
        app.add_option("-a,--aas", m_Info.aas, "Anti-Aliasing Scale");
        app.add_option("--FPS", m_Info.FPS, "FPS")->check(CLI::PositiveNumber);
        app.add_option("--CPU", m_Info.CPUNum, "CPU Core Num")->check(CLI::Range(1, 24));

        try {
            app.parse(argc, argv);
        }
        catch (const CLI::ParseError& e) {
            app.exit(e);
            exit(1);
        }

        if (app.count("-p") > 0)
            m_Info.RenderPic = true;
        if (app.count("-a") > 0) {
            m_Width = (int)(m_Width * m_Info.aas);
            m_Height = (int)(m_Height * m_Info.aas);
        }
        if (app.count("-o") == 0)
            m_Info.OutPath = "Unnamed";

        LoadFiles();

        ToDir(m_Info.ResDir);
        m_Framebuffer = Framebuffer::Create(m_Width, m_Height);
        m_Framebuffer->LoadFontTTF("D:\\PhiVideo\\resources\\Font.ttf");
    }

    void Application::Terminate() {
        if (m_Framebuffer) {
            delete m_Framebuffer;
            m_Framebuffer = nullptr;
        }
        if (m_Info.chart.image) {
            delete m_Info.chart.image;
            m_Info.chart.image = nullptr;
        }
        if (m_Info.chart.imageBlur) {
            delete m_Info.chart.imageBlur;
            m_Info.chart.imageBlur = nullptr;
        }

        delete m_Info.noteImgs.click;
        delete m_Info.noteImgs.drag;
        delete m_Info.noteImgs.hold;
        delete m_Info.noteImgs.flick;
        delete m_Info.noteImgs.hitFx;
        delete m_Info.noteImgs.clickMH;
        delete m_Info.noteImgs.dragMH;
        delete m_Info.noteImgs.holdMH;
        delete m_Info.noteImgs.flickMH;
        delete m_Info.noteImgs.holdMHHead;
        delete m_Info.noteImgs.holdMHBody;
        delete m_Info.noteImgs.holdMHTail;
        delete m_Info.noteImgs.holdHead;
        delete m_Info.noteImgs.holdBody;
        delete m_Info.noteImgs.holdTail;

        for (size_t i = 0; i < m_Info.hitFxImgs.size(); i++) {
            delete m_Info.hitFxImgs[i];
        }
        m_Info.hitFxImgs.clear();
    }

    void Application::Run() {
        if (DEBUG)
            m_UI.title = m_UI.title2;

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        const int maxThreads = (int)Min((float)sysInfo.dwNumberOfProcessors, 16.0f);
        int CPUNum = (int)Max(1.0f, Min((float)m_Info.CPUNum, (float)maxThreads));

        ToDir(m_Info.ResDir);
        m_Framebuffer->Clear(Vec3(0.0f, 0.0f, 0.0f));
        m_Framebuffer->LoadFontTTF("Font.ttf");

        ToDir(m_Info.WorkDir);

        if (m_Info.RenderCover) {
            m_Framebuffer->Clear();
            Render(0.0f, m_Framebuffer, false, true);
            m_Framebuffer->ToPNG(m_Info.OutPath + ".png");
        }

        if (m_Info.RenderPic) {
            m_Framebuffer->Clear();
            Render(m_Info.PicTime, m_Framebuffer, true);
            m_Framebuffer->ToPNG(m_Info.OutPath + std::to_string(-m_Info.PicTime) + ".png");
        }

        if (m_Info.RenderVideo) {
            MixMusic();
            RenderVideo();
        }

    }

}
