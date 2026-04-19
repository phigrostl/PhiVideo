#include "Application.h"

#include <mutex>
#include <queue>

namespace PhiVideo {

    void Application::RenderVideo() {
        ToDir(m_Info.TempDir);

        float time = m_Info.endTime - m_Info.startTime;
        float currentFPS = 0.0f;
        int CPUNum = m_Info.CPUNum;
        int frameNum = (int)(time * m_Info.FPS);
        int framesPerThread = frameNum / CPUNum;
        int curS = 0;

        std::atomic<int> frameCounter = 0;
        std::atomic<int> lastFrameCounter = 0;
        std::vector<int> threadStart(CPUNum);
        std::vector<int> threadEnd(CPUNum);
        std::vector<bool> frameDone(frameNum, false);
        std::vector<std::thread> threads;
        std::vector<std::string> tempVideoFiles(CPUNum);
        std::vector<Framebuffer*> fbs(CPUNum);
        std::mutex tempVFilesMutex;
        Framebuffer back = Framebuffer(m_Width, m_Height);
        stbtt_fontinfo* FontInfo;
        auto lastTime = std::chrono::steady_clock::now();
        start = std::chrono::steady_clock::now();
        FontInfo = m_Framebuffer->GetFontInfo();

        for (int i = 0; i < CPUNum; i++) {
            fbs[i] = Framebuffer::Create(m_Width, m_Height);
            fbs[i]->LoadFontTTF(FontInfo);
            fbs[i]->SetAlphaMode(m_Framebuffer->GetAlphaMode());
            fbs[i]->SetSampleNum(m_Framebuffer->GetSampleNum());
            fbs[i]->Clear();
        }

        for (int i = 0; i < CPUNum; i++) {
            int curE = curS + framesPerThread + (i < frameNum % CPUNum ? 1 : 0);
            curE = (int)Min((float)curE, (float)frameNum);
            threadStart[i] = curS;
            threadEnd[i] = curE;
            curS = curE;
        }

        back.LoadFontTTF(FontInfo);
        back.SetAlphaMode(m_Framebuffer->GetAlphaMode());
        back.SetSampleNum(m_Framebuffer->GetSampleNum());
        back.Clear();
        RenderBack(&back);

        std::vector<std::atomic<int>> threadFrameCounters(CPUNum);
        for (int i = 0; i < CPUNum; i++) {
            threadFrameCounters[i] = threadStart[i];
        }

        const bool useAA = m_Info.aas > 1;
        const size_t srcWidth = m_Width;
        const size_t srcHeight = m_Height;
        const size_t dstWidth = useAA ? (size_t)(m_Width / m_Info.aas) : m_Width;
        const size_t dstHeight = useAA ? (size_t)(m_Height / m_Info.aas) : m_Height;
        const size_t fps = m_Info.FPS;

        std::mutex workStealMutex;
        std::vector<FILE*> ffmpegPipes(CPUNum, nullptr);
        std::vector<std::mutex> pipeMutexes(CPUNum);

        struct FrameData {
            int frameIndex;
            std::vector<unsigned char> data;
        };

        std::vector<std::map<int, std::vector<unsigned char>>> frameBuffers(CPUNum);
        std::vector<std::atomic<int>> nextFrameToWrite(CPUNum);
        std::vector<std::mutex> bufferMutexes(CPUNum);

        for (int i = 0; i < CPUNum; i++) {
            nextFrameToWrite[i] = 0;
        }

        for (int i = 0; i < CPUNum; i++) {
            if (threadStart[i] < threadEnd[i]) {
                char tempVideoFile[64];
                sprintf_s(tempVideoFile, "temp_video_%d.mp4", i);

                char ffmpegCmd[512];
                if (i < m_Info.GPUNum) {
                    sprintf_s(
                        ffmpegCmd, "ffmpeg -y -loglevel error -f rawvideo -pixel_format rgb24"
                        " -threads %d -video_size %zdx%zd -framerate %zd -i - "
                        "-c:v h264_nvenc -pix_fmt yuv420p -r %zd -preset medium -b:v %fM %s",
                        CPUNum, dstWidth, dstHeight, fps, fps, m_Info.bitrate, tempVideoFile
                    );
                } else {
                    sprintf_s(
                        ffmpegCmd, "ffmpeg -y -loglevel error -f rawvideo -pixel_format rgb24"
                        " -threads %d -video_size %zdx%zd -framerate %zd -i - "
                        "-c:v libx264 -pix_fmt yuv420p -r %zd -preset medium -b:v %fM %s",
                        CPUNum, dstWidth, dstHeight, fps, fps, m_Info.bitrate, tempVideoFile
                    );
                }

                ffmpegPipes[i] = _popen(ffmpegCmd, "wb");
            }
        }

        for (int i = 0; i < CPUNum; i++) {
            threads.emplace_back(
                [
                    this, i, frameNum, back, CPUNum, useAA,
                    srcWidth, srcHeight, dstWidth, dstHeight, fps,
                    &fbs, &frameCounter, &lastFrameCounter, &lastTime, &currentFPS,
                    &threadStart, &threadEnd, &tempVideoFiles, &tempVFilesMutex,
                    &threadFrameCounters, &workStealMutex, &ffmpegPipes, &pipeMutexes,
                    &frameBuffers, &nextFrameToWrite, &bufferMutexes, &frameDone
                ]() {
                const size_t width3 = dstWidth * 3;

                char tempVideoFile[64];
                sprintf_s(tempVideoFile, "temp_video_%d.mp4", i);

                unsigned char* frameData = new unsigned char[dstWidth * dstHeight * 3];
                memset(frameData, 0, dstWidth * dstHeight * 3);

                auto writeFramesToPipe = [&](int threadIndex) {
                    if (ffmpegPipes[threadIndex] == nullptr) return;

                    while (true) {
                        int currentFrame = nextFrameToWrite[threadIndex].load(
                            std::memory_order_relaxed
                        );
                        int expectedStart = threadStart[threadIndex];
                        int frameIndex = expectedStart + currentFrame;

                        {
                            std::lock_guard<std::mutex> lock(bufferMutexes[threadIndex]);
                            auto it = frameBuffers[threadIndex].find(frameIndex);
                            if (it == frameBuffers[threadIndex].end()) {
                                break;
                            }

                            std::lock_guard<std::mutex> pipeLock(pipeMutexes[threadIndex]);
                            fwrite(it->second.data(), 1u, dstWidth * dstHeight * 3, ffmpegPipes[threadIndex]);
                            frameDone[frameIndex] = true;
                            frameBuffers[threadIndex].erase(it);
                            nextFrameToWrite[threadIndex].fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                };

                auto stealWork = [&]() -> std::pair<int, int> {
                    std::lock_guard<std::mutex> lock(workStealMutex);
                    for (int j = 0; j < CPUNum; j++) {
                        if (j == i) continue;
                        int current = threadFrameCounters[j].load(std::memory_order_relaxed);
                        if (current < threadEnd[j]) {
                            int next = current + 1;
                            if (
                                threadFrameCounters[j]
                                .compare_exchange_strong(
                                    current, next, std::memory_order_relaxed
                                )) {
                                return std::make_pair(current, j);
                            }
                        }
                    }
                    return std::make_pair(-1, -1);
                };

                while (true) {
                    int j = threadFrameCounters[i].load(std::memory_order_relaxed);
                    if (j < threadEnd[i]) {
                        if (threadFrameCounters[i].compare_exchange_strong(
                            j, j + 1, std::memory_order_relaxed
                        )) {
                            fbs[i]->Clear(back);
                            Render(((float)(j) / (float)(fps)) + m_Info.startTime, fbs[i], false);

                            const Vec3* colorBuffer = fbs[i]->GetColorData();

                            if (useAA) {
                                for (size_t y = 0; y < dstHeight; y++) {
                                    for (size_t x = 0; x < dstWidth; x++) {
                                        size_t r = 0, g = 0, b = 0;
                                        for (size_t j = 0; j < (size_t)(m_Info.aas); j++) {
                                            for (size_t i = 0; i < (size_t)(m_Info.aas); i++) {
                                                const Vec3& color = colorBuffer[((y * (size_t)(m_Info.aas) + j) * srcWidth + (x * (size_t)(m_Info.aas) + i))];
                                                r += (size_t)(color.X * 255.0f);
                                                g += (size_t)(color.Y * 255.0f);
                                                b += (size_t)(color.Z * 255.0f);
                                            }
                                        }
                                        frameData[y * width3 + x * 3 + 0] = (unsigned char)(r / ((unsigned long long)m_Info.aas * m_Info.aas));
                                        frameData[y * width3 + x * 3 + 1] = (unsigned char)(g / ((unsigned long long)m_Info.aas * m_Info.aas));
                                        frameData[y * width3 + x * 3 + 2] = (unsigned char)(b / ((unsigned long long)m_Info.aas * m_Info.aas));
                                    }
                                }
                            } else {
                                for (size_t y = 0; y < dstHeight; y++) {
                                    for (size_t x = 0; x < dstWidth; x++) {
                                        const Vec3& color = colorBuffer[y * srcWidth + x];
                                        int index = y * width3 + x * 3;
                                        frameData[index + 0] = (unsigned char)(color.X * 255.0f);
                                        frameData[index + 1] = (unsigned char)(color.Y * 255.0f);
                                        frameData[index + 2] = (unsigned char)(color.Z * 255.0f);
                                    }
                                }
                            }

                            {
                                std::lock_guard<std::mutex> lock(bufferMutexes[i]);
                                std::vector<unsigned char> data(frameData, frameData + dstWidth * dstHeight * 3);
                                frameBuffers[i][j] = std::move(data);
                            }

                            writeFramesToPipe(i);

                            frameCounter.fetch_add(1, std::memory_order_relaxed);

                            auto currentTime = std::chrono::steady_clock::now();
                            static constexpr std::chrono::seconds updateInterval(1);
                            if (currentTime - lastTime >= updateInterval) {
                                int renderedFrames = frameCounter.load(std::memory_order_relaxed) - lastFrameCounter.load(std::memory_order_relaxed);
                                currentFPS = (float)(renderedFrames) * 60.0f;
                                lastFrameCounter.store(frameCounter.load(std::memory_order_relaxed), std::memory_order_relaxed);
                                lastTime = currentTime;
                            }
                        }
                    } else {
                        auto [stolenFrame, targetThread] = stealWork();
                        if (stolenFrame == -1) {
                            break;
                        }
                        fbs[i]->Clear(back);
                        Render(((float)(stolenFrame) / (float)(fps)) + m_Info.startTime, fbs[i], false);

                        const Vec3* colorBuffer = fbs[i]->GetColorData();

                        if (useAA) {
                            for (size_t y = 0; y < dstHeight; y++) {
                                for (size_t x = 0; x < dstWidth; x++) {
                                    size_t r = 0, g = 0, b = 0;
                                    for (size_t j = 0; j < (size_t)(m_Info.aas); j++) {
                                        for (size_t i = 0; i < (size_t)(m_Info.aas); i++) {
                                            const Vec3& color = colorBuffer[((y * (size_t)(m_Info.aas) + j) * srcWidth + (x * (size_t)(m_Info.aas) + i))];
                                            r += (size_t)(color.X * 255.0f);
                                            g += (size_t)(color.Y * 255.0f);
                                            b += (size_t)(color.Z * 255.0f);
                                        }
                                    }
                                    frameData[y * width3 + x * 3 + 0] = (unsigned char)(r / ((unsigned long long)m_Info.aas * m_Info.aas));
                                    frameData[y * width3 + x * 3 + 1] = (unsigned char)(g / ((unsigned long long)m_Info.aas * m_Info.aas));
                                    frameData[y * width3 + x * 3 + 2] = (unsigned char)(b / ((unsigned long long)m_Info.aas * m_Info.aas));
                                }
                            }
                        } else {
                            for (size_t y = 0; y < dstHeight; y++) {
                                for (size_t x = 0; x < dstWidth; x++) {
                                    const Vec3& color = colorBuffer[y * srcWidth + x];
                                    int index = y * width3 + x * 3;
                                    frameData[index + 0] = (unsigned char)(color.X * 255.0f);
                                    frameData[index + 1] = (unsigned char)(color.Y * 255.0f);
                                    frameData[index + 2] = (unsigned char)(color.Z * 255.0f);
                                }
                            }
                        }

                        {
                            std::lock_guard<std::mutex> lock(bufferMutexes[targetThread]);
                            std::vector<unsigned char> data(frameData, frameData + dstWidth * dstHeight * 3);
                            frameBuffers[targetThread][stolenFrame] = std::move(data);
                        }

                        writeFramesToPipe(targetThread);

                        frameCounter.fetch_add(1, std::memory_order_relaxed);

                        auto currentTime = std::chrono::steady_clock::now();
                        static constexpr std::chrono::seconds updateInterval(1);
                        if (currentTime - lastTime >= updateInterval) {
                            int renderedFrames = frameCounter.load(std::memory_order_relaxed) - lastFrameCounter.load(std::memory_order_relaxed);
                            currentFPS = (float)(renderedFrames) * 60.0f;
                            lastFrameCounter.store(frameCounter.load(std::memory_order_relaxed), std::memory_order_relaxed);
                            lastTime = currentTime;
                        }
                    }
                }

                writeFramesToPipe(i);
                for (int j = 0; j < CPUNum; j++) {
                    if (j != i) {
                        writeFramesToPipe(j);
                    }
                }

                delete[] frameData;

                std::lock_guard<std::mutex> lock(tempVFilesMutex);
                tempVideoFiles[i] = tempVideoFile;
            });
        }

        threads.emplace_back([&frameCounter, &frameNum, &currentFPS, &frameDone, this]() {
            const int fps = m_Info.FPS;
            const double totalFrames = (double)frameNum;
            std::vector<std::string> SYM = m_UI.SYM;

            std::this_thread::sleep_for(std::chrono::milliseconds(166));

            while (true) {
                const int renderedFrames = frameCounter.load(std::memory_order_relaxed);
                if (renderedFrames >= frameNum) break;

                const auto now = std::chrono::steady_clock::now();
                const float dur = std::chrono::duration_cast<std::chrono::duration<float>>(now - start).count();

                const double progress = (double)(renderedFrames) / totalFrames * 100.0;
                const double estimatedTotalTime = dur / (double)(renderedFrames)*totalFrames;
                const double remainingTime = ((double)(frameNum)-(double)(renderedFrames)) / (double)(currentFPS) * 60.0 + dur;

                std::string str;
                size_t BAR_LENGTH = 60;
                size_t totalSize = frameDone.size();

                for (size_t idx = 0; idx < BAR_LENGTH; ++idx) {
                    size_t start = idx * totalSize / BAR_LENGTH;
                    size_t end = (idx + 1) * totalSize / BAR_LENGTH;

                    if (start >= end) {
                        str += SYM[0];
                        continue;
                    }

                    int doneCount = 0;
                    for (size_t i = start; i < end && i < totalSize; ++i) {
                        if (frameDone[i]) doneCount++;
                    }

                    float rate = (float)doneCount / (float)(end - start);

                    int symIndex = (int)(rate * (SYM.size() - 1));
                    symIndex = std::clamp(symIndex, 0, (int)SYM.size() - 1);

                    str += SYM[symIndex];
                }
                str += "\033[0m";

                char buf[512];
                sprintf(buf, "\r%05.2f%% %03d.%03d/%03d.%03d %06.2fs/%06.2fs(%06.2fs) FPS: %03d ",
                    progress,
                    renderedFrames / fps, renderedFrames % fps,
                    (int)(totalFrames) / fps, (int)(totalFrames) % fps,
                    dur, estimatedTotalTime, remainingTime,
                    (int)(currentFPS) / 60
                );
                LogInfo("%s", (buf + str).c_str());
            }

            float time = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - start).count();
            LogInfo("Rendered videos in %.2fs, total frames: %d, average FPS: %.2f", time, frameNum, (double)(frameNum) / (double)time);
        });

        for (auto& t : threads) t.join();

        for (int i = 0; i < CPUNum; i++) {
            if (ffmpegPipes[i] != nullptr) {
                _pclose(ffmpegPipes[i]);
            }
        }

        std::ofstream inputListFile("input_list.txt");
        if (inputListFile.is_open()) {
            for (int i = 0; i < CPUNum; i++) {
                if (threadStart[i] < threadEnd[i] && !tempVideoFiles[i].empty()) {
                    inputListFile << "file '" << tempVideoFiles[i] << "'" << std::endl;
                }
            }
            inputListFile.close();
            system("ffmpeg -y -loglevel error -f concat -safe 0 -i input_list.txt -c:v copy -pix_fmt yuv420p -preset medium -b:v 8M rendered.mp4");
        }

        system((
            "ffmpeg -y -loglevel error -ss "
            + std::to_string(m_Info.startTime)
            + " -i \"" + m_Info.TempDir
            + "mixed.wav\" -t "
            + std::to_string(m_Info.endTime)
            + " -c copy mixed_cut.wav")
            .c_str()
        );

        ToDir(m_Info.WorkDir);
        std::string filename = m_Info.OutPath + ".mp4";
        Overwrite(filename);
        system((
            "ffmpeg -y -loglevel error -i \""
            + m_Info.TempDir + "rendered.mp4\" -i \""
            + m_Info.TempDir + "mixed_cut.wav\" -c:v copy -pix_fmt yuv420p -preset medium -b:v "
            + std::to_string(m_Info.bitrate) + "M -c:a aac -strict experimental -b:a 192k -shortest \""
            + filename + "\""
            ).c_str()
        );
    }

}
