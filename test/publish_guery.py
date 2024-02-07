from threading import Condition
import time

import fastdds
import Query_msg

def publish_query():
    # Create a DomainParticipant
    participant = fastdds.DomainParticipantFactory.get_instance().create_participant(0, fastdds.DomainParticipantQos())
    if participant is None:
        print("Participant creation failed!")
        return

    # Register the Type
    type_support = Query_msg.Query_msgPubSubType()
    type_support.register_type(participant)

    # Create Topic
    topic = participant.create_topic("AZD_query", "Query_msg", fastdds.DomainParticipant.TOPIC_QOS_DEFAULT)
    if topic is None:
        print("Topic creation failed!")
        return

    # Create Publisher
    publisher = participant.create_publisher(fastdds.DomainParticipant.PUBLISHER_QOS_DEFAULT, None)
    if publisher is None:
        print("Publisher creation failed!")
        return

    # Create DataWriter
    writer = publisher.create_datawriter(topic, fastdds.Publisher.DATAWRITER_QOS_DEFAULT, None)
    if writer is None:
        print("Writer creation failed!")
        return

    # Create and Send QueryMsg
    query = Query_msg.Query_msg()
    query.slave_id (2)
    query.func_code(3)  # Assuming 1 is the code for write
    query.write_addr(0)  # Example write address
    query.write_num(7)  # Number of registers to write
    query.data = [0,0,0,0,0,0,0]  # Example data to write

    writer.write(query)
    print("Query message published.")

    # Clean up
    participant.delete_contained_entities()
    fastdds.DomainParticipantFactory.get_instance().delete_participant(participant)

if __name__ == "__main__":
    publish_query()