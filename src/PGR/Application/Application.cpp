#include "Application.h"

namespace PGR {

    Application::Application(
        int argc, char** argv,
        std::string WorkDir, std::string ResDir) : argc(argc), argv(argv) {
        m_Info.WorkDir = WorkDir;
        m_Info.ResDir = ResDir;
        Init();
    }

    Application::~Application() {
        Terminate();
    }

    LogLevel stringToLogLevel(const std::string& level_str) {
        if (level_str[0] == 'd' || level_str[0] == 'D')
            return LogLevel::Debug;
        else if (level_str[0] == 'i' || level_str[0] == 'I')
            return LogLevel::Info;
        else if (level_str[0] == 'n' || level_str[0] == 'N')
            return LogLevel::Notice;
        else if (level_str[0] == 'w' || level_str[0] == 'W')
            return LogLevel::Warning;
        else if (level_str[0] == 'e' || level_str[0] == 'E')
            return LogLevel::Error;
        else if (level_str[0] == 'f' || level_str[0] == 'F')
            return LogLevel::Fatal;
        else {
            LogNotice("Unknown log level: %s, Used Info instead", level_str.c_str());
            return LogLevel::Info;
        }
    }

    void Application::Init() {
        CLI::App app("PhiVideo", "PhiVideo");
        std::string log_level_str = "Debug";
        app.add_option("File", m_Info.InfoPath, "The Path of the Chart file")->required(false);
        app.add_flag("-d,--debug", DEBUG, "Debug Mode");
        app.add_flag("-y,--overwrite", m_Info.overwrite, "Overwrite");
        app.add_option("-v,--video", m_Info.RenderVideo, "Render Video");
        app.add_option("-c,--cover", m_Info.RenderCover, "Render Cover");
        app.add_option("-o,--output", m_Info.OutPath, "Output Name");
        app.add_option("-p,--picTime", m_Info.PicTime, "Render a Picture at the Time");
        app.add_option("-s,--startTime", m_Info.startTime, "Render from the Time");
        app.add_option("-e,--endTime", m_Info.endTime, "Render to the Time");
        app.add_option("-z,--zoom", m_Info.size, "Zoom")->check(CLI::PositiveNumber);
        app.add_option("-m,--musicVolume", m_Info.musicVolume, "Music Volume")->check(CLI::PositiveNumber);
        app.add_option("-n,--notesVolume", m_Info.notesVolume, "Notes Volume")->check(CLI::PositiveNumber);
        app.add_option("-W,--width", m_Width, "Width")->check(CLI::PositiveNumber);
        app.add_option("-H,--height", m_Height, "Height")->check(CLI::PositiveNumber);
        app.add_option("-a,--aas", m_Info.aas, "Anti-Aliasing Scale")->check(CLI::PositiveNumber);
        app.add_option("-b,--bitrate", m_Info.bitrate, "Bitrate")->check(CLI::PositiveNumber);
        app.add_option("-l,--logLevel", log_level_str, "Log level");
        app.add_option("--FPS", m_Info.FPS, "FPS")->check(CLI::PositiveNumber);
        app.add_option("--CPU", m_Info.CPUNum, "CPU Core Num")->check(CLI::Range(1, 24));

        try {
            app.parse(argc, argv);
        }
        catch (const CLI::ParseError& e) {
            std::ostringstream out;
            std::ostringstream err;
            app.exit(e, out, err);
            LogInfo("%s", out.str().c_str());
            LogFatal("%s", err.str().c_str());
            LogNotice("Exiting with code: %d", 1);
            exit(1);
        }

        setLogLevel(stringToLogLevel(log_level_str));

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
            try { Render(0.0f, m_Framebuffer, false, true); } catch (std::exception& e) { LogError("Render Cover Error: %s", e.what()); }
            Overwrite(m_Info.OutPath + ".png");
            m_Framebuffer->ToPNG(m_Info.OutPath + ".png");
        }

        if (m_Info.RenderPic) {
            m_Framebuffer->Clear();
            try { Render(m_Info.PicTime, m_Framebuffer, true); } catch (std::exception& e) { LogError("Render Picture Error: %s", e.what()); }
            Overwrite(m_Info.OutPath + std::to_string(m_Info.PicTime) + ".png");
            m_Framebuffer->ToPNG(m_Info.OutPath + std::to_string(-m_Info.PicTime) + ".png");
        }

        if (m_Info.RenderVideo) {
            try {
                MixMusic();
                RenderVideo();
            }
            catch (std::exception& e) { LogError("Render Video Error: %s", e.what()); }

            ToDir(m_Info.ChartDir);
            Remove("output.wav");
            Remove("output_cut.wav");
            Remove("output.mp4");
        }
        
        delete m_Framebuffer->GetFontInfo();
        delete m_Framebuffer->GetDFontInfo();

        ToDir(m_Info.ChartDir);
        Remove("blurred_output.png");

    }

    bool isFileOpenedByOtherProcess(const std::string& filePath) {

        HANDLE hFile = CreateFileA(
            filePath.c_str(),
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (hFile == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            if (error == ERROR_SHARING_VIOLATION || error == ERROR_LOCK_VIOLATION)
                return true;
        }

        CloseHandle(hFile);
        return false;
    }

    bool Application::Overwritea(const std::string& path, const char* file, int line, const char* func) {

        std::string utf8path = gbk2utf8(path);

        if (isFileOpenedByOtherProcess(path)) {
            log(LogLevel::Error, file, line, func, "File is opened by other process: " + utf8path);
            log(LogLevel::Error, file, line, func, "Please close the process which is using the file.");
            while (isFileOpenedByOtherProcess(path)) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        std::fstream File(path);
        if (File.is_open()) {
            File.close();
            if (m_Info.overwrite) {
                log(LogLevel::Notice, file, line, func, "Overwrite file: " + utf8path);
            }
            else {
                LogLevel l = getLogLevel();
                setLogEnd("");
                setLogLevel(LogLevel::Warning);
                log(LogLevel::Warning, file, line, func, "Do you want to overwrite file: " + gbk2utf8(GetDir()) + "\\" + utf8path + "? (Yes/No/All yes): ");
                setLogEnd();
                setLogLevel(l);
                std::string input;
                std::cin >> input;
                std::cout << "\033[1A";
                if (input == "y" || input == "Y") {
                    putchar('\n');
                    return true;
                }
                else if (input == "n" || input == "N") {
                    putchar('\n');
                    Exit("Please clear or rename the file: " + path, 1);
                    return false;
                }
                else if (input == "a" || input == "A") {
                    putchar('\n');
                    m_Info.overwrite = true;
                    return true;
                }
            }
        }
        return true;
    }

    void Application::Removea(const char* path, const char* file, int line, const char* func){
        remove(path);
        log(LogLevel::Notice, file, line, func, (std::string)"Remove file: " + path);
    }

}
