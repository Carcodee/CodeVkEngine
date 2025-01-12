//
// Created by carlo on 2024-10-24.
//

#ifndef SYSTEMSINCLUDE_HPP
#define SYSTEMSINCLUDE_HPP

#include <any>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <typeindex>
#include <map>
#include <unordered_map>
#include <fstream>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <filesystem>
#include <assert.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "TaskThreat.hpp"
#include "SingletonStorage.hpp"
#include "Logger.hpp"
#include "OS.hpp"
#include "InputSystem.hpp"
#include "ObserverSystem.hpp"
#include "FileWatcher.hpp"

#endif //SYSTEMSINCLUDE_HPP
