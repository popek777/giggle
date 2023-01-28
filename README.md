# giggle
kafka and avro based messaging system

# background
show a use case for:
 - avro (serialization)
 - kafka (messaging)
 - conan (c++ packaging)
 
# install kafka docker image
https://developer.confluent.io/quickstart/kafka-docker/?build=apps#5-read-messages-from-the-topic

# start kafka broker
docker compose up -d

# create kafka topic
create topic with 2 partitions:

docker exec --interactive --tty broker \
kafka-console-consumer --bootstrap-server broker:9092 \
                       --topic quickstart \
                       --partitions 2 \
                       --from-beginning

# run build/install script

./run_package_steps.sh

# run 'chat service bot' on one terminal
  
  ./test_package/build/RelWithDebInfo/chat_gpt server
  
# run 'chat service client' on the other terminal
  
  ./test_package/build/RelWithDebInfo/chat_gpt client
  
  
