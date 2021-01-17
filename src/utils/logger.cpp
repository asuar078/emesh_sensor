//
// Created by bigbywolf on 1/15/21.
//

#include "logger.hpp"

Logger& Logger::get_instance() noexcept
{
  static Logger instance; /* Guaranteed to be destroyed */
  /* Instantiated on first use. */
  return instance;
}

