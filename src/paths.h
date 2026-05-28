/*
 * Runtime path helpers.
 */

#ifndef GMCHESS_PATHS_H
#define GMCHESS_PATHS_H

#include <string>

std::string gmchess_data_dir();
std::string gmchess_data_path(const std::string& relative_path);
std::string gmchess_engine_path(const std::string& engine_name);

#endif
