#include "Application.h"
#include <immintrin.h>

#pragma warning(disable:6386)
#pragma warning(disable:4996)

namespace PGR {

    void Application::RenderBack(Framebuffer* fb) {
        fb->Clear();
        fb->DrawTexture(0, 0, m_Info.chart.imageBlur, m_Width, m_Height);

        fb->FillRect(
            (int)(m_Width / 2.0f - m_Width / 2.0 * m_Info.size + 0.5f),
            (int)(m_Height / 2.0f - m_Height / 2.0f * m_Info.size + 0.5f),
            (int)(m_Width / 2.0f + m_Width / 2.0f * m_Info.size + 0.5f),
            (int)(m_Height / 2.0f + m_Height / 2.0f * m_Info.size + 0.5f),
            Vec4(0.0f, 0.0f, 0.0f, 0.2f));

        fb->FillRect(
            0, 0, m_Width, m_Height, Vec4(0.0f, 0.0f, 0.0f, 0.3f));
    }

    void Application::RenderVideo() {
        ToDir(m_Info.ChartDir);

        if (m_Info.endTime == -1.0f) m_Info.endTime = m_Info.chart.data.time;
        float time = m_Info.endTime - m_Info.startTime;
        float currentFPS = 0.0f;
        int CPUNum = m_Info.CPUNum;
        int frameNum = (int)(time * m_Info.FPS);
        int framesPerThread = frameNum / CPUNum;
        int n = 0;
        int curS = 0;

        std::atomic<int> fn = 0;
        std::atomic<int> lastFn = 0;
        std::vector<int> threadStart(CPUNum);
        std::vector<int> threadEnd(CPUNum);
        std::vector<std::thread> threads;
        std::vector<std::string> tempVideoFiles(CPUNum);
        std::vector<Framebuffer*> fbs(CPUNum);
        std::mutex tempVFilesMutex;
        std::string str;
        Framebuffer back = Framebuffer(m_Width, m_Height);
        stbtt_fontinfo* FontInfo;
        auto lastTime = std::chrono::steady_clock::now();
        start = std::chrono::steady_clock::now();
        FontInfo = m_Framebuffer->GetFontInfo();

        for (int i = 0; i < CPUNum; i++) {
            fbs[i] = Framebuffer::Create(m_Width, m_Height);
            fbs[i]->LoadFontTTF(FontInfo);
            fbs[i]->Clear();
        }

        for (int i = 0; i < CPUNum; i++) {
            int curE = curS + framesPerThread + (i < frameNum% CPUNum ? 1 : 0);
            curE = (int)Min((float)curE, (float)frameNum);
            threadStart[i] = curS;
            threadEnd[i] = curE;
            curS = curE;
        }

        back.LoadFontTTF(FontInfo);
        back.Clear();
        RenderBack(&back);

        for (int i = 0; i < CPUNum; i++) {
            if (Overwrite("temp_video_" + std::to_string(i) + ".mp4"))
                break;
        }

        for (int i = 0; i < CPUNum; i++) {
            threads.emplace_back([this, i, &fbs, &fn, &lastFn, &lastTime, &currentFPS, frameNum, &threadStart, &threadEnd, &tempVideoFiles, &tempVFilesMutex, back, time]()
                {
                    const int startFrame = threadStart[i];
                    const int endFrame = threadEnd[i];

                    char tempVideoFile[64];
                    sprintf_s(tempVideoFile, "temp_video_%d.mp4", i);

                    const bool useAA = m_Info.aas > 1;
                    const size_t srcWidth = m_Width;
                    const size_t srcHeight = m_Height;
                    const size_t dstWidth = useAA ? static_cast<size_t>(m_Width / m_Info.aas) : m_Width;
                    const size_t dstHeight = useAA ? static_cast<size_t>(m_Height / m_Info.aas) : m_Height;
                    const size_t fps = m_Info.FPS;

                    char ffmpegCmd[512];
                    sprintf_s(ffmpegCmd, "ffmpeg -y -loglevel error -f rawvideo -pixel_format rgb24 -threads 1 -video_size %zdx%zd -framerate %zd -i - -c:v libx264 -pix_fmt yuv420p -r %zd -preset medium -b:v %fM %s",
                        dstWidth, dstHeight, fps, fps, m_Info.bitrate, tempVideoFile);

                    FILE* ffmpegPipe = _popen(ffmpegCmd, "wb");
                    const size_t dstPixelCount = dstWidth * dstHeight;
                    const size_t dstFrameSize = dstPixelCount * 3;

                    unsigned char* frameData = new unsigned char[dstFrameSize];
                    memset(frameData, 0, dstFrameSize);
                    const size_t width3 = dstWidth * 3;

                    for (int j = startFrame; j < endFrame; j++) {
                        fbs[i]->Clear(back);
                        Render((static_cast<float>(j) / static_cast<float>(fps)) + m_Info.startTime, fbs[i], false);

                        const Vec3* colorBuffer = fbs[i]->GetColorData();

                        if (useAA) {
                            const int aaScaleInt = static_cast<int>(m_Info.aas);
                            const float aaScale = m_Info.aas;
                            const int sampleArea = aaScaleInt * aaScaleInt;
                            const float invSampleArea = 1.0f / sampleArea;

                            for (size_t dy = 0; dy < dstHeight; ++dy) {
                                const int srcY = static_cast<int>(dy * aaScale);
                                const int srcYEnd = srcY + aaScaleInt;

                                unsigned char* currentRow = frameData + dy * width3;

                                for (size_t dx = 0; dx < dstWidth; ++dx) {
                                    const int srcX = static_cast<int>(dx * aaScale);
                                    const int srcXEnd = srcX + aaScaleInt;

                                    __m128 r_sum = _mm_setzero_ps();
                                    __m128 g_sum = _mm_setzero_ps();
                                    __m128 b_sum = _mm_setzero_ps();
                                    int count = 0;

                                    for (int sy = srcY; sy < srcYEnd && sy < srcHeight; ++sy) {
                                        const Vec3* srcRow = colorBuffer + sy * srcWidth;
                                        for (size_t sx = srcX; sx < srcXEnd && sx < srcWidth; sx += 4) {
                                            if (sx + 3 < srcXEnd && sx + 3 < srcWidth) {
                                                const Vec3& c0 = srcRow[sx];
                                                const Vec3& c1 = srcRow[sx + 1];
                                                const Vec3& c2 = srcRow[sx + 2];
                                                const Vec3& c3 = srcRow[sx + 3];

                                                __m128 r = _mm_set_ps(c3.X, c2.X, c1.X, c0.X);
                                                __m128 g = _mm_set_ps(c3.Y, c2.Y, c1.Y, c0.Y);
                                                __m128 b = _mm_set_ps(c3.Z, c2.Z, c1.Z, c0.Z);

                                                r_sum = _mm_add_ps(r_sum, r);
                                                g_sum = _mm_add_ps(g_sum, g);
                                                b_sum = _mm_add_ps(b_sum, b);
                                                count += 4;
                                            } else {
                                                for (int i = sx; i < srcXEnd && i < srcWidth; ++i) {
                                                    const Vec3& color = srcRow[i];
                                                    r_sum = _mm_add_ss(r_sum, _mm_set_ss(color.X));
                                                    g_sum = _mm_add_ss(g_sum, _mm_set_ss(color.Y));
                                                    b_sum = _mm_add_ss(b_sum, _mm_set_ss(color.Z));
                                                    count++;
                                                }
                                                break;
                                            }
                                        }
                                    }

                                    __m128 r_avg = _mm_mul_ps(r_sum, _mm_set1_ps(invSampleArea));
                                    __m128 g_avg = _mm_mul_ps(g_sum, _mm_set1_ps(invSampleArea));
                                    __m128 b_avg = _mm_mul_ps(b_sum, _mm_set1_ps(invSampleArea));

                                    __m128 mult = _mm_set1_ps(255.0f);
                                    r_avg = _mm_mul_ps(r_avg, mult);
                                    g_avg = _mm_mul_ps(g_avg, mult);
                                    b_avg = _mm_mul_ps(b_avg, mult);

                                    __m128i ri = _mm_cvtps_epi32(r_avg);
                                    __m128i gi = _mm_cvtps_epi32(g_avg);
                                    __m128i bi = _mm_cvtps_epi32(b_avg);

                                    size_t dstPixelOffset = dx * 3;
                                    currentRow[dstPixelOffset] = static_cast<unsigned char>(_mm_extract_epi32(ri, 0));
                                    currentRow[dstPixelOffset + 1] = static_cast<unsigned char>(_mm_extract_epi32(gi, 0));
                                    currentRow[dstPixelOffset + 2] = static_cast<unsigned char>(_mm_extract_epi32(bi, 0));
                                }
                            }
                        }
                        else {
                            const Vec3* srcRow = colorBuffer;
                            unsigned char* dstRow = frameData;

                            for (size_t y = 0; y < dstHeight; ++y) {
                                size_t x = 0;
                                for (; x + 3 < dstWidth; x += 4) {
                                    const Vec3& c0 = srcRow[x];
                                    const Vec3& c1 = srcRow[x + 1];
                                    const Vec3& c2 = srcRow[x + 2];
                                    const Vec3& c3 = srcRow[x + 3];

                                    __m128 r = _mm_set_ps(c3.X, c2.X, c1.X, c0.X);
                                    __m128 g = _mm_set_ps(c3.Y, c2.Y, c1.Y, c0.Y);
                                    __m128 b = _mm_set_ps(c3.Z, c2.Z, c1.Z, c0.Z);

                                    __m128 mult = _mm_set1_ps(255.0f);
                                    r = _mm_mul_ps(r, mult);
                                    g = _mm_mul_ps(g, mult);
                                    b = _mm_mul_ps(b, mult);

                                    __m128i ri = _mm_cvtps_epi32(r);
                                    __m128i gi = _mm_cvtps_epi32(g);
                                    __m128i bi = _mm_cvtps_epi32(b);

                                    dstRow[x * 3] = static_cast<unsigned char>(_mm_extract_epi32(ri, 0));
                                    dstRow[x * 3 + 1] = static_cast<unsigned char>(_mm_extract_epi32(gi, 0));
                                    dstRow[x * 3 + 2] = static_cast<unsigned char>(_mm_extract_epi32(bi, 0));

                                    dstRow[(x + 1) * 3] = static_cast<unsigned char>(_mm_extract_epi32(ri, 1));
                                    dstRow[(x + 1) * 3 + 1] = static_cast<unsigned char>(_mm_extract_epi32(gi, 1));
                                    dstRow[(x + 1) * 3 + 2] = static_cast<unsigned char>(_mm_extract_epi32(bi, 1));

                                    dstRow[(x + 2) * 3] = static_cast<unsigned char>(_mm_extract_epi32(ri, 2));
                                    dstRow[(x + 2) * 3 + 1] = static_cast<unsigned char>(_mm_extract_epi32(gi, 2));
                                    dstRow[(x + 2) * 3 + 2] = static_cast<unsigned char>(_mm_extract_epi32(bi, 2));

                                    dstRow[(x + 3) * 3] = static_cast<unsigned char>(_mm_extract_epi32(ri, 3));
                                    dstRow[(x + 3) * 3 + 1] = static_cast<unsigned char>(_mm_extract_epi32(gi, 3));
                                    dstRow[(x + 3) * 3 + 2] = static_cast<unsigned char>(_mm_extract_epi32(bi, 3));
                                }

                                for (; x < dstWidth; ++x) {
                                    const Vec3& color = srcRow[x];
                                    size_t dstPixelOffset = x * 3;
                                    dstRow[dstPixelOffset] = static_cast<unsigned char>(color.X * 255.0f);
                                    dstRow[dstPixelOffset + 1] = static_cast<unsigned char>(color.Y * 255.0f);
                                    dstRow[dstPixelOffset + 2] = static_cast<unsigned char>(color.Z * 255.0f);
                                }

                                srcRow += srcWidth;
                                dstRow += width3;
                            }
                        }
                        fwrite(frameData, 1u, dstFrameSize, ffmpegPipe);

                        fn.fetch_add(1, std::memory_order_relaxed);

                        auto currentTime = std::chrono::steady_clock::now();
                        static constexpr std::chrono::seconds updateInterval(1);
                        if (currentTime - lastTime >= updateInterval) {
                            int renderedFrames = fn.load(std::memory_order_relaxed) - lastFn.load(std::memory_order_relaxed);
                            currentFPS = static_cast<float>(renderedFrames) * 60.0f;
                            lastFn.store(fn.load(std::memory_order_relaxed), std::memory_order_relaxed);
                            lastTime = currentTime;
                        }
                    }

                    delete[] frameData;
                    _pclose(ffmpegPipe);

                    LogInfo("Ending thread %d", i);

                    {
                        std::lock_guard<std::mutex> lock(tempVFilesMutex);
                        tempVideoFiles[i] = tempVideoFile;
                    }

                });

        }

        threads.emplace_back([&fn, &frameNum, &currentFPS, this]() {
            const int fps = m_Info.FPS;
            const double totalFrames = static_cast<double>(frameNum);

            std::this_thread::sleep_for(std::chrono::milliseconds(166));

            while (true) {
                const int renderedFrames = fn.load(std::memory_order_relaxed);

                if (renderedFrames >= frameNum) break;

                const auto now = std::chrono::steady_clock::now();
                const float dur = std::chrono::duration_cast<std::chrono::duration<float>>(now - start).count();

                const double progress = static_cast<double>(renderedFrames) / totalFrames * 100.0;
                const double estimatedTotalTime = dur / static_cast<double>(renderedFrames) * totalFrames;
                const double remainingTime = (static_cast<double>((double)frameNum - (double)renderedFrames) / static_cast<double>(currentFPS) * 60.0) + dur;

                char buf[512];
                sprintf_s(buf, "\r%.2f%% %d.%d/%d.%d %.2fs/%.2fs(%.2fs) FPS: %d",
                    progress,
                    renderedFrames / fps, renderedFrames % fps,
                    static_cast<int>(totalFrames) / fps, static_cast<int>(totalFrames) % fps,
                    dur, estimatedTotalTime, remainingTime,
                    static_cast<int>(currentFPS) / 60
                );
                LogInfo("%s", buf);
            }

            float time = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - start).count();
            LogInfo("Rendered videos in %.2fs, total frames: %d, average FPS: %.2f", time, frameNum, static_cast<double>(frameNum) / (double)time);

            });

        for (auto& t : threads)
            t.join();

        Overwrite("input_list.txt");
        std::ofstream inputListFile("input_list.txt");
        if (inputListFile.is_open()) {
            for (const auto& file : tempVideoFiles) {
                if (!file.empty()) {
                    inputListFile << "file '" << file << "'" << std::endl;
                }
            }
            inputListFile.close();

            std::string concatCmd = "ffmpeg -y -loglevel error -f concat -safe 0 -i input_list.txt -c copy output.mp4";
            Overwrite("output.mp4");
            system(concatCmd.c_str());

            Remove("input_list.txt");
        }

        for (const auto& file : tempVideoFiles) {
            if (!file.empty()) {
                Remove(file.c_str());
            }
        }

        Overwrite("output_cut.wav");
        str = "ffmpeg -y -loglevel error -ss " + std::to_string(m_Info.startTime) + " -i " + m_Info.ChartDir + "output.wav -t " + std::to_string(m_Info.endTime) + " -c copy output_cut.wav";
        system(str.c_str());

        ToDir(m_Info.WorkDir);

        std::string filename = m_Info.OutPath + ".mp4";

        str = "ffmpeg -y -loglevel error -i " + m_Info.ChartDir + "output.mp4 -i " + m_Info.ChartDir + "output_cut.wav -c:v libx264 -pix_fmt yuv420p -preset medium -b:v " + std::to_string(m_Info.bitrate) + "M -c:a aac -strict experimental -b:a 192k -shortest " + filename;
        Overwrite(filename);
        system(str.c_str());
    }

    Texture* GetHoldTexture(const Texture* head, const int headH, const Texture* body, const int bodyH, const Texture* tail, const int tailH, const int w, const int bh = -1) {
        if (bh == -1) {
            const int h = headH + bodyH + tailH;
            Texture* tex = new Texture(w, h);

            const int headWidth = head->GetWidth();
            const int headHeight = head->GetHeight();
            const int bodyHeight = body->GetHeight();
            const int tailHeight = tail->GetHeight();

            for (int i = 0; i < w; i++) {
                const int headX = (int)((float)i / w * headWidth);

                for (int j = 0; j < headH; j++) {
                    const int headY = (int)((float)j / headH * headHeight);
                    tex->SetColor(i, j, head->GetColor(headX, headY));
                }

                for (int j = headH; j < headH + bodyH; j++) {
                    const int bodyY = (int)((float)(j - headH) / bodyH * bodyHeight);
                    tex->SetColor(i, j, body->GetColor(headX, bodyY));
                }

                for (int j = headH + bodyH; j < h; j++) {
                    const int tailY = (int)((float)(j - headH - bodyH) / tailH * tailHeight);
                    tex->SetColor(i, j, tail->GetColor(headX, tailY));
                }
            }
            return tex;
        }
        else {
            const int h = headH + bh;
            Texture* tex = new Texture(w, h);

            const int headWidth = head->GetWidth();
            const int headHeight = head->GetHeight();
            const int bodyHeight = body->GetHeight();

            for (int i = 0; i < w; i++) {
                const int headX = (int)((float)i / w * headWidth);

                for (int j = 0; j < headH; j++) {
                    const int headY = (int)((float)j / headH * headHeight);
                    tex->SetColor(i, j, head->GetColor(headX, headY));
                }

                for (int j = headH; j < h; j++) {
                    const int bodyY = (int)((float)(j - headH) / bodyH * bodyHeight);
                    tex->SetColor(i, j, body->GetColor(headX, bodyY));
                }
            }
            return tex;
        }
    }

    void Application::Render(float t, Framebuffer* fb, bool drawBack, bool cover) {

        const float PGRW_TIMES_WIDTH = PGRW * m_Width * m_Info.size;
        const float NOTE_SIZE_TIMES_SIZE = NOTE_SIZE * m_Info.size;
        const float noteW = NOTE_SIZE * m_Width * m_Info.size;
        const float size = m_Info.size;

        if (drawBack) RenderBack(fb);

        if (cover) {
            fb->DrawTexture(0, 0, m_Info.chart.imageBlur, m_Width, m_Height);

            fb->DrawTexture((int)(m_Width / 4.0f + 0.5f), (int)(m_Height * 138.0f / 1080.0f), m_Info.chart.image, (int)(m_Width / 2.0f));

            fb->DrawTextTTF(
                (int)(m_Width / 2.0f + 0.5f), (int)(m_Height * 828.0f / 1080.0f),
                m_UI.title, Vec4(PCOLOR, 1.0f), m_Height * 0.15f, 0.5f, 0.5f);
            return;
        }

        int combo = 0;
        const int numJudgeLines = (int)(m_Info.chart.data.judgeLines.size());
        const float oh = m_Width * (0.005f * size + 0.005f);

        std::vector<EventsValue> evs;
        std::vector<float> beats;
        std::vector<float> fps;
        std::vector<float> sins;
        std::vector<float> coss;

        for (size_t i = 0; i < numJudgeLines; i++) {

            JudgeLine line = m_Info.chart.data.judgeLines[i];

            EventsValue ev = line.getState(t, m_Info.chart.data.offset);
            ev.x = (ev.x * m_Width - m_Width / 2) * size + m_Width / 2;
            ev.y = m_Height - ((ev.y * m_Height - m_Height / 2) * size + m_Height / 2);
            evs.push_back(ev);

            Vec2 linePos[2] = {
                rotatePoint(ev.x, ev.y, m_Height * LINEH * size, -ev.rotate),
                rotatePoint(ev.x, ev.y, m_Height * LINEH * size, -ev.rotate + 180.0f) };

            fb->DrawLine(
                (int)(linePos[0].X + 0.5f), (int)(linePos[0].Y + 0.5f),
                (int)(linePos[1].X + 0.5f), (int)(linePos[1].Y + 0.5f),
                m_Height * LINEW * size + 0.5f,
                Vec4(PCOLOR, Max(DEBUG ? ev.alpha * 0.8f + 0.2f : ev.alpha, 0.0f)));

            const float beatt = line.sec2beat(t, m_Info.chart.data.offset);
            const float lineFp = line.getFp(beatt);
            beats.push_back(beatt);
            fps.push_back(lineFp);

            JudgeLine& currentLine = m_Info.chart.data.judgeLines[i];

            const float evRotateRad = ev.rotate * PI_OVER_180;
            const float sinEvRotate = sin(evRotateRad);
            const float cosEvRotate = cos(evRotateRad);
            sins.push_back(sinEvRotate);
            coss.push_back(cosEvRotate);

            if (DEBUG) {
                char speedBuf[32];
                sprintf(speedBuf, "%.2f", ev.speed);

                std::string lineStr =
                    "[" + std::to_string(i) +
                    "]" + speedBuf;

                float oh = m_Width * (0.004f * size + 0.0055f);
                fb->DrawRotatedTextTTF(
                    (int)(ev.x + sinEvRotate * oh + 0.5f),
                    (int)(ev.y + cosEvRotate * oh + 0.5f),
                    lineStr,
                    Vec4(PCOLOR, Max(DEBUG ? ev.alpha * 0.8f + 0.2f : ev.alpha, 0.0f)), m_Width * 0.01f, ev.rotate);
            }
        }

        const float viewFp = m_Height * 2.0f * (DEBUG ? 1 : size);

        for (int i = 0; i < numJudgeLines; i++) {
            const EventsValue& ev = evs[i];
            const float beatt = beats[i];
            const float lineFp = fps[i];
            const float sinEvRotate = sins[i];
            const float cosEvRotate = coss[i];
            JudgeLine& line = m_Info.chart.data.judgeLines[i];

            const auto& notes = line.notes;
            const size_t notesCount = notes.size();

            for (size_t j = 0; j < notesCount; j++) {
                const Note& note = notes[j];

                bool clicked = note.secTime <= t;

                if (!note.isHold) {
                    continue;
                }

                if (note.secHoldEndTime <= t) {
                    combo++;
                    continue;
                }

                float noteFp = (note.floorPosition - lineFp) * PGRH * (PGRBEAT / line.bpm) * m_Height * size;

                const bool isVisible = (DEBUG ? noteFp : noteFp) <= viewFp;
                if (!isVisible) continue;

                bool isHide = noteFp < 0 && !clicked;
                if (isHide && !DEBUG) continue;

                const bool drawHead = !clicked;
                Texture* noteHeadImg = m_Info.noteHeadImgs[note.type - 1][note.morebets];

                float thisNoteWidth = NOTE_SIZE_TIMES_SIZE;
                if (note.morebets) {
                    const float widthRatio = (float)m_Info.noteHeadImgs[note.type - 1][1]->GetWidth() /
                        (float)m_Info.noteHeadImgs[note.type - 1][0]->GetWidth();
                    thisNoteWidth *= widthRatio;
                }

                const float headImgWidth = (float)noteHeadImg->GetWidth();
                const float headImgHeight = (float)noteHeadImg->GetHeight();
                const float thisNoteHeadHeight = thisNoteWidth / headImgWidth * headImgHeight;
                const float drawHeadHeight = thisNoteWidth * m_Width / headImgWidth * headImgHeight;
                const float posX = note.positionX * PGRW_TIMES_WIDTH;

                const float noteDrawRotate = ev.rotate - (note.isAbove ? 0.0f : 180.0f);
                const float noteDrawRotateRad = noteDrawRotate * PI_OVER_180;
                const float cosDrawRad = cos(noteDrawRotateRad);
                const float sinDrawRad = sin(noteDrawRotateRad);

                const float noteAtlineX = ev.x + posX * cosEvRotate;
                const float noteAtlineY = ev.y - posX * sinEvRotate;

                float headX = noteAtlineX - noteFp * sinDrawRad;
                float headY = noteAtlineY - noteFp * cosDrawRad;

                const float texScale = thisNoteWidth * m_Width / headImgWidth;

                Texture* noteBodyImg = m_Info.holdBodyImgs[note.morebets];
                Texture* noteTailImg = m_Info.holdTailImgs[note.morebets];

                const float noteTillHeight = thisNoteWidth * m_Width / noteTailImg->GetWidth() * noteTailImg->GetHeight();
                float noteHeadHeight = thisNoteWidth * m_Width / headImgWidth * headImgHeight;

                const float holdProgress = clicked ? (1.0f - (t - note.secTime) / (note.secHoldEndTime - note.secTime)) : 1.0f;
                const float Hl = note.holdLength * size * m_Height * holdProgress;

                const float noteBodyHeight = Max(Hl, 0.0f);

                if (Hl < 0.0f)
                    if (!DEBUG)
                        continue;
                    else
                        isHide = true;

                if (note.holdLength == 0.0f)
                    if (!DEBUG) continue;
                    else isHide = true;

                if (!drawHead) {
                    noteHeadHeight = 0.0f;
                    noteFp = 0.0f;
                }
                const float noteTailFp = noteFp + noteBodyHeight;
                Texture* holdImg = GetHoldTexture(noteHeadImg, (int)noteHeadHeight, noteBodyImg, (int)noteBodyHeight, noteTailImg, (int)noteTillHeight, (int)(thisNoteWidth * m_Width), (int)(noteTailFp <= viewFp ? -1 : viewFp - noteFp));

                float drawX = 0.0f;
                float drawY = 0.0f;

                if (drawHead) {
                    drawX = headX - cosDrawRad * headImgWidth * texScale / 2.0f + drawHeadHeight * sinDrawRad - holdImg->GetHeight() * sinDrawRad;
                    drawY = headY + sinDrawRad * headImgWidth * texScale / 2.0f + drawHeadHeight * cosDrawRad - holdImg->GetHeight() * cosDrawRad;
                }
                else {
                    drawX = noteAtlineX - cosDrawRad * headImgWidth * texScale / 2.0f - holdImg->GetHeight() * sinDrawRad;
                    drawY = noteAtlineY + sinDrawRad * headImgWidth * texScale / 2.0f - holdImg->GetHeight() * cosDrawRad;
                }

                fb->DrawTexture(
                    (int)(drawX + 0.5f),
                    (int)(drawY + 0.5f),
                    holdImg,
                    -1, -1,
                    noteDrawRotate, isHide ? 0.5f : 1.0f);
                delete holdImg;
                

                if (DEBUG) {

                    EventsValue nev = line.getState(Max(t, note.secTime), m_Info.chart.data.offset);

                    char speedBuf[32];
                    if (note.speed != nev.speed && clicked)
                        sprintf(speedBuf, "%.2f : ", note.speed / nev.speed);
                    else
                        speedBuf[0] = 0;

                    std::string noteStr =
                        "[" + std::to_string(i) +
                        "]" + speedBuf +
                        std::to_string((int)(note.time + 0.5f));

                    noteStr += " / " + std::to_string((int)(note.time + note.holdTime + 0.5f));

                    const float debugX = (!drawHead ? noteAtlineX : headX) + sin(noteDrawRotateRad) * oh;
                    const float debugY = (!drawHead ? noteAtlineY : headY) + cos(noteDrawRotateRad) * oh;

                    fb->DrawRotatedTextTTF(
                        (int)(debugX + 0.5f),
                        (int)(debugY + 0.5f),
                        noteStr, Vec4(1.0f, isHide ? 0.25f : 0.5f), m_Width * 0.0066f, noteDrawRotate);
                }
            }
        }

        for (int i = 0; i < numJudgeLines; i++) {
            const EventsValue& ev = evs[i];
            const float beatt = beats[i];
            const float lineFp = fps[i];
            const float sinEvRotate = sins[i];
            const float cosEvRotate = coss[i];
            JudgeLine& line = m_Info.chart.data.judgeLines[i];

            const auto& notes = line.notes;
            const size_t notesCount = notes.size();

            for (size_t j = 0; j < notesCount; j++) {
                const Note& note = notes[j];

                if (note.isHold)
                    continue;

                if (note.secTime <= t) {
                    combo++;
                    continue;
                }

                float noteFp = (note.floorPosition - lineFp) * PGRH * (PGRBEAT / line.bpm) * m_Height * size * note.speed;

                const bool isVisible = (DEBUG ? noteFp : noteFp) <= viewFp;
                if (!isVisible) continue;

                const bool isHide = noteFp < 0 && note.secTime > t;
                if (isHide && !DEBUG) continue;

                const bool drawHead = true;
                Texture* noteHeadImg = m_Info.noteHeadImgs[note.type - 1][note.morebets];

                float thisNoteWidth = NOTE_SIZE_TIMES_SIZE;
                if (note.morebets) {
                    const float widthRatio = (float)m_Info.noteHeadImgs[note.type - 1][1]->GetWidth() /
                        (float)m_Info.noteHeadImgs[note.type - 1][0]->GetWidth();
                    thisNoteWidth *= widthRatio;
                }

                const float headImgWidth = (float)noteHeadImg->GetWidth();
                const float headImgHeight = (float)noteHeadImg->GetHeight();
                const float thisNoteHeadHeight = thisNoteWidth / headImgWidth * headImgHeight;

                const float posX = note.positionX * PGRW_TIMES_WIDTH;
                const float noteAtlineX = ev.x + posX * cosEvRotate;
                const float noteAtlineY = ev.y - posX * sinEvRotate;

                const float l2nRotate = ev.rotate - (note.isAbove ? -90.0f : 90.0f);
                const float l2nRotateRad = l2nRotate * PI_OVER_180;
                const float cosL2n = cos(l2nRotateRad);
                const float sinL2n = sin(l2nRotateRad);

                const float headX = noteAtlineX + noteFp * cosL2n;
                const float headY = noteAtlineY - noteFp * sinL2n;

                const float noteDrawRotate = ev.rotate - (note.isAbove ? 0.0f : 180.0f);
                const float noteDrawRotateRad = noteDrawRotate * PI_OVER_180;
                const float cosDrawRad = cos(noteDrawRotateRad);
                const float sinDrawRad = sin(noteDrawRotateRad);

                const float drawWidth = thisNoteWidth * m_Width / headImgWidth;
                const float offsetX = drawWidth * headImgWidth / 2.0f * cosDrawRad + drawWidth * headImgHeight / 2.0f * sinDrawRad;
                const float offsetY = drawWidth * headImgHeight / 2.0f * cosDrawRad - drawWidth * headImgWidth / 2.0f * sinDrawRad;

                const float drawX = headX - offsetX;
                const float drawY = headY - offsetY;

                if (drawHead) {
                    fb->DrawTexture(
                        (int)(drawX + 0.5f),
                        (int)(drawY + 0.5f),
                        noteHeadImg,
                        (int)(thisNoteWidth * m_Width + 0.5f),
                        -1,
                        noteDrawRotate,
                        isHide ? 0.5f : 1.0f);
                }

                if (DEBUG && drawHead) {

                    char speedBuf[32];
                    if (note.speed != 1.0f)
                        sprintf(speedBuf, "%.2f : ", note.speed);
                    else
                        speedBuf[0] = 0;

                    std::string noteStr =
                        "[" + std::to_string(i) +
                        "]" + speedBuf +
                        std::to_string((int)(note.time + 0.5f));

                    if (!note.isAbove) {
                        noteStr += " (Below)";
                    }

                    const float debugX = headX + sinDrawRad * oh;
                    const float debugY = headY + cosDrawRad * oh;

                    fb->DrawRotatedTextTTF(
                        (int)(debugX + 0.5f),
                        (int)(debugY + 0.5f),
                        noteStr, Vec4(1.0f, isHide ? 0.25f : 0.5f), m_Width * 0.0066f, noteDrawRotate);
                }
            }
        }

        float effectDur = 0.5f;
        const float pgrwTimesWidthEffect = PGRW_TIMES_WIDTH;
        const float PI_OVER_180_EFFECT = PI_OVER_180;
        const auto& hitFxImgs = m_Info.hitFxImgs;
        size_t effectCount = m_Info.chart.data.clickEffectCollection.size();
        size_t hitFxImgsCount = hitFxImgs.size();

        for (size_t effectIdx = 0; effectIdx < effectCount; effectIdx++) {
            const NoteMap& nm = m_Info.chart.data.clickEffectCollection[effectIdx];

            float secnt = m_Info.chart.data.judgeLines[nm.note.line].beat2sec(nm.time, m_Info.chart.data.offset);

            if (secnt > t)
                break;
            if (secnt + effectDur < t)
                continue;

            float p = (t - secnt) / effectDur;
            float alpha = 1.0f - p;

            size_t imgIndex = (size_t)(Max(0.0f, Min((float)(hitFxImgsCount - 1), floor(p * hitFxImgsCount))));
            Texture* img = hitFxImgs[imgIndex];
            float effectSize = noteW * 1.375f * 1.12f;
            float halfEffectSize = effectSize * 0.5f;

            JudgeLine& line = m_Info.chart.data.judgeLines[nm.note.line];
            EventsValue ev = line.getState(-nm.time, m_Info.chart.data.offset);

            ev.x *= m_Width;
            ev.y *= m_Height;
            ev.x = (ev.x - m_Width / 2) * size + m_Width / 2;
            ev.y = m_Height - ((ev.y - m_Height / 2) * size + m_Height / 2);

            float posX = ev.x;
            float posY = ev.y;
            float rotRad = ev.rotate * PI_OVER_180_EFFECT;
            float cosRot = cos(rotRad);
            float sinRot = sin(rotRad);
            float offsetX = nm.note.positionX * pgrwTimesWidthEffect;

            float finalX = posX + offsetX * cosRot;
            float finalY = posY - offsetX * sinRot;
            Vec2 pos(finalX, finalY);

            float texScale = effectSize;
            fb->DrawTexture(
                (int)(finalX - halfEffectSize + 0.5f), (int)(finalY - halfEffectSize + 0.5f), img, (int)texScale);

            for (int parIdx = 0; parIdx < 4; parIdx++) {
                const Vec3& parItem = nm.particles.pars[parIdx];

                float s = m_Width / 4040.0f * 3.0f;

                float parRad = parItem.X * PI_OVER_180_EFFECT;
                float cosParRad = cos(parRad);
                float sinParRad = sin(parRad);

                float parCenterX = parItem.Z * s * cosParRad * size;
                float parCenterY = parItem.Z * s * sinParRad * size;
                float scaledParSize = parItem.Y * p * size;

                float pScaledCenterX = parCenterX * (9.0f * p / (8.0f * p + 1.0f));
                float pScaledCenterY = parCenterY * (9.0f * p / (8.0f * p + 1.0f));
                float x1 = pScaledCenterX + finalX;
                float y1 = pScaledCenterY + finalY;

                fb->FillSizeRect((int)(x1 + 0.5f), (int)(y1 + 0.5f), (int)(parItem.Y * size * s + 0.5f), (int)(parItem.Y * size * s + 0.5f), Vec4(PCOLOR, alpha));
            }
        }

        std::vector<std::string> noteStrs;
        std::vector<float> alphas;

        if (DEBUG) {
            std::sort(m_Info.chart.data.clickCollection.begin(), m_Info.chart.data.clickCollection.end(), [](const NoteMap& a, const NoteMap& b) {
                if (a.note.secTime != b.note.secTime)
                    return a.note.secTime < b.note.secTime;
                if (a.note.positionX != b.note.positionX)
                    return a.note.positionX < b.note.positionX;
                return a.note.line < b.note.line;
                });

            int n = 0;
            for (int i = 0; i < m_Info.chart.data.clickCollection.size(); i++) {
                NoteMap& nm = m_Info.chart.data.clickCollection[i];
                JudgeLine& line = m_Info.chart.data.judgeLines[nm.note.line];

                float p = (t - nm.note.secTime) / (30.0f / Min(m_Info.chart.data.judgeLines[nm.note.line].bpm, 15 * m_Info.FPS));

                if (p < 0.0f) continue;
                if (p > 1.0f) continue;

                EventsValue ev = line.getState(-nm.time, m_Info.chart.data.offset);
                float alpha = 1.0f - p;
                ev.x *= m_Width;
                ev.y *= m_Height;
                ev.x = (ev.x - m_Width / 2) * size + m_Width / 2;
                ev.y = m_Height - ((ev.y - m_Height / 2) * size + m_Height / 2);

                float posX = ev.x;
                float posY = ev.y;
                float rotRad = ev.rotate * PI_OVER_180_EFFECT;
                float cosRot = cos(rotRad);
                float sinRot = sin(rotRad);
                float offsetX = nm.note.positionX * pgrwTimesWidthEffect;

                float finalX = posX + offsetX * cosRot;
                float finalY = posY - offsetX * sinRot;

                char PosXBuf[32];
                char speedBuf[32];

                const char* typeStr = "";

                switch (nm.note.type) {
                case 1:
                    typeStr = "Tap";
                    break;
                case 2:
                    typeStr = "Drag";
                    break;
                case 3:
                    typeStr = "Hold";
                    break;
                case 4:
                    typeStr = "Flick";
                    break;
                }

                float beatt = line.sec2beat(t, m_Info.chart.data.offset);
                float lineFp = line.getFp(beatt);
                float noteFp = (nm.note.floorPosition - lineFp) * PGRH * (PGRBEAT / line.bpm) * m_Height * size;

                EventsValue nev = line.getState(Max(t, nm.note.secTime), m_Info.chart.data.offset);

                sprintf(PosXBuf, "%.2f", nm.note.positionX);
                if (nm.note.isHold)
                    if (nev.speed != nm.note.speed)
                        sprintf(speedBuf, "%.2f : ", nm.note.speed / nev.speed);
                    else
                        speedBuf[0] = 0;
                else if (nm.note.speed != 1.0f)
                    sprintf(speedBuf, "%.2f : ", nm.note.speed);
                else
                    speedBuf[0] = 0;


                std::string noteStr =
                    "[" + std::to_string(nm.note.line) +
                    "]" + typeStr +
                    " : " + speedBuf +
                    +PosXBuf +
                    " : " + std::to_string((int)(nm.note.time + 0.5f));

                if (nm.note.isHold)
                    noteStr += " / " + std::to_string((int)(nm.note.time + nm.note.holdTime + 0.5f));

                if (!nm.note.isAbove) {
                    noteStr += " (Below)";
                }

                noteStrs.push_back(noteStr);
                alphas.push_back(alpha);

                if (alpha > 0.0f) {

                    fb->DrawTextTTF(
                        (int)(finalX + 0.5f), (int)(finalY / 1080.0f * m_Height + 0.5f),
                        std::to_string(n), Vec4(1.0f, 1.0f, 1.0f, alpha), m_Width * 0.01f, 0.5f, 0.5f
                    );

                    n++;
                }
            }

            float Ny = 76.0f / 1080.0f * m_Height;
            float NSize = noteStrs.size() <= 20 ? m_Width * 0.01f : m_Width * 0.01f / (noteStrs.size() / 20.0f);
            float No = noteStrs.size() <= 20 ? 20.0f / 1080.0f * m_Height : 20.0f / 1080.0f * m_Height / (noteStrs.size() / 20.0f);

            for (int i = 0; i < noteStrs.size(); i++) {

                fb->DrawTextTTF(
                    m_Width, (int)(Ny + 0.5f),
                    noteStrs[i],
                    Vec4(Vec3(1.0f), 0.5f * alphas[i]), NSize, 1.0f);

                Ny += No;
            }

            std::vector<std::string> LineStrs;
            char Lbuf[256];
            for (int i = (int)m_Info.chart.data.judgeLines.size() - 1; i >= 0; i--) {
                JudgeLine line = m_Info.chart.data.judgeLines[i];
                EventsValue ev = line.getState(t, m_Info.chart.data.offset);
                float sx = (ev.x - 0.5f) * size + 0.5f;
                float sy = (ev.y - 0.5f) * size + 0.5f;
                if (sx < 0.1f || sx > 0.9f || sy < 0.1f || sy > 0.9f || ev.alpha < 0.0f) {
                    sprintf(Lbuf,
                        "[%d](%.2f, %.2f) : %dd : %.2f : %.2f",
                        i, ev.x, ev.y, (int)ev.rotate, ev.alpha, ev.speed);
                    LineStrs.push_back(Lbuf);
                }
            }

            float Ly = 1000.0f / 1080.0f * m_Height;
            float LSize = LineStrs.size() <= 20 ? m_Width * 0.01f : m_Width * 0.01f / (LineStrs.size() / 20.0f);
            float Lo = LineStrs.size() <= 20 ? 20.0f / 1080.0f * m_Height : 20.0f / 1080.0f * m_Height / (LineStrs.size() / 20.0f);

            for (int i = 0; i < LineStrs.size(); i++) {
                Ly -= Lo;

                fb->DrawTextTTF(
                    0, (int)(Ly + 0.5f),
                    LineStrs[i],
                    Vec4(Vec3(1.0f), 0.5f), LSize);
            }

            std::vector<std::string> InfoStrs;
            size_t En = 0, An = 0;
            size_t EMove = 0, AMove = 0;
            size_t ERotate = 0, ARotate = 0;
            size_t EDisappear = 0, ADisappear = 0;
            size_t ESpeed = 0, ASpeed = 0;
            for (int i = 0; i < m_Info.chart.data.judgeLines.size(); i++) {
                JudgeLine line = m_Info.chart.data.judgeLines[i];
                An += line.notes.size();
                AMove += line.moveEvents.size();
                ARotate += line.rotateEvents.size();
                ADisappear += line.disappearEvents.size();
                ASpeed += line.speedEvents.size();
                En = combo;
                EMove += (size_t)findEvent(line.sec2beat(t, m_Info.chart.data.offset), line.moveEvents) + 1u;
                ERotate += (size_t)findEvent(line.sec2beat(t, m_Info.chart.data.offset), line.rotateEvents) + 1u;
                EDisappear += (size_t)findEvent(line.sec2beat(t, m_Info.chart.data.offset), line.disappearEvents) + 1u;
                ESpeed += (size_t)findEvent(line.sec2beat(t, m_Info.chart.data.offset), line.speedEvents) + 1u;
            }
            char Str[32];
            if (m_Info.chart.data.oneBPM) {
                sprintf(Str, "Offset: %.2f BPM: %.2f", m_Info.chart.data.offset, m_Info.chart.data.judgeLines[0].bpm);
            }
            else {
                sprintf(Str, "Offset: %.2f", m_Info.chart.data.offset);
            }
            InfoStrs.push_back(Str);
            InfoStrs.push_back("SpeedEvent : " + std::to_string(ESpeed) + " / " + std::to_string(ASpeed));
            InfoStrs.push_back("DisappearEvent : " + std::to_string(EDisappear) + " / " + std::to_string(ADisappear));
            InfoStrs.push_back("RotateEvent : " + std::to_string(ERotate) + " / " + std::to_string(ARotate));
            InfoStrs.push_back("MoveEvent : " + std::to_string(EMove) + " / " + std::to_string(AMove));
            InfoStrs.push_back("Note : " + std::to_string(En) + " / " + std::to_string(An));

            float Iy = 1000.0f / 1080.0f * m_Height;
            float ISize = InfoStrs.size() <= 20 ? m_Width * 0.01f : m_Width * 0.01f / (InfoStrs.size() / 20.0f);
            float Io = InfoStrs.size() <= 20 ? 20.0f / 1080.0f * m_Height : 20.0f / 1080.0f * m_Height / (InfoStrs.size() / 20.0f);

            for (int i = 0; i < InfoStrs.size(); i++) {
                Iy -= Io;

                fb->DrawTextTTF(
                    m_Width, (int)(Iy + 0.5f),
                    InfoStrs[i],
                    Vec4(Vec3(1.0f), 0.5f), ISize, 1.0f);
            }

            std::vector<SpeedEventLine> vs;

            for (size_t i = 0; i < m_Info.chart.data.judgeLines.size(); i++) {
                JudgeLine line = m_Info.chart.data.judgeLines[i];
                for (const SpeedEvent& se : line.speedEvents) {
                    float secTime = line.beat2sec(se.startTime, m_Info.chart.data.offset);
                    if (secTime + (30.0f / Min(m_Info.chart.data.judgeLines[i].bpm, 15 * m_Info.FPS)) > t && secTime <= t) {
                        vs.push_back(SpeedEventLine{ i, se.startTime, t - secTime, se.value });
                    }
                }
            }

            std::sort(vs.begin(), vs.end(), [](const SpeedEventLine& a, const SpeedEventLine& b) { return a.stime < b.stime; });

            float Sy = 76.0f / 1080.0f * m_Height;
            float SSize = vs.size() <= 20 ? m_Width * 0.01f : m_Width * 0.01f / (vs.size() / 20.0f);
            float So = vs.size() <= 20 ? 20.0f / 1080.0f * m_Height : 20.0f / 1080.0f * m_Height / (vs.size() / 20.0f);

            char Sbuf[256];
            char speedBuf[256];
            char bpmBuf[256];

            for (const SpeedEventLine& v : vs) {
                float a = (1.0f - v.durtime / (30.0f / Min(m_Info.chart.data.judgeLines[v.line].bpm, 15 * m_Info.FPS)));

                sprintf(speedBuf, "%.2f", (double)(v.v));
                sprintf(bpmBuf, "%.2f", (double)(m_Info.chart.data.judgeLines[v.line].bpm));

                sprintf(Sbuf, "[%zd]%d : %s", v.line, (int)(v.stime + 0.5f), speedBuf);

                if (!m_Info.chart.data.oneBPM && v.stime <= 0.0f) {
                    sprintf(Sbuf, "%s : %s", Sbuf, bpmBuf);
                }

                fb->DrawTextTTF(
                    0, (int)Sy,
                    Sbuf,
                    Vec4(Vec3(1.0f), a * 0.5f),
                    SSize
                );
                Sy += So;
            }

        }

        float endTime = m_Info.chart.data.time;

        int progressWidth = (int)(m_Width * (Min(t, endTime) / endTime));
        int barHeight = (int)(m_Height * 12.0f / 1080.0f);

        fb->FillRect(0, 0, progressWidth, barHeight, Vec4(0.45f, DEBUG ? 0.75f : 1.0f));
        fb->FillRect((int)(progressWidth - 0.5f), 0, (int)(progressWidth + 0.5f), barHeight, Vec4(1.0f, DEBUG ? 0.75f : 1.0f));

        int comboIndicatorX1 = (int)(m_Width * 36.0f / 1920.0f);
        int comboIndicatorX2 = (int)(m_Width * 46.0f / 1920.0f);
        int comboIndicatorX3 = (int)(m_Width * 57.0f / 1920.0f);
        int comboIndicatorX4 = (int)(m_Width * 67.0f / 1920.0f);
        int comboIndicatorY1 = (int)(m_Height * 39.0f / 1080.0f);
        int comboIndicatorY2 = (int)(m_Height * 76.0f / 1080.0f);

        fb->FillRect(comboIndicatorX1, comboIndicatorY1, comboIndicatorX2, comboIndicatorY2, Vec4(1.0f, DEBUG ? 0.75f : 1.0f));
        fb->FillRect(comboIndicatorX3, comboIndicatorY1, comboIndicatorX4, comboIndicatorY2, Vec4(1.0f, DEBUG ? 0.75f : 1.0f));

        if (combo >= 3) {
            fb->DrawTextTTF(
                (int)(m_Width * 0.5f),
                (int)(m_Height * 10.0f / 1080.0f),
                std::to_string(combo),
                Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
                (m_Width * 76.0f / 1920.0f),
                0.5f, 0.0f
            );

            fb->DrawTextTTF(
                (int)(m_Width * 0.5f),
                (int)(m_Height * 94.0f / 1080.0f),
                m_UI.combo,
                Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
                (m_Width * 26.0f / 1920.0f),
                0.5f, 0.5f
            );
        }

        float score = (float)combo / m_Info.chart.data.noteCount * 1000000.0f + 0.5f;
        if (m_Info.chart.data.noteCount == 0) score = 0.0f;
        char scoreStr[10];
        sprintf(scoreStr, "%07d", (int)score);

        fb->DrawTextTTF(
            (int)(m_Width * 1881.0f / 1920.0f),
            (int)(m_Height * 27.0f / 1080.0f),
            scoreStr,
            Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
            m_Width * 56.0f / 1920.0f,
            1.0f
        );

        int textWidth, textHeight;
        float baseFontSize = m_Width * 39.0f / 1920.0f;

        fb->GetTextSize(m_Info.chart.info.name, baseFontSize, &textWidth, &textHeight);

        int songNameX = (int)(m_Width * 45.0f / 1920.0f);
        int levelX = (int)(m_Width * 1876.0f / 1920.0f);
        int baseY = (int)(m_Height * 1010.0f / 1080.0f);

        if (textWidth <= m_Width * 0.5f) {
            fb->DrawTextTTF(
                songNameX,
                baseY,
                m_Info.chart.info.name,
                Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
                baseFontSize
            );

            fb->DrawTextTTF(
                levelX,
                baseY,
                m_Info.chart.info.level,
                Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
                baseFontSize,
                1.0f
            );
        }
        else {
            float scaledFontSize = baseFontSize * (m_Width * 0.5f / textWidth);
            int origTextWidth, origTextHeight;

            fb->GetTextSize(m_Info.chart.info.name, baseFontSize, &origTextWidth, &origTextHeight);
            fb->GetTextSize(m_Info.chart.info.level, scaledFontSize, &textWidth, &textHeight);

            float yOff = (origTextHeight - textHeight) * 0.5f;

            fb->DrawTextTTF(
                songNameX,
                (int)(baseY + yOff),
                m_Info.chart.info.name,
                Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
                scaledFontSize
            );

            fb->DrawTextTTF(
                levelX,
                (int)(baseY + yOff),
                m_Info.chart.info.level,
                Vec4(1.0f, DEBUG ? 0.75f : 1.0f),
                scaledFontSize,
                1.0f
            );
        }

        if (m_Info.chart.info.composer != "")
            fb->DrawTextTTF(
                0, (int)(m_Height * 1054.0f / 1080.0f),
                "Composer: " + m_Info.chart.info.composer, Vec4(1.0f, 0.5f), m_Width * 18.0f / 1920.0f);

        if (m_Info.chart.info.charter != "")
            fb->DrawTextTTF(
                m_Width, (int)(m_Height * 1054.0f / 1080.0f),
                "Charter: " + m_Info.chart.info.charter, Vec4(1.0f, 0.5f), m_Width * 18.0f / 1920.0f, 1.0f);

        if (m_Info.chart.info.illustrator != "")
            fb->DrawTextTTF(
                m_Width, (int)(m_Height * 13.0f / 1080.0f),
                "Illustrator: " + m_Info.chart.info.illustrator, Vec4(1.0f, 0.5f), m_Width * 18.0f / 1920.0f, 1.0f);

        char timeStr[32];
        if (!DEBUG)
            sprintf(timeStr, "%.2f%% %.2fs/%.2fs", (double)t / (double)endTime * 100.0, t, endTime);
        else {
            std::vector<size_t> BpmIndexes;
            for (size_t i = 0; i < m_Info.chart.data.judgeLines.size(); i++) {
                bool has = false;
                for (size_t j = 0; j < BpmIndexes.size(); j++) {
                    if (m_Info.chart.data.judgeLines[BpmIndexes[j]].bpm == m_Info.chart.data.judgeLines[i].bpm) {
                        has = true;
                        break;
                    }
                }
                if (!has) BpmIndexes.push_back(i);
            }
            std::string bpmStr = "";
            for (size_t i = 0; i < BpmIndexes.size(); i++) {
                char buf[32];
                if (BpmIndexes.size() == 1)
                    sprintf(buf, "%d/%d ", (int)m_Info.chart.data.judgeLines[BpmIndexes[i]].sec2beat(t, m_Info.chart.data.offset), (int)m_Info.chart.data.judgeLines[BpmIndexes[i]].sec2beat(endTime, m_Info.chart.data.offset));
                else
                    sprintf(buf, "[%.2f] %d/%d ", m_Info.chart.data.judgeLines[BpmIndexes[i]].bpm, (int)m_Info.chart.data.judgeLines[BpmIndexes[i]].sec2beat(t, m_Info.chart.data.offset), (int)m_Info.chart.data.judgeLines[BpmIndexes[i]].sec2beat(endTime, m_Info.chart.data.offset));
                bpmStr += buf;
                if (i == BpmIndexes.size() - 1)
                    bpmStr.pop_back();
            }
            sprintf(timeStr, "%.2f%% %.2fs/%.2fs (%s)", (double)t / (double)endTime * 100.0, t, endTime, bpmStr.c_str());
        }

        int w;
        fb->GetTextSize(timeStr, m_Width * 0.01f, &w, NULL);
        if (w >= m_Width * 0.5f)
            fb->DrawTextTTF(0, (int)(m_Height * 12.0f / 1080.0f), timeStr, Vec4(1.0f, 1.0f, 1.0f, 0.75f), m_Width * 0.01f / (w / (m_Width * 0.5f)));
        else
            fb->DrawTextTTF(0, (int)(m_Height * 12.0f / 1080.0f), timeStr, Vec4(1.0f, 1.0f, 1.0f, 0.75f), m_Width * 0.01f);

        fb->DrawTextTTF(
            m_Width / 2, (int)(m_Height * 1054.0f / 1080.0f), m_UI.info, Vec4(1.0f, 0.5f), m_Width * 20.0f / 1920.0f, 0.5f
        );

        if (size < 1.0f) {
            int x1 = (int)((m_Width - m_Width * size) / 2.0f);
            int y1 = (int)((m_Height - m_Height * size) / 2.0f);
            int x2 = m_Width - x1;
            int y2 = m_Height - y1;
            fb->DrawLine(x1, y1, x1, y2, 1.0f, Vec4(1.0f, 0.5f));
            fb->DrawLine(x1, y1, x2, y1, 1.0f, Vec4(1.0f, 0.5f));
            fb->DrawLine(x2, y2, x1, y2, 1.0f, Vec4(1.0f, 0.5f));
            fb->DrawLine(x2, y2, x2, y1, 1.0f, Vec4(1.0f, 0.5f));
        }

    }

}
