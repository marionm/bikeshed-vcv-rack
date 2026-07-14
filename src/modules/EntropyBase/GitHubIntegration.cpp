#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "GitHubIntegration.hpp"

#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <rack.hpp>

#include <thread>

using namespace rack;

namespace {
  void fail(GitHubIntegration::Callback callback, std::string error) {
    callback(GitHubIntegration::Result{false, error, {}});
  }
}

void GitHubIntegration::fetchNormalizedContributions(std::string nameAndToken, int length, bool includeWeekends, Callback callback) {
  std::thread([=] {
    try {
      size_t atPos = nameAndToken.find('@');
      size_t splitIndex = (atPos == std::string::npos) ? 0 : atPos + 1;
      std::string username = (atPos == std::string::npos) ? "" : nameAndToken.substr(0, atPos);
      std::string token = nameAndToken.substr(splitIndex);

      if (token.empty()) {
        fail(callback, "Token is required");
        return;
      }

      httplib::SSLClient client("api.github.com");
      client.enable_server_certificate_verification(false);

      httplib::Headers headers = {
        {"Authorization", "Bearer " + token},
        {"Content-Type", "application/json"},
        {"User-Agent", "cpp-httplib-plugin"}
      };

      std::string header, requestScope, responseScope;
      if (username.empty()) {
        header = "query";
        requestScope = "viewer";
        responseScope = "viewer";
      } else {
        header = "query($username: String!)";
        requestScope = "user(login: $username)";
        responseScope = "user";
      }

      std::string query = "contributionsCollection { contributionCalendar { weeks { contributionDays { contributionCount weekday } } } }";
      std::string body = header + " { " + requestScope + " { " + query + " } }";

      nlohmann::json jsonBody;
      jsonBody["query"] = body;
      if (!username.empty()) {
        jsonBody["variables"] = {{"username", username}};
      }

      if (auto res = client.Post("/graphql", headers, jsonBody.dump(), "application/json")) {
        if (res->status == 200) {
          try {
            auto json = nlohmann::json::parse(res->body);
            auto calendar = json.at("data").at(responseScope).at("contributionsCollection").at("contributionCalendar");
            auto normalizedContributions = normalizeContributions(calendar, length, includeWeekends);
            callback(GitHubIntegration::Result{true, "", normalizedContributions});
          } catch (...) {
            DEBUG("%s", res->body.c_str());
            fail(callback, "Parsing error");
          }
        } else if (res->status == 401) {
          fail(callback, "Bad token (401)");
        } else if (res->status == 403) {
          fail(callback, "Unauthorized (403)");
        } else if (res->status >= 500) {
          fail(callback, string::f("API error (%i)", res->status));
        } else if (res->status >= 400) {
          fail(callback, string::f("Request error (%i)", res->status));
        } else {
          fail(callback, string::f("Error (%i)", res->status));
        }
      } else {
        fail(callback, "Failed");
      }
    } catch (...) {
      fail(callback, "Failed");
    }
  }).detach();
}

std::vector<float> GitHubIntegration::normalizeContributions(const nlohmann::json& calendar, int length, bool includeWeekends) {
  std::vector<int> contributions;

  int maxValue = 0;
  auto weeks = calendar.at("weeks");
  for (int w = (int)weeks.size() - 1; w >= 0 && contributions.size() < (size_t)length; --w) {
    const auto& days = weeks.at(w).at("contributionDays");
    for (int d = (int)days.size() - 1; d >= 0 && contributions.size() < (size_t)length; --d) {
      auto day = days.at(d);
      int weekday = day.at("weekday").get<int>();
      if (includeWeekends || (weekday != 0 && weekday != 6)) {
        int value = days.at(d).at("contributionCount").get<int>();
        contributions.push_back(value);
        maxValue = std::max(value, maxValue);
      }
    }
  }

  float scale = maxValue == 0 ? 0.f : 1.f / (float)maxValue;

  std::vector<float> values;
  for (int i = contributions.size() - 1; i >= 0; --i) {
    values.push_back((float)contributions[i] * scale);
  }

  while (values.size() < (size_t)length) {
    values.push_back(0);
  }

  return values;
}
