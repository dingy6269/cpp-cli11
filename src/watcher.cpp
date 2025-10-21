#include "watcher.h"

#include "core_defs.hpp"
#include <chrono>
#include <efsw/FileSystem.hpp>
#include <efsw/System.hpp>
#include <efsw/efsw.hpp>
#include <filesystem>
#include <functional>
#include <iostream>
#include <syncstream>
#include <thread>
#include <typeinfo>



void DefaultListener::handleFileAction(efsw::WatchID watchid, 
                                       const std::string &dir,
                                       const std::string &filename, 
                                       efsw::Action action,
                                       std::string oldFilename) {
    DBG(watchid);
    DBG(dir);
    DBG(filename);
    DBG(action);
    DBG(oldFilename);
}

namespace app::watcher {


int watch_dir(
    std::vector<std::unique_ptr<efsw::FileWatchListener>>&& custom_listeners
) {
  return watch_dir(std::move(custom_listeners), true);
}

int watch_dir(
    std::vector<std::unique_ptr<efsw::FileWatchListener>> custom_listeners,
    bool recursive) {
  std::string CurrentPath(efsw::System::getProcessPath());
  auto watcher = std::make_unique<efsw::FileWatcher>();

  std::cout << "Watching: " << CurrentPath << std::endl;

  auto add_listener = [&](efsw::FileWatchListener *listener) -> int {
    efsw::WatchID watchId = watcher->addWatch(CurrentPath, listener, recursive);

    if (watchId < 0) {
      std::cerr << "watcher failed: " << watchId << '\n';
      return 1;
    };

    return 0;
  };

  //  TODO: change the typings
  //  should be array of [watcher_class, directory]
  //  allows only one handler
  //   if (add_listener(new DefaultListener())) return 1;

  for (auto &listener : custom_listeners) {
    if (add_listener(listener.get()))
      return 1;
  };

  watcher->watch();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}
}
