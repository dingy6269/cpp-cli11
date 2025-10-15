#include <iostream>
#include <efsw/efsw.hpp>
#include <typeinfo>
#include <syncstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <efsw/FileSystem.hpp>
#include <efsw/System.hpp>


// class UpdateListener:
//     public efsw:

// int main() {

//     `
// }

// dbg!(x) equivalent
#define DBG(x)                                                                     \
    do                                                                             \
    {                                                                              \
        std::osyncstream(std::cerr)                                                \
            << __FILE__ << ':' << __LINE__ << " | " << #x << " = " << (x) << '\n'; \
    } while (0)

// efsw

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

int watch()
{
    std::string CurPath( efsw::System::getProcessPath());

    std::cout << CurPath << std::endl;

    auto watcher = std::make_unique<efsw::FileWatcher>();

    fs::path watchDir = fs::current_path().parent_path() / "dev";
    std::cout << "Watching: " << watchDir << std::endl;

    std::cout << typeid(*watcher).name() << std::endl;

    UpdateListener *listener = new UpdateListener();
    efsw::WatchID watchId = watcher->addWatch(watchDir, listener, true);

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