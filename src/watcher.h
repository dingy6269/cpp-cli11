#pragma once

#include <vector>
#include <string>
#include <memory>

#include <efsw/efsw.hpp>


class DefaultListener : public efsw::FileWatchListener {
public:
    void handleFileAction(efsw::WatchID watchid, 
                          const std::string &dir,
                          const std::string &filename, 
                          efsw::Action action,
                          std::string oldFilename) override;
};


namespace app::watcher {
    

// perfect forwarding
int watch_dir(std::vector<std::unique_ptr<efsw::FileWatchListener>>&& custom_listeners);
int watch_dir(std::vector<std::unique_ptr<efsw::FileWatchListener>> custom_listeners, bool recursive);
}