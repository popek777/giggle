#pragma once

#include <giggle/AvroUtils.h>

#include <ctime>
#include <string>

namespace giggle {

class Message {
public:
  std::string senderId;
  std::string payload;
  time_t generationTmstmp;

  template <typename Archiver, typename MessageT>
  static void archive(Archiver &a, MessageT &&m) {
    a &m.senderId;
    a &m.payload;
    a &m.generationTmstmp;
  }
};

} // namespace giggle

GIGGLE_AVRO_ARCHIVE_TRAITS(giggle::Message);

