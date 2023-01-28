#pragma once

#include "avro/Decoder.hh"
#include "avro/Encoder.hh"
#include "avro/Generic.hh"
#include "avro/Specific.hh"

namespace giggle {

namespace details {

struct EncoderArchiver {
  avro::Encoder &e;
  template <typename T> void operator&(const T &v) { avro::encode(e, v); }
};
struct DecoderArchiver {
  avro::Decoder &d;
  template <typename T> void operator&(T &v) { avro::decode(d, v); }
};

} // namespace details
} // namespace giggle

// 
// trait required by avro which uses 
// archiver approach.
//
// constraints:
//
// Type which is to be serailized has template static method
//
// template<typename Archiver, typename T>
// static void archive(Archiver& a, T &&o)
// {
//    a & o.field1;
//    a & o.field2;
//    ...
//    a & o.fieldN;
// }
//
//
#define GIGGLE_AVRO_ARCHIVE_TRAITS(Type)                                       \
  namespace avro {                                                             \
  template <> struct codec_traits<Type> {                                      \
    static void encode(Encoder &e, const Type &o) {                            \
      giggle::details::EncoderArchiver a{e};                                   \
      Type::archive(a, o);                                                     \
    }                                                                          \
    static void decode(Decoder &d, Type &o) {                                  \
      giggle::details::DecoderArchiver a{d};                                   \
      Type::archive(a, o);                                                     \
    }                                                                          \
  };                                                                           \
  }
