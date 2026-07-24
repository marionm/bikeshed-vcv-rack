#include "GitHubIntegration.hpp"

#include <curl/curl.h>

#include <thread>

using namespace rack;

namespace {
  struct CurlCleanup {
    CURL* curl = nullptr;
    curl_slist* headers = nullptr;
    char* requestBody = nullptr;
    json_t* requestJson = nullptr;
    json_t* responseJson = nullptr;

    ~CurlCleanup() {
      if (requestBody) {
        free(requestBody);
      }

      if (requestJson) {
        json_decref(requestJson);
      }

      if (responseJson) {
        json_decref(responseJson);
      }

      if (headers) {
        curl_slist_free_all(headers);
      }

      if (curl) {
        curl_easy_cleanup(curl);
      }
    }
  };

  size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total);
    return total;
  }

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

      CURL* curl = curl_easy_init();
      if (!curl) {
        fail(callback, "Failed to initialize curl");
        return;
      }

      CurlCleanup cleanup;
      cleanup.curl = curl;

      curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/graphql");
      curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
      curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

      struct curl_slist* headers = nullptr;
      headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
      headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
      headers = curl_slist_append(headers, "Content-Type: application/json");
      headers = curl_slist_append(headers, "User-Agent: bikeshed-vcv-rack");
      cleanup.headers = headers;
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

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

      json_t* jsonBody = json_object();
      cleanup.requestJson = jsonBody;
      json_object_set_new(jsonBody, "query", json_string(body.c_str()));
      if (!username.empty()) {
        json_t* variables = json_object();
        json_object_set_new(variables, "username", json_string(username.c_str()));
        json_object_set_new(jsonBody, "variables", variables);
      }

      char* requestBody = json_dumps(jsonBody, JSON_COMPACT);
      cleanup.requestBody = requestBody;
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody);

      std::string response;
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

      CURLcode result = curl_easy_perform(curl);
      if (result != CURLE_OK) {
        fail(callback, curl_easy_strerror(result));
        return;
      }

      long status = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

      if (status == 200) {
        json_error_t error;
        json_t* json = json_loads(response.c_str(), 0, &error);
        cleanup.responseJson = json;
        if (!json) {
          DEBUG("%s", response.c_str());
          fail(callback, "Parsing error");
          return;
        }

        json_t* errors = json_object_get(json, "errors");
        if (errors) {
          fail(callback, "GitHub API error");
          return;
        }

        json_t* data = json_object_get(json, "data");
        json_t* scope = json_object_get(data, responseScope.c_str());
        json_t* collection = json_object_get(scope, "contributionsCollection");
        json_t* calendar = json_object_get(collection, "contributionCalendar");

        auto normalized = normalizeContributions(calendar, length, includeWeekends);
        callback({ true, "", normalized });
      } else if (status == 401) {
        fail(callback, "Bad token (401)");
      } else if (status == 403) {
        fail(callback, "Unauthorized (403)");
      } else if (status >= 500) {
        fail(callback, string::f("Error (%i)", status));
      } else {
        fail(callback, "Failed");
      }
    } catch (...) {
      fail(callback, "Failed");
    }
  }).detach();
}

std::vector<float> GitHubIntegration::normalizeContributions(json_t* calendar, int length, bool includeWeekends) {
  std::vector<int> contributions;

  int maxValue = 0;
  json_t* weeks = json_object_get(calendar, "weeks");
  for (int w = json_array_size(weeks) - 1; w >= 0 && contributions.size() < (size_t)length; --w) {
    json_t* week = json_array_get(weeks, w);
    json_t* days = json_object_get(week, "contributionDays");

    for (int d = json_array_size(days) - 1; d >= 0 && contributions.size() < (size_t)length; --d) {
      json_t* day = json_array_get(days, d);
      int weekday = json_integer_value(json_object_get(day, "weekday"));

      if (includeWeekends || (weekday != 0 && weekday != 6)) {
        int value = json_integer_value(json_object_get(day, "contributionCount"));
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
