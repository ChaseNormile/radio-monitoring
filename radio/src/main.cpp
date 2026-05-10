#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "config.h"
#include "database.h"
#include "recorder.h"
#include "transcription.h"

struct Job
{
    Station station;
    RecordingResult recording;
};

std::queue<Job> jobQueue;
std::mutex queueMutex;
std::condition_variable queueCondition;
bool shutdownRequested = false;



void recordStationLoop(const Station& station, const Config& config)
{
    std::cout << "Starting recording thread: " << station.name << std::endl;

    Recorder recorder(config);

    bool continuousMode = config.getBool("continuousMode", true);
    int maxLoops = config.getInt("maxLoops", 0);
    int recordingDuration = config.getInt("recordingDuration", 60);
    int recordingOverlap = config.getInt("recordingOverlap", 5);

    int targetIntervalSeconds = recordingDuration - recordingOverlap;

    if (targetIntervalSeconds < 0)
    {
        targetIntervalSeconds = 0;
    }

    int loopCount = 0;

    do
    {
        auto loopStart = std::chrono::steady_clock::now();

        loopCount++;

        std::cout << "\n[" << station.name << "] Recording loop "
                  << loopCount << std::endl;

        RecordingResult recording = recorder.recordStation(station);

        if (recording.success)
        {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                jobQueue.push({station, recording});
            }

            queueCondition.notify_one();

            std::cout << "[" << station.name << "] Queued recording for transcription."
                      << std::endl;
        }
        else
        {
            std::cerr << "[" << station.name << "] Recording failed." << std::endl;
        }

        if (!continuousMode)
        {
            break;
        }

        if (maxLoops > 0 && loopCount >= maxLoops)
        {
            break;
        }

        auto loopEnd = std::chrono::steady_clock::now();

        auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
            loopEnd - loopStart
        ).count();

        int sleepSeconds = targetIntervalSeconds - elapsedSeconds;

        if (sleepSeconds > 0)
        {
            std::cout << "[" << station.name << "] Sleeping "
                      << sleepSeconds
                      << " seconds before next recording."
                      << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(sleepSeconds));
        }
        else
        {
            std::cout << "[" << station.name << "] Starting next recording immediately."
                      << std::endl;
        }

    } while (true);

    std::cout << "Finished recording thread: " << station.name << std::endl;
}

void transcriptionWorker(const Config& config, int workerId)
{
    std::cout << "Starting transcription worker " << workerId << std::endl;

    Database db;

    if (!db.connect(config))
    {
        std::cerr << "Database connection failed for transcription worker "
                  << workerId
                  << std::endl;
        return;
    }

    TranscriptionClient transcriber(config);

    while (true)
    {
        Job job;

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            queueCondition.wait(lock, [] {
                return !jobQueue.empty() || shutdownRequested;
            });

            if (shutdownRequested && jobQueue.empty())
            {
                break;
            }

            job = jobQueue.front();
            jobQueue.pop();
        }

        std::cout << "[Worker " << workerId << "] Transcribing "
                  << job.recording.filePath
                  << std::endl;

        std::string transcription =
            transcriber.transcribeFile(job.recording.filePath);

        if (transcription.empty())
        {
            transcription = "Transcription failed or returned empty text.";
        }

        db.insertRecording(
            job.station.name,
            job.recording.startTime,
            job.recording.endTime,
            transcription,
            job.recording.filePath,
            job.recording.durationSeconds
        );

        std::cout << "[Worker " << workerId << "] Finished "
                  << job.station.name
                  << std::endl;
    }

    std::cout << "Stopping transcription worker " << workerId << std::endl;
}



int main()
{
    Config config;

    if (!config.load("config.ini"))
    {
        return 1;
    }

    std::vector<Station> stations = config.getStations();

    std::cout << "Loaded " << stations.size() << " stations.\n";

    int transcriptionWorkers = config.getInt("transcriptionWorkers", 2);

    std::vector<std::thread> recordingThreads;
    std::vector<std::thread> workerThreads;

    for (int i = 0; i < transcriptionWorkers; i++)
    {
        workerThreads.emplace_back(transcriptionWorker, std::ref(config), i + 1);
    }

    for (const Station& station : stations)
    {
        recordingThreads.emplace_back(recordStationLoop, station, std::ref(config));
    }

    for (std::thread& t : recordingThreads)
    {
        t.join();
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        shutdownRequested = true;
    }

    queueCondition.notify_all();

    for (std::thread& t : workerThreads)
    {
        t.join();
    }

    std::cout << "All threads finished.\n";

    return 0;
}