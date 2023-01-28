#include <giggle/Error.h>
#include <giggle/Messages.h>
#include <giggle/Producer.h>

#include "avro/Stream.hh"

#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
class DeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
  void dr_cb(RdKafka::Message &message) {
    if (message.err())
      std::cerr << "% Message delivery failed: " << message.errstr()
                << std::endl;
    else
      std::cerr << "% Message delivered to topic " << message.topic_name()
                << " [" << message.partition() << "] at offset "
                << message.offset() << std::endl;
  }
};

} // namespace

namespace giggle {

Producer::Producer(const Settings &settings)
    : callback(std::make_unique<DeliveryReportCb>()), topic(settings.topic),
      partition(settings.partition) {

  auto conf = std::unique_ptr<RdKafka::Conf>{
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)};

  std::string err;
  if (conf->set("bootstrap.servers", settings.broker, err) !=
      RdKafka::Conf::CONF_OK) {
    throw Error{"failed to set broker: " + err};
  }

  if (conf->set("dr_cb", callback.get(), err) != RdKafka::Conf::CONF_OK) {
    throw Error{"failed to set delivery callback: " + err};
  }

  impl.reset(RdKafka::Producer::create(conf.get(), err));
  if (nullptr == impl) {
    throw Error{"failed to crate kafka producer: " + err};
  }
}

Producer::~Producer() {
  /* Wait for final messages to be delivered or fail.
   * flush() is an abstraction over poll() which
   * waits for all messages to be delivered. */
  impl->flush(10 * 1000 /* wait for max 10 seconds */);

  if (impl->outq_len() > 0)
    std::cerr << "% " << impl->outq_len() << " message(s) were not delivered"
              << std::endl;
}

void Producer::sendMsg(const Message& msg) {

  auto out = avro::memoryOutputStream();
  auto e = avro::binaryEncoder();
  e->init(*out);
  avro::encode(*e, msg);
  auto snapshot = avro::snapshot(*out);

  constexpr auto maxRetries = 5u;
  auto retriesCount{0u};
  bool success{false};

  while (!success && retriesCount < maxRetries) {
    success = sendMsgImpl(&snapshot->at(0), snapshot->size());
    ++retriesCount;
  }
  if (!success)
  {
    throw Error{"failed to send. Attempting to send it " +
                std::to_string(maxRetries) + " times"};
  }
}

bool Producer::sendMsgImpl(void *payload, std::size_t payloadSize) {

  auto err = impl->produce(
      /* Topic name */
      topic,
      /* Any Partition: the builtin partitioner will be
       * used to assign the message to a topic based
       * on the message key, or random partition if
       * the key is not set. */
      // RdKafka::Topic::PARTITION_UA,
      partition,
      /* Make a copy of the value */
      RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
      /* Value */
      payload,
      payloadSize,
      /* Key */
      nullptr,
      0,
      /* Timestamp (defaults to current time) */
      0,
      /* Message headers, if any */
      nullptr,
      /* Per-message opaque value passed to
       * delivery report */
      nullptr);

  if (err == RdKafka::ERR__QUEUE_FULL) {
    /* If the internal queue is full, wait for
     * messages to be delivered and then retry.
     * The internal queue represents both
     * messages to be sent and messages that have
     * been sent or failed, awaiting their
     * delivery report callback to be called.
     *
     * The internal queue is limited by the
     * configuration property
     * queue.buffering.max.messages and queue.buffering.max.kbytes */
    impl->poll(1000 /*block for max 1000ms*/);
    return false;

  } else if (err != RdKafka::ERR_NO_ERROR) {
    throw Error{"failed to produce msg: " + RdKafka::err2str(err)};
  }

  /* A producer application should continually serve
   * the delivery report queue by calling poll()
   * at frequent intervals.
   * Either put the poll call in your main loop, or in a
   * dedicated thread, or call it after every produce() call.
   * Just make sure that poll() is still called
   * during periods where you are not producing any messages
   * to make sure previously produced messages have their
   * delivery report callback served (and any other callbacks
   * you register). */
  impl->poll(0);
  return true;
}
} // namespace giggle
