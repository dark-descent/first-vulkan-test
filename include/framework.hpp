#ifndef ENGINE_FRAMEWORK_HPP
#define ENGINE_FRAMEWORK_HPP

#include <vector>
#include <set>
#include <stdio.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <algorithm>
#include <limits>
#include <functional>
#include <unordered_map>
#include <type_traits>
#include <filesystem>
#include <stdarg.h>
#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include <libgen.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>

#include <v8/v8.h>
#include <v8/libplatform/libplatform.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#pragma region DEFINES

#ifdef NDEBUG
#define CHECK(expr, msg) { if(!expr) { printf(msg"\n"); return false; } else { printf(#expr" success!\n"); } }
#else
#define CHECK(expr, msg) if(!expr) return false;
#endif

#ifdef NDEBUG
#define ON_DEBUG(expr) expr
#else
#define ON_DEBUG(expr)
#endif

#pragma endregion

#endif
