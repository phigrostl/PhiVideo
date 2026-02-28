#include "Application.h"

namespace PGR {

	void Application::MixMusic() {
        ToDir(m_Info.TempDir);
        int CPUNum = m_Info.CPUNum;

        std::string click = m_Info.ResDir + "\\Notes\\Tap.wav";
        std::string drag = m_Info.ResDir + "\\Notes\\Drag.wav";
        std::string flick = m_Info.ResDir + "\\Notes\\Flick.wav";
        std::string music = m_Info.ChartDir + "\\" + m_Info.chart.info.song;

        system(("ffmpeg -y -loglevel error -i \"" + music + "\" music.wav").c_str());
        const int BATCH_SIZE = 100;

        struct NoteInfo {
            std::string soundFile;
            float delayTime = 0.0f;
        };

        std::vector<NoteInfo> allNotes;
        allNotes.reserve(m_Info.chart.data.noteCount);

        for (int i = 0; i < m_Info.chart.data.judgeLines.size(); i++) {
            for (const auto& note : m_Info.chart.data.judgeLines[i].notes) {
                NoteInfo ni;

                float delay = 0;

                switch (note.type) {
                case 1:
                case 3:
                    ni.soundFile = click;
                    break;
                case 2:
                    ni.soundFile = drag;
                    break;
                case 4:
                    ni.soundFile = flick;
                    break;
                default:
                    continue;
                }

                ni.delayTime = (note.secTime - m_UI.noteDelays[note.type - 1]) * 1000.0f;
                allNotes.push_back(ni);
            }
        }

        std::sort(allNotes.begin(), allNotes.end(), [](const NoteInfo& a, const NoteInfo& b)
            { return a.delayTime < b.delayTime; });

        size_t totalNotes = allNotes.size();
        int totalBatches = (int)((totalNotes + BATCH_SIZE - 1) / BATCH_SIZE);
        std::vector<std::thread> audioThreads;
        std::vector<std::string> tempAudioFiles;
        std::mutex tempMFilesMutex;
        std::atomic<int> completedBatches = 0;
        int ThreadsN = 0;

        system("ffmpeg -y -loglevel error -f lavfi -i anullsrc=channel_layout=stereo:sample_rate=44100 -t 1 empty.wav");

        for (int batchIdx = 0; batchIdx < totalBatches; batchIdx++) {
            audioThreads.emplace_back([this, batchIdx, BATCH_SIZE, &allNotes, totalNotes, &tempAudioFiles, &tempMFilesMutex, &completedBatches, totalBatches, &ThreadsN, CPUNum]() {

                while (ThreadsN >= CPUNum)
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                ThreadsN++;

                int batchStart = batchIdx * BATCH_SIZE;
                int batchEnd = (int)Min((float)(batchStart + BATCH_SIZE), (float)totalNotes);
                int currentBatchSize = batchEnd - batchStart;

                std::vector<std::string> tempInputFiles;
                std::vector<std::string> filterComplexParts;
                std::string baseInput = "0:a";
                int inputIndex = 1;

                for (int k = batchStart; k < batchEnd; k++) {
                    const auto& note = allNotes[k];
                    std::string delayTime = std::to_string((int)(note.delayTime + 0.5f));

                    bool fileExists = false;
                    size_t existingIndex = 0;
                    for (size_t idx = 0; idx < tempInputFiles.size(); idx++) {
                        if (tempInputFiles[idx] == note.soundFile) {
                            fileExists = true;
                            existingIndex = idx + 1;
                            break;
                        }
                    }

                    if (!fileExists) {
                        tempInputFiles.push_back(note.soundFile);
                        existingIndex = inputIndex;
                        inputIndex++;
                    }

                    std::string outputLabel = "delayed" + std::to_string(k);
                    filterComplexParts.push_back("[" + std::to_string(existingIndex) + ":a]adelay=" +
                        delayTime + "|" + delayTime + "[" + outputLabel + "];");
                }

                std::string tempOutputFile = "batch_" + std::to_string(batchIdx) + ".wav";
                std::string batchCmd = "ffmpeg -y -loglevel error -i empty.wav";

                for (const auto& file : tempInputFiles) {
                    batchCmd += " -i " + file;
                }

                batchCmd += " -filter_complex \"";

                for (const auto& filter : filterComplexParts) {
                    batchCmd += filter;
                }

                batchCmd += "[0:a]";
                for (int k = batchStart; k < batchEnd; k++) {
                    batchCmd += "[delayed" + std::to_string(k) + "]";
                }

                batchCmd += "amix=inputs=" +
                    std::to_string(1 + (batchEnd - batchStart)) +
                    ":duration=longest:normalize=0\" " + tempOutputFile;

                system(batchCmd.c_str());

                {
                    std::lock_guard<std::mutex> lock(tempMFilesMutex);
                    tempAudioFiles.push_back(tempOutputFile);
                }

                completedBatches++;
                LogInfo("\rProcessing batch %d/%d", (int)completedBatches, totalBatches);

                ThreadsN--;
                });

        }

        for (auto& thread : audioThreads) {
            thread.join();
        }

        if (tempAudioFiles.size() > 0) {
            std::string mergeCmd = "ffmpeg -y -loglevel error";

            for (const auto& file : tempAudioFiles) {
                mergeCmd += " -i " + file;
            }

            mergeCmd += " -filter_complex \"";
            for (int i = 0; i < tempAudioFiles.size(); i++) {
                mergeCmd += "[" + std::to_string(i) + ":a]";
            }
            mergeCmd += "amix=inputs=" + std::to_string(tempAudioFiles.size()) + ":duration=longest:normalize=0\" notes.wav";
            system(mergeCmd.c_str());
        }
        else {
            system("ffmpeg -y -loglevel error -i empty.wav notes.wav");
        }

        std::string mixCmd = "ffmpeg -y -loglevel error -i \"" + music + "\" -i notes.wav " +
            "-filter_complex \"[0:a]volume=" + std::to_string(m_Info.musicVolume) + "[a0];[1:a]volume=" + std::to_string(m_Info.notesVolume) + "[a1];[a0][a1]amix=inputs=2:duration=longest:normalize=0\" mixed.wav";
        system(mixCmd.c_str());

        LogInfo("Mixed music");
	}

}
