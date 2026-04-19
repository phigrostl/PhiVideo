#include "Application.h"
#include <mutex>
#include <thread>
#include <algorithm>
#include <atomic>

namespace PhiVideo {

    void Application::MixMusic() {
        ToDir(m_Info.TempDir);

        if (m_Info.musicVolume == 0 && m_Info.notesVolume == 0) {
            GenerateEmptyAudio("mixed.wav", (float)m_Info.chart.data.time);
            LogInfo("Mixed music");
            return;
        }

        const std::string musicFile = m_Info.ChartDir + "\\" + m_Info.chart.info.song;
        const int BATCH_SIZE = 100;
        const int CPU_NUM = m_Info.CPUNum;

        system((FfmpegBaseCmd() + "-i \"" + musicFile + "\" music.wav").c_str());
        GenerateEmptyAudio("empty.wav", 1.0f);
        auto allNotes = CollectAllNotes();
        std::sort(
            allNotes.begin(), allNotes.end(), [](const NoteSoundInfo& a, const NoteSoundInfo& b) {
            return a.delayTime < b.delayTime;
        });

        std::vector<std::thread> audioThreads;
        std::vector<std::string> tempBatchFiles;
        std::mutex filesMutex;
        std::atomic<int> completedBatches = 0;
        std::atomic<int> runningThreads = 0;
        const size_t totalNotes = allNotes.size();
        const int totalBatches = (totalNotes + BATCH_SIZE - 1) / BATCH_SIZE;

        for (int i = 0; i < totalBatches; ++i) {
            audioThreads.emplace_back([&, i]() {
                while (runningThreads >= CPU_NUM) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                runningThreads++;
                ProcessNoteBatch(
                    i, allNotes, tempBatchFiles, filesMutex, completedBatches, totalBatches
                );
                runningThreads--;
            });
        }

        for (auto& t : audioThreads) t.join();
        MergeAudioFiles(tempBatchFiles, "notes.wav");
        MixFinalAudio(musicFile);

        LogInfo("Mixed music");
    }

    void Application::GenerateEmptyAudio(const std::string& outputFile, float duration) {
        std::string cmd = FfmpegBaseCmd() +
            "-f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 -t " +
            std::to_string(duration) + " " + outputFile;
        system(cmd.c_str());
    }

    std::vector<NoteSoundInfo> Application::CollectAllNotes() {
        std::vector<NoteSoundInfo> notes;
        notes.reserve(m_Info.chart.data.noteCount);

        const std::string click = m_Info.ResDir + "\\Notes\\Tap.wav";
        const std::string drag = m_Info.ResDir + "\\Notes\\Drag.wav";
        const std::string flick = m_Info.ResDir + "\\Notes\\Flick.wav";

        for (const auto& judgeLine : m_Info.chart.data.judgeLines) {
            for (const auto& note : judgeLine.notes) {
                NoteSoundInfo info;
                switch (note.type) {
                case 1: case 3: info.soundFile = click; break;
                case 2: info.soundFile = drag; break;
                case 4: info.soundFile = flick; break;
                default: continue;
                }
                info.delayTime = (note.secTime - m_UI.noteDelays[note.type - 1]) * 1000.0f;
                if (info.delayTime / 1000.0f >= m_Info.startTime && info.delayTime / 1000.0f <= m_Info.endTime) {
                    notes.push_back(info);
                }
            }
        }
        return notes;
    }

    void Application::ProcessNoteBatch(int batchIdx, const std::vector<NoteSoundInfo>& allNotes,
        std::vector<std::string>& tempAudioFiles, std::mutex& filesMutex,
        std::atomic<int>& completedBatches, int totalBatches) {
        const int BATCH_SIZE = 100;
        const int batchStart = batchIdx * BATCH_SIZE;
        const int batchEnd = Min(batchStart + BATCH_SIZE, (int)allNotes.size());

        std::vector<std::string> inputFiles;
        std::vector<std::string> filterParts;
        std::string baseInput = "0:a";
        int inputIndex = 1;

        for (int k = batchStart; k < batchEnd; ++k) {
            const auto& note = allNotes[k];
            std::string delay = std::to_string((int)(note.delayTime + 0.5f));

            size_t fileIdx = 0;
            bool exists = false;
            for (size_t i = 0; i < inputFiles.size(); ++i) {
                if (inputFiles[i] == note.soundFile) {
                    exists = true;
                    fileIdx = i + 1;
                    break;
                }
            }

            if (!exists) {
                inputFiles.push_back(note.soundFile);
                fileIdx = inputIndex++;
            }

            filterParts.push_back(
                "[" + std::to_string(fileIdx) +
                ":a]adelay=" + delay + "|" + delay +
                "[delayed" + std::to_string(k) + "];"
            );
        }

        std::string outputFile = "batch_" + std::to_string(batchIdx) + ".wav";
        std::string cmd = FfmpegBaseCmd() + "-i empty.wav";
        for (const auto& f : inputFiles) cmd += " -i " + f;

        cmd += " -filter_complex \"";
        for (const auto& p : filterParts) cmd += p;
        cmd += "[0:a]";
        for (int k = batchStart; k < batchEnd; ++k) cmd += "[delayed" + std::to_string(k) + "]";
        cmd += "amix=inputs=" + std::to_string(1 + batchEnd - batchStart)
            + ":duration=longest:normalize=0\" " + outputFile;

        system(cmd.c_str());

        std::lock_guard<std::mutex> lock(filesMutex);
        tempAudioFiles.push_back(outputFile);

        completedBatches++;
        LogInfo("\rProcessing batch %d/%d", (int)completedBatches, totalBatches);
    }

    void Application::MergeAudioFiles(
        const std::vector<std::string>& inputFiles,
        const std::string& outputFile
    ) {
        if (inputFiles.empty()) {
            system((FfmpegBaseCmd() + "-i empty.wav " + outputFile).c_str());
            return;
        }

        std::string cmd = FfmpegBaseCmd();
        for (const auto& f : inputFiles) cmd += " -i " + f;
        cmd += " -filter_complex \"";
        for (int i = 0; i < inputFiles.size(); ++i) cmd += "[" + std::to_string(i) + ":a]";
        cmd += "amix=inputs=" + std::to_string(inputFiles.size()) + ":duration=longest:normalize=0\" " + outputFile;

        system(cmd.c_str());
    }

    void Application::MixFinalAudio(const std::string& musicFile) const {
        std::string cmd = FfmpegBaseCmd() +
            "-i \"" + musicFile + "\" -i notes.wav "
            "-filter_complex \"[0:a]volume=" + std::to_string(m_Info.musicVolume) +
            "[a0];[1:a]volume=" + std::to_string(m_Info.notesVolume) +
            "[a1];[a0][a1]amix=inputs=2:duration=longest:normalize=0\" mixed.wav";
        system(cmd.c_str());
    }

}
