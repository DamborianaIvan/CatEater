#pragma once

#include <Arduino.h>
#include <vector>
#include <utility>

using HttpHeader = std::pair<String, String>;
using HttpHeaders = std::vector<HttpHeader>;