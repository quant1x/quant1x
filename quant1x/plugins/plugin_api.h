#pragma once
#ifndef QUANT1X_PLUGIN_API_H
#define QUANT1X_PLUGIN_API_H 1

#include <quant1x/engine/rule_engine.h>

#ifdef _WIN32
#define api_declare __cdecl
#else
#define api_declare
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (api_declare *PluginInitFunc)(engine::RuleEngine*);

#ifdef __cplusplus
}
#endif

#endif  // QUANT1X_PLUGIN_API_H
