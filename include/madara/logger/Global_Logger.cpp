#include "Global_Logger.h"

std::auto_ptr <madara::logger::Logger> madara::logger::global_logger (
  new Logger ());
