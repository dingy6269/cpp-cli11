#pragma once

#include <iostream>
#include <efsw/efsw.hpp>
#include <typeinfo>
#include <syncstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <efsw/FileSystem.hpp>
#include <efsw/System.hpp>
#include <functional>
#include "cli.hpp"


namespace fs = std::filesystem;

#define DBG(x)                                                                     \
    do                                                                             \
    {                                                                              \
        std::osyncstream(std::cerr)                                                \
            << __FILE__ << ':' << __LINE__ << " | " << #x << " = " << (x) << '\n'; \
    } while (0)

class UpdateListener : public efsw::FileWatchListener
{
public:
    void handleFileAction(efsw::WatchID watchid,
                          const std::string &dir,
                          const std::string &filename,
                          efsw::Action action, std::string oldFilename) override
    {
        DBG(watchid);
        DBG(dir);
        DBG(filename);
        DBG(action);
        DBG(oldFilename);
    }
};

using WatchHandler = std::function<void(const std::optional<RunConfig>&)>;

int watch(WatchHandler handler)
{
    std::string CurrentPath( efsw::System::getProcessPath());
    auto watcher = std::make_unique<efsw::FileWatcher>();

    // manual here
    // fs::path watchDir = fs::current_path().parent_path() / "dev";
    std::cout << "Watching: " << CurrentPath << std::endl;

    // std::cout << typeid(*watcher).name() << std::endl;

    UpdateListener *default_listener = new UpdateListener();
    efsw::WatchID watchId = watcher->addWatch(CurrentPath, default_listener, true);

    if (watchId < 0)
    {
        std::cerr << "addWatch failed: " << watchId << '\n';
        return 1;
    }

    watcher->watch();

    while (true)
    {
        std::this_thread::sleep_for(
            std::chrono::seconds(1));
    }

    return 0;
}