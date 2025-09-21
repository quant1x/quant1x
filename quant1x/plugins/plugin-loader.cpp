#include <quant1x/plugins/plugin-loader.h>
#include <quant1x/engine/rule-engine.h>
#include <filesystem>
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#undef max  // 避免与 std::max 冲突
#undef min  // 避免与 std::min 冲突
#else
#include <dlfcn.h>
#endif

#include <spdlog/spdlog.h>
#include <quant1x/plugins/plugin-api.h>

namespace fs = std::filesystem;
using namespace engine;

void PluginLoader::LoadPlugins(const std::string& plugin_dir) {
    spdlog::info("开始加载插件目录: " + plugin_dir);

    for (const auto& entry : fs::directory_iterator(plugin_dir)) {
        if (!entry.is_regular_file()) continue;

        std::string path = entry.path().string();
        std::string ext = entry.path().extension().string();

        // 根据平台判断是否是动态库文件
#if defined(_WIN32) || defined(_WIN64)
        if (ext != ".dll") continue;
#else
        if (ext != ".so") continue;
#endif

        spdlog::info("加载插件: " + path);

#if defined(_WIN32) || defined(_WIN64)
        HMODULE handle = LoadLibraryA(path.c_str());
        if (!handle) {
            spdlog::error("无法加载 DLL: " + path);
            continue;
        }

        // 然后进行转换
        auto init_func = reinterpret_cast<PluginInitFunc>(reinterpret_cast<void*>(GetProcAddress(handle, "InitializePlugin")));
#else
        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            spdlog::error(std::string("无法加载 SO: ") + dlerror());
            continue;
        }

        auto init_func = (PluginInitFunc)dlsym(handle, "InitializePlugin");
#endif

        if (!init_func) {
            spdlog::error("插件缺少 InitializePlugin 函数");
#if defined(_WIN32)
            FreeLibrary(handle);
#else
            dlclose(handle);
#endif
            continue;
        }

        init_func(engine::RuleEngine::GetInstance().get());
        spdlog::info("插件加载成功: " + path);
    }
}