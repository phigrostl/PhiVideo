#include "Application.h"

namespace PGR {

    UI::UI(std::string& str) {
        cJSON* root = cJSON_Parse(str.c_str());
        cJSON* array;

        array = cJSON_GetObjectItem(root, "hitFX");
        hitFx.X = (float)cJSON_GetArrayItem(array, 0)->valueint;
        hitFx.Y = (float)cJSON_GetArrayItem(array, 1)->valueint;

        array = cJSON_GetObjectItem(root, "holdAtlas");
        holdAtlas.X = (float)cJSON_GetArrayItem(array, 0)->valueint;
        holdAtlas.Y = (float)cJSON_GetArrayItem(array, 1)->valueint;

        array = cJSON_GetObjectItem(root, "holdAtlasMH");
        holdAtlasMH.X = (float)cJSON_GetArrayItem(array, 0)->valueint;
        holdAtlasMH.Y = (float)cJSON_GetArrayItem(array, 1)->valueint;

        root = cJSON_GetObjectItem(root, "title");
        title = cJSON_GetObjectItem(root, "Title")->valuestring;
        title2 = cJSON_GetObjectItem(root, "Title2")->valuestring;
        combo = cJSON_GetObjectItem(root, "Combo")->valuestring;
        info = cJSON_GetObjectItem(root, "Info")->valuestring;

        cJSON_Delete(root);
    }

    void Application::LoadJsons() {
        std::ifstream file;

        try {
            ToDir(m_Info.ResDir);
            std::ifstream file("UI.json");
            std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
            m_UI = UI(json);
        }
        catch (std::exception e) {
            Exit("UI.json not found or format error.", 1);
        }

        try {
            ToDir(m_Info.WorkDir);
            if (m_Info.InfoPath != "") {
                std::filesystem::path path(m_Info.InfoPath);
                file.open(m_Info.InfoPath);
                if (path.is_absolute())
                    m_Info.ChartDir = GetFilePath(m_Info.InfoPath);
                else {
                    m_Info.InfoPath = "./" + m_Info.InfoPath;
                    m_Info.ChartDir = m_Info.WorkDir + GetFilePath(m_Info.InfoPath);
                }
            }
            else {
                OPENFILENAME ofn;
                char szFile[MAX_PATH] = "";

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = NULL;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
                ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrInitialDir = "chart\\";
                ofn.lpstrTitle = "Choose a ChartInfo file";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                if (GetOpenFileName(&ofn))
                    file.open(ofn.lpstrFile);
                m_Info.InfoPath = ofn.lpstrFile;
                m_Info.ChartDir = GetFilePath(m_Info.InfoPath);
            }

            if (!file.is_open())
                Exit("Please select a CORRECT ChartInfo file.", 1);
            else {
                while (!file.eof()) {
                    std::string line;
                    std::getline(file, line);
                    if (line.find("Name:") == 0)
                        m_Info.chart.info.name = line.substr(6);
                    else if (line.find("Level:") == 0)
                        m_Info.chart.info.level = line.substr(7);
                    else if (line.find("Song:") == 0)
                        m_Info.chart.info.song = line.substr(6);
                    else if (line.find("Picture:") == 0)
                        m_Info.chart.info.picture = line.substr(9);
                    else if (line.find("Chart:") == 0)
                        m_Info.chart.info.chart = line.substr(7);
                    else if (line.find("Composer: ") == 0)
                        m_Info.chart.info.composer = line.substr(10);
                    else if (line.find("Illustrator: ") == 0)
                        m_Info.chart.info.illustrator = line.substr(13);
                    else if (line.find("Charter: ") == 0)
                        m_Info.chart.info.charter = line.substr(9);
                }
                file.close();
            }

            if (m_Info.chart.info.chart == "" || m_Info.chart.info.song == "" || m_Info.chart.info.picture == "")
                throw std::exception();

            LogInfo("Name: %s\nLevel: %s\nSong: %s\nPicture: %s\nChart: %s", m_Info.chart.info.name.c_str(), m_Info.chart.info.level.c_str(), m_Info.chart.info.song.c_str(), m_Info.chart.info.picture.c_str(), m_Info.chart.info.chart.c_str());
        }
        catch (std::exception e) {
            Exit("ChartInfo file format error.", 1);
        }

        try {
            LoadChartJson();
        }
        catch (std::exception e) {
            Exit("Chart file format error.", 1);
        }
    }

    void Application::LoadChartJson() {
        cJSON* root;
        cJSON* legacyTemp;
        cJSON* lines;
        cJSON* line;
        cJSON* events;
        cJSON* event;

        ToDir(m_Info.ChartDir);
        std::ifstream file(m_Info.chart.info.chart);
        std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        root = cJSON_Parse(json.c_str());

        m_Info.chart.data.offset = (float)cJSON_GetObjectItem(root, "offset")->valuedouble;
        lines = cJSON_GetObjectItem(root, "judgeLineList");

        std::map<float, int> noteSectCounter;

        for (int i = 0; i < cJSON_GetArraySize(lines); i++) {
            JudgeLine jline;
            line = cJSON_GetArrayItem(lines, i);
            jline.bpm = (float)cJSON_GetObjectItem(line, "bpm")->valuedouble;
            events = cJSON_GetObjectItem(line, "judgeLineMoveEvents");
            for (int j = 0; j < cJSON_GetArraySize(events); j++) {
                JudgeLineMoveEvent e;
                event = cJSON_GetArrayItem(events, j);
                e.startTime = (float)cJSON_GetObjectItem(event, "startTime")->valuedouble;
                e.endTime = (float)cJSON_GetObjectItem(event, "endTime")->valuedouble;
                e.start = (float)cJSON_GetObjectItem(event, "start")->valuedouble;
                e.end = (float)cJSON_GetObjectItem(event, "end")->valuedouble;

                legacyTemp = cJSON_GetObjectItem(event, "start2");
                if (legacyTemp != NULL) {
                    e.start2 = (float)cJSON_GetObjectItem(event, "start2")->valuedouble;
                    e.end2 = (float)cJSON_GetObjectItem(event, "end2")->valuedouble;
                }
                else {
                    float t;
                    t = e.start;
                    e.start = (int)(t / 1000.0f) / 880.0f;
                    e.start2 = (int)(fmodf(t, 1000.0f)) / 530.0f;
                    t = e.end;
                    e.end = (int)(t / 1000.0f) / 880.0f;
                    e.end2 = (int)(fmodf(t, 1000.0f)) / 530.0f;
                }
                jline.moveEvents.push_back(e);
            }
            events = cJSON_GetObjectItem(line, "judgeLineRotateEvents");
            for (int j = 0; j < cJSON_GetArraySize(events); j++) {
                JudgeLineRotateEvent e;
                event = cJSON_GetArrayItem(events, j);
                e.startTime = (float)cJSON_GetObjectItem(event, "startTime")->valuedouble;
                e.endTime = (float)cJSON_GetObjectItem(event, "endTime")->valuedouble;
                e.start = (float)cJSON_GetObjectItem(event, "start")->valuedouble;
                e.end = (float)cJSON_GetObjectItem(event, "end")->valuedouble;
                jline.rotateEvents.push_back(e);
            }
            events = cJSON_GetObjectItem(line, "judgeLineDisappearEvents");
            for (int j = 0; j < cJSON_GetArraySize(events); j++) {
                JudgeLineDisappearEvent e;
                event = cJSON_GetArrayItem(events, j);
                e.startTime = (float)cJSON_GetObjectItem(event, "startTime")->valuedouble;
                e.endTime = (float)cJSON_GetObjectItem(event, "endTime")->valuedouble;
                e.start = (float)cJSON_GetObjectItem(event, "start")->valuedouble;
                e.end = (float)cJSON_GetObjectItem(event, "end")->valuedouble;
                jline.disappearEvents.push_back(e);
            }
            events = cJSON_GetObjectItem(line, "speedEvents");
            for (int j = 0; j < cJSON_GetArraySize(events); j++) {
                SpeedEvent e;
                event = cJSON_GetArrayItem(events, j);
                e.startTime = (float)cJSON_GetObjectItem(event, "startTime")->valuedouble;
                e.endTime = (float)cJSON_GetObjectItem(event, "endTime")->valuedouble;
                e.value = (float)cJSON_GetObjectItem(event, "value")->valuedouble;
                jline.speedEvents.push_back(e);
            }
            events = cJSON_GetObjectItem(line, "notesAbove");
            for (int j = 0; j < cJSON_GetArraySize(events); j++) {
                Note n;
                event = cJSON_GetArrayItem(events, j);
                n.type = (int)cJSON_GetObjectItem(event, "type")->valueint;
                n.time = (float)cJSON_GetObjectItem(event, "time")->valuedouble;
                n.floorPosition = (float)cJSON_GetObjectItem(event, "floorPosition")->valuedouble;
                n.holdTime = (float)cJSON_GetObjectItem(event, "holdTime")->valuedouble;
                n.speed = (float)cJSON_GetObjectItem(event, "speed")->valuedouble;
                n.positionX = (float)cJSON_GetObjectItem(event, "positionX")->valuedouble;
                n.isAbove = true;
                jline.notes.push_back(n);
            }
            events = cJSON_GetObjectItem(line, "notesBelow");
            for (int j = 0; j < cJSON_GetArraySize(events); j++) {
                Note n;
                event = cJSON_GetArrayItem(events, j);
                n.type = (int)cJSON_GetObjectItem(event, "type")->valueint;
                n.time = (float)cJSON_GetObjectItem(event, "time")->valuedouble;
                n.floorPosition = (float)cJSON_GetObjectItem(event, "floorPosition")->valuedouble;
                n.holdTime = (float)cJSON_GetObjectItem(event, "holdTime")->valuedouble;
                n.speed = (float)cJSON_GetObjectItem(event, "speed")->valuedouble;
                n.positionX = (float)cJSON_GetObjectItem(event, "positionX")->valuedouble;
                n.isAbove = false;
                jline.notes.push_back(n);
            }

            std::sort(jline.speedEvents.begin(), jline.speedEvents.end(), [](SpeedEvent a, SpeedEvent b)
                { return a.startTime < b.startTime; });
            for (size_t j = 0; j < jline.speedEvents.size(); j++) {
                if (jline.speedEvents[j].endTime < jline.speedEvents[j].startTime) {
                    jline.speedEvents.erase(jline.speedEvents.begin() + j);
                    j--;
                }
            }
            for (size_t i = 1; i < jline.speedEvents.size(); i++)
                jline.speedEvents[i].startTime = jline.speedEvents[i - 1u].endTime;

            std::sort(jline.moveEvents.begin(), jline.moveEvents.end(), [](JudgeLineMoveEvent a, JudgeLineMoveEvent b)
                { return a.startTime < b.startTime; });
            for (size_t j = 0; j < jline.moveEvents.size(); j++) {
                if (jline.moveEvents[j].endTime < jline.moveEvents[j].startTime) {
                    jline.moveEvents.erase(jline.moveEvents.begin() + j);
                    j--;
                }
            }
            for (size_t i = 1; i < jline.moveEvents.size(); i++)
                jline.moveEvents[i].startTime = jline.moveEvents[i - 1u].endTime;

            std::sort(jline.rotateEvents.begin(), jline.rotateEvents.end(), [](JudgeLineRotateEvent a, JudgeLineRotateEvent b)
                { return a.startTime < b.startTime; });
            for (size_t j = 0; j < jline.rotateEvents.size(); j++) {
                if (jline.rotateEvents[j].endTime < jline.rotateEvents[j].startTime) {
                    jline.rotateEvents.erase(jline.rotateEvents.begin() + j);
                    j--;
                }
            }
            for (size_t i = 1; i < jline.rotateEvents.size(); i++)
                jline.rotateEvents[i].startTime = jline.rotateEvents[i - 1u].endTime;

            std::sort(jline.disappearEvents.begin(), jline.disappearEvents.end(), [](JudgeLineDisappearEvent a, JudgeLineDisappearEvent b)
                { return a.startTime < b.startTime; });
            for (size_t j = 0; j < jline.disappearEvents.size(); j++) {
                if (jline.disappearEvents[j].endTime < jline.disappearEvents[j].startTime) {
                    jline.disappearEvents.erase(jline.disappearEvents.begin() + j);
                    j--;
                }
            }
            for (size_t i = 1; i < jline.disappearEvents.size(); i++)
                jline.disappearEvents[i].startTime = jline.disappearEvents[i - 1u].endTime;

            jline.initSpeedEvents();
            jline.initNoteFp();

            std::sort(jline.notes.begin(), jline.notes.end(), [](Note a, Note b)
                { return a.time < b.time; });

            for (auto& n : jline.notes) {
                n.secTime = jline.beat2sec(n.time, m_Info.chart.data.offset);
                n.secHoldTime = jline.beat2sec(n.holdTime, 0.0f);
                n.secHoldEndTime = n.secTime + n.secHoldTime;
                n.holdLength = n.secHoldTime * n.speed * PGRH;
                n.isHold = n.type == 3;
                n.line = (int)m_Info.chart.data.judgeLines.size();
                if (noteSectCounter.find(n.secTime) == noteSectCounter.end()) {
                    noteSectCounter[n.secTime] = 0;
                }

                noteSectCounter[n.secTime]++;
            }

            m_Info.chart.data.judgeLines.push_back(jline);
            m_Info.chart.data.noteCount += (int)jline.notes.size();

            LogInfo("Line: %d, Notes: %zd, Events: %zd", i, jline.notes.size(), jline.moveEvents.size() + jline.rotateEvents.size() + jline.disappearEvents.size() + jline.speedEvents.size());
        }

        for (long long i = 0; i < (int)m_Info.chart.data.judgeLines.size() - 1; i++) {
            if (m_Info.chart.data.judgeLines[i].bpm != m_Info.chart.data.judgeLines[i + 1llu].bpm) {
                m_Info.chart.data.oneBPM = false;
                break;
            }
        }
        if (m_Info.chart.data.judgeLines.size() == 0) m_Info.chart.data.oneBPM = false;

        for (auto& line : m_Info.chart.data.judgeLines) {
            for (auto& n : line.notes) {
                n.morebets = noteSectCounter[n.secTime] > 1;
                EventsValue ev = line.getState(n.time, m_Info.chart.data.offset);
                Vec2 pos = rotatePoint(
                    ev.x * m_Width, ev.y * m_Height,
                    n.positionX * m_Width * PGRW, ev.rotate);
                std::vector<float> seeds = {
                    0.0f,
                    n.time,
                    n.holdTime * 49999,
                    n.speed * 19997,
                    n.positionX * 12347
                };
                m_Info.chart.data.clickEffectCollection.push_back({ n, n.time, Particles((float)m_Width, (float)m_Height, seeds) });
                m_Info.chart.data.clickCollection.push_back({ n, n.time, Particles((float)m_Width, (float)m_Height, seeds) });
                if (n.isHold) {
                    float dt = 16;
                    float st = n.time;
                    while (st < n.time + n.holdTime) {
                        seeds[4] = st;
                        m_Info.chart.data.clickEffectCollection.push_back({ n, st, Particles((float)m_Width, (float)m_Height, seeds) });
                        st += dt;
                    }
                }
            }
        }

        std::sort(
            m_Info.chart.data.clickEffectCollection.begin(),
            m_Info.chart.data.clickEffectCollection.end(),
            [this](NoteMap a, NoteMap b) { return m_Info.chart.data.judgeLines[a.note.line].beat2sec(a.time, m_Info.chart.data.offset) < m_Info.chart.data.judgeLines[b.note.line].beat2sec(b.time, m_Info.chart.data.offset); }
        );

        LogInfo("Organized Notes");
    }

    void Application::LoadFiles() {
        LoadJsons();
        LoadImgs();
        LoadFxImgs();
        LoadMusics();
    }

    void Application::LoadImgs() {
        ToDir(m_Info.ResDir);
        Texture* hold = m_Info.noteImgs.hold;
        m_Info.noteImgs.holdHead = hold->ClipBlockImg(0, 0, hold->GetWidth(), (int)m_UI.holdAtlas.X);
        m_Info.noteImgs.holdBody = hold->ClipBlockImg(0, (int)m_UI.holdAtlas.X, hold->GetWidth(), hold->GetHeight() - (int)m_UI.holdAtlas.Y);
        m_Info.noteImgs.holdTail = hold->ClipBlockImg(0, hold->GetHeight() - (int)m_UI.holdAtlas.Y, hold->GetWidth(), hold->GetHeight());

        Texture* holdMH = m_Info.noteImgs.holdMH;
        m_Info.noteImgs.holdMHHead = holdMH->ClipBlockImg(0, 0, holdMH->GetWidth(), (int)m_UI.holdAtlasMH.X);
        m_Info.noteImgs.holdMHBody = holdMH->ClipBlockImg(0, (int)m_UI.holdAtlasMH.X, holdMH->GetWidth(), holdMH->GetHeight() - (int)m_UI.holdAtlasMH.Y);
        m_Info.noteImgs.holdMHTail = holdMH->ClipBlockImg(0, holdMH->GetHeight() - (int)m_UI.holdAtlasMH.Y, holdMH->GetWidth(), holdMH->GetHeight());

        m_Info.holdBodyImgs[0] = m_Info.noteImgs.holdBody;
        m_Info.holdBodyImgs[1] = m_Info.noteImgs.holdMHBody;
        m_Info.holdTailImgs[0] = m_Info.noteImgs.holdTail;
        m_Info.holdTailImgs[1] = m_Info.noteImgs.holdMHTail;

        m_Info.noteHeadImgs[0][0] = m_Info.noteImgs.click;
        m_Info.noteHeadImgs[0][1] = m_Info.noteImgs.clickMH;
        m_Info.noteHeadImgs[1][0] = m_Info.noteImgs.drag;
        m_Info.noteHeadImgs[1][1] = m_Info.noteImgs.dragMH;
        m_Info.noteHeadImgs[2][0] = m_Info.noteImgs.holdHead;
        m_Info.noteHeadImgs[2][1] = m_Info.noteImgs.holdMHHead;
        m_Info.noteHeadImgs[3][0] = m_Info.noteImgs.flick;
        m_Info.noteHeadImgs[3][1] = m_Info.noteImgs.flickMH;

        ToDir(m_Info.ChartDir);
        m_Info.chart.image = new Texture(m_Info.chart.info.picture);
        Overwrite("blurred_output.png");
        system(("ffmpeg -y -loglevel error -i " + m_Info.chart.info.picture + " -vf \"gblur = sigma = 99.0\" blurred_output.png").c_str());
        m_Info.chart.imageBlur = (new Texture("blurred_output.png"))->GetShaderImg(1.5f);

        LogInfo("Loaded Images");
    }

    void Application::LoadFxImgs() {
        const int hitFxX = (int)(m_UI.hitFx.X);
        const int hitFxY = (int)(m_UI.hitFx.Y);
        const float invHitFxX = 1.0f / m_UI.hitFx.X;
        const float invHitFxY = 1.0f / m_UI.hitFx.Y;
        const float textureWidth = (float)(m_Info.noteImgs.hitFx->GetWidth());
        const float textureHeight = (float)(m_Info.noteImgs.hitFx->GetHeight());

        m_Info.hitFxImgs.reserve((size_t)hitFxX * (size_t)hitFxY);

        for (int j = hitFxY - 1; j >= 0; j--) {
            for (int i = 0; i < hitFxX; i++) {
                const int x1 = (int)(i * invHitFxX * textureWidth);
                const int y1 = (int)(j * invHitFxY * textureHeight);
                const int x2 = (int)((i + 1) * invHitFxX * textureWidth);
                const int y2 = (int)((j + 1) * invHitFxY * textureHeight);

                m_Info.hitFxImgs.push_back(
                    m_Info.noteImgs.hitFx->ClipBlockImg(x1, y1, x2, y2)
                    ->ColorTexture(Vec4(PCOLOR, 1.0f), false));
            }
        }

        LogInfo("Loaded FxImages");
    }

    void Application::LoadMusics() {
        try {
            ToDir(m_Info.ChartDir);
            std::string music = m_Info.chart.info.song;
            std::string str = "open \"" + music + "\" alias music";
            mciSendString(str.c_str(), NULL, 0, NULL);

            char durationStr[256] = { 0 };
            MCIERROR err = mciSendStringA("status music length", durationStr, sizeof(durationStr), NULL);
            mciSendString("close music", NULL, 0, NULL);
            m_Info.chart.data.time = atoi(durationStr) / 1000.0f;

            LogInfo("Loaded Music");
        }
        catch (std::exception e) {
            Exit("Failed to load music", 1);
        }
    }

}
