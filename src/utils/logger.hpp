//
// Created by bigbywolf on 1/15/21.
//

#ifndef EMESH_SENSOR_LOGGER_HPP
#define EMESH_SENSOR_LOGGER_HPP

#include <zpp.hpp>

class Logger {
  private:
    Logger() noexcept = default;
    zpp::mutex log_mutex{};

  public:
    static Logger& get_instance() noexcept;

    template<class ...Args>
    void print(const char* fmt, Args... args) noexcept
    {
      auto lock = zpp::lock_guard<zpp::mutex>{log_mutex};
      zpp::print(fmt, args...);
    }


    ~Logger() = default;
    Logger(Logger const&) = delete;
    void operator=(Logger const&) = delete;
};

#define log_msg(M, ...) Logger::get_instance().print(M "\n", ##__VA_ARGS__)

#endif //EMESH_SENSOR_LOGGER_HPP
