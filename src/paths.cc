/*
 * Runtime path helpers.
 */

#include "paths.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <vector>

static bool path_exists(const std::string& path)
{
	return g_file_test(path.c_str(), G_FILE_TEST_EXISTS);
}

static std::string join_path(const std::string& dir, const std::string& name)
{
	if(dir.empty())
		return name;
	if(dir[dir.size() - 1] == '/')
		return dir + name;
	return dir + "/" + name;
}

static std::string executable_dir()
{
	char path[PATH_MAX];
	ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
	if(len <= 0)
		return std::string();
	path[len] = '\0';

	char* last_slash = strrchr(path, '/');
	if(!last_slash)
		return std::string();
	*last_slash = '\0';
	return std::string(path);
}

std::string gmchess_data_dir()
{
	const char* env_dir = g_getenv("GMCHESS_DATA_DIR");
	if(env_dir && path_exists(join_path(env_dir, "gmchess.png")))
		return std::string(env_dir);

	if(path_exists(DATA_DIR "/gmchess.png"))
		return DATA_DIR;

	const std::string exe_dir = executable_dir();
	char* current_dir_raw = g_get_current_dir();
	const std::string current_dir = current_dir_raw ? current_dir_raw : "";
	g_free(current_dir_raw);
	const std::vector<std::string> candidates = {
		join_path(exe_dir, "../share/gmchess"),
		join_path(exe_dir, "../../data"),
		join_path(current_dir, "data"),
	};

	for(std::vector<std::string>::const_iterator iter = candidates.begin();
			iter != candidates.end(); ++iter) {
		if(path_exists(join_path(*iter, "gmchess.png")))
			return *iter;
	}

	return DATA_DIR;
}

std::string gmchess_data_path(const std::string& relative_path)
{
	return join_path(gmchess_data_dir(), relative_path);
}

std::string gmchess_engine_path(const std::string& engine_name)
{
	if(engine_name.find('/') != std::string::npos)
		return engine_name;

	const char* env_dir = g_getenv("GMCHESS_ENGINE_DIR");
	if(env_dir) {
		const std::string path = join_path(env_dir, engine_name);
		if(path_exists(path))
			return path;
	}

	const std::string exe_dir = executable_dir();
	const std::vector<std::string> candidates = {
		join_path(exe_dir, engine_name),
		join_path(exe_dir, join_path("engine", engine_name)),
	};

	for(std::vector<std::string>::const_iterator iter = candidates.begin();
			iter != candidates.end(); ++iter) {
		if(path_exists(*iter))
			return *iter;
	}

	return engine_name;
}
