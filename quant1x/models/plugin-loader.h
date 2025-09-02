#pragma once
#ifndef QUANT1X_PLUGIN_LOADER_H
#define QUANT1X_PLUGIN_LOADER_H 1

#include <string>
#include <vector>

namespace engine {

    class PluginLoader {
    public:
        static void LoadPlugins(const std::string& plugin_dir);
    };

} // namespace engine

#endif //QUANT1X_PLUGIN_LOADER_H
