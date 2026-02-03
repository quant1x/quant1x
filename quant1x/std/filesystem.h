#pragma once
#ifndef QUANT1X_STD_FILESYSTEM_H
#define QUANT1X_STD_FILESYSTEM_H

#include <string>
#include <system_error>

namespace filesystem {

/**
 * @brief Get the current user's home directory
 * 
 * Priority:
 * 1. QUANT1X_HOME environment variable
 * 2. GOX_HOME environment variable
 * 3. HOME environment variable
 * 4. USERPROFILE environment variable (Windows only)
 * 5. System temporary directory (fallback)
 * 
 * @return std::string The home directory path
 */
std::string homedir();

/**
 * @brief Expand user home directory in path (e.g. "~/data" -> "/home/user/data")
 * 
 * @param path The path to expand
 * @return std::string The expanded path
 */
std::string expand_user(const std::string &path);

std::error_code mkdirs(const std::string &path, bool notExistToCreate = true);
std::error_code check_filepath(const std::string &filename, bool notExistToCreate = false);

} // namespace filesystem

#endif // QUANT1X_STD_FILESYSTEM_H
