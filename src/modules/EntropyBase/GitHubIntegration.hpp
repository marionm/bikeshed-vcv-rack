#pragma once

#include <rack.hpp>

#include <functional>
#include <string>
#include <vector>

struct GitHubIntegration {
  struct Result {
    bool success;
    std::string error;
    std::vector<float> values;
  };

  using Callback = std::function<void(Result)>;

  static void fetchNormalizedContributions(std::string nameAndToken, int length, bool includeWeekends, Callback callback);

private:
  static std::vector<float> normalizeContributions(json_t* contributions, int length, bool includeWeekends);
};
