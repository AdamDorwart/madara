#include "madara/transport/splice/Splice_DDS_Transport.h"
#include "madara/knowledge/Update_Types.h"
#include "madara/utility/Utility.h"
#include <time.h>

#include <iostream>
#include <sstream>
#include "madara/logger/Global_Logger.h"

namespace logger = Madara::Logger;

const char * madara::transport::Splice_DDS_Transport::topic_names_[] = {
  "MADARA_KaRL_Data",
  "MADARA_KaRL_Control"
};

/* Array to hold the names for all ReturnCodes. */
const char * madara::transport::Splice_DDS_Transport::ret_code_names[] = { 
    "DDS_RETCODE_OK",
    "DDS_RETCODE_ERROR",
    "DDS_RETCODE_UNSUPPORTED",
    "DDS_RETCODE_BAD_PARAMETER",
    "DDS_RETCODE_PRECONDITION_NOT_MET",
    "DDS_RETCODE_OUT_OF_RESOURCES",
    "DDS_RETCODE_NOT_ENABLED",
    "DDS_RETCODE_IMMUTABLE_POLICY",
    "DDS_RETCODE_INCONSISTENT_POLICY",
    "DDS_RETCODE_ALREADY_DELETED",
    "DDS_RETCODE_TIMEOUT",
    "DDS_RETCODE_NO_DATA",
    "DDS_RETCODE_ILLEGAL_OPERATION" };

const char * madara::transport::Splice_DDS_Transport::partition_ = "Madara_knowledge";

madara::transport::Splice_DDS_Transport::Splice_DDS_Transport (
  const std::string & id,
  knowledge::Thread_Safe_Context & context, 
  Settings & config, bool launch_transport)
  : madara::transport::Base (id, config, context), 
  domain_ (0), domain_factory_ (0), 
  domain_participant_ (0), publisher_ (0), subscriber_ (0), 
  datawriter_ (0), datareader_ (0), 
  update_writer_ (0), update_reader_ (0),
  update_topic_ (0), 
  thread_ (0)
  //dr_listener_ (id, context), sub_listener_ (id, context)
  //reliability_ (reliability), 
  //valid_setup_ (false),
  //data_topic_name_ (topic_names_[0]),
  //control_topic_name_ (topic_names_[1])
{
  if (launch_transport)
    setup ();
}
madara::transport::Splice_DDS_Transport::~Splice_DDS_Transport ()
{
  close ();
}

void
madara::transport::Splice_DDS_Transport::close (void)
{
  this->invalidate_transport ();

  //if (thread_)
  //{
  //  thread_->close ();
  //}

  //if (subscriber_.in ())
  //{
  //  subscriber_->delete_datareader (update_reader_);
  //}

  //if (publisher_.in ())
  //{
  //  publisher_->delete_datawriter (update_writer_);
  //  publisher_->delete_datawriter (latency_update_writer_);
  //}


  //if (domain_participant_.in ())
  //{
  //  domain_participant_->delete_subscriber (subscriber_);
  //  domain_participant_->delete_publisher (publisher_);
  //  domain_participant_->delete_topic (update_topic_);
  //}

  //if (domain_factory_.in ())
  //  domain_factory_->delete_participant (domain_participant_);

  thread_ = 0;
  update_reader_ = 0;
  update_writer_ = 0;
  update_topic_ = 0;
  subscriber_ = 0;
  publisher_ = 0;
  domain_participant_ = 0;
  domain_factory_ = 0;

  this->shutting_down_ = false;
}

int
madara::transport::Splice_DDS_Transport::reliability (void) const
{
  return this->settings_.reliability;
}

int
madara::transport::Splice_DDS_Transport::reliability (const int & setting)
{
  return this->settings_.reliability = setting;
}

int
madara::transport::Splice_DDS_Transport::setup (void)
{
  Base::setup ();
  DDS::ReturnCode_t                         status;

  this->is_valid_ = false;

  // reset the valid setup flag
  //valid_setup_ = false;

  context_.get_logger ().log (logger::LOG_DETAILED,
      "Splice_DDS_Transport::setup:" \
      " Creating a participant for topic (%s)\n", 
      Madara::Utility::dds_topicify (settings_.domains).c_str ());

  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Participant settings are being read from the OSPL_URI environment"
    " variable\n",
    Madara::Utility::dds_topicify (settings_.domains).c_str ());

  // get the domain participant factory
  domain_factory_ = DDS::DomainParticipantFactory::get_instance ();
  domain_factory_->get_default_participant_qos (part_qos_);
	domain_participant_ = domain_factory_->create_participant (
    domain_, 
    part_qos_, NULL, DDS::STATUS_MASK_NONE);

  // if dp == NULL, we've got an error
  if (domain_participant_ == NULL)
  {
    context_.get_logger ().log (logger::LOG_ERROR,
      "\nSplice_DDS_Transport::setup:" \
      " splice daemon not running. Try 'ospl start'...\n");

    exit (-2);
  }

  domain_participant_->get_default_topic_qos(topic_qos_);

  if (madara::transport::RELIABLE == this->settings_.reliability)
  {
    topic_qos_.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    topic_qos_.history.depth = this->settings_.queue_length;
    topic_qos_.resource_limits.max_samples_per_instance = 
      this->settings_.queue_length;
    topic_qos_.resource_limits.max_samples = this->settings_.queue_length;
    topic_qos_.destination_order.kind = 
      DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    //topic_qos_.
  }
  //topic_qos_.resource_limits.max_samples_per_instance= 10;
  domain_participant_->set_default_topic_qos(topic_qos_);


  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Registering type support\n");

  //  Register Update type
  status = this->update_type_support_.register_type (
    domain_participant_, "Knowledge::Update");
  check_status(status, "Knowledge::UpdateTypeSupport::register_type");

  //  Register Mutex type
  //status = this->mutex_type_support_.register_type (
  //  domain_participant_, "Knowledge::Mutex");
  //check_status(status, "Knowledge::MutexTypeSupport::register_type");

  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Setting up knowledge domain via topic (%s)\n",
    Madara::Utility::dds_topicify (settings_.domains).c_str ());

  // Create Update topic
  update_topic_ = domain_participant_->create_topic (
    Madara::Utility::dds_topicify (settings_.domains).c_str (), 
    "Knowledge::Update", 
    topic_qos_, NULL, DDS::STATUS_MASK_NONE);
  check_handle(update_topic_, 
    "DDS::DomainParticipant::create_topic (Knowledge_Update)");

  // Get default qos for publisher
  status = domain_participant_->get_default_publisher_qos (pub_qos_);
  check_status(status, "DDS::DomainParticipant::get_default_publisher_qos");


  if (madara::transport::RELIABLE == this->settings_.reliability)
  {
    pub_qos_.presentation.access_scope = DDS::TOPIC_PRESENTATION_QOS;
    pub_qos_.presentation.coherent_access = true;
    pub_qos_.presentation.ordered_access = false;
    //topic_qos_.
  }

  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Creating publisher for topic (%s)\n",
    Madara::Utility::dds_topicify (settings_.domains).c_str ());

  // Create publisher
  pub_qos_.partition.name.length (1);
  pub_qos_.partition.name[0] = DDS::string_dup (partition_);
  publisher_ = domain_participant_->create_publisher (
    pub_qos_, NULL, DDS::STATUS_MASK_NONE);
  check_handle(publisher_, "DDS::DomainParticipant::create_publisher");

  // Create subscriber
  status = domain_participant_->get_default_subscriber_qos (sub_qos_);
  check_status(status, "DDS::DomainParticipant::get_default_subscriber_qos");


  if (madara::transport::RELIABLE == this->settings_.reliability)
  {
    sub_qos_.presentation.access_scope = DDS::TOPIC_PRESENTATION_QOS;
    sub_qos_.presentation.coherent_access = true;
    sub_qos_.presentation.ordered_access = false;
  }

  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Creating subscriber for topic (%s)\n",
    Madara::Utility::dds_topicify (settings_.domains).c_str ());

  sub_qos_.partition.name.length (1);
  sub_qos_.partition.name[0] = DDS::string_dup (partition_);
  subscriber_ = domain_participant_->create_subscriber (
//    sub_qos_, &sub_listener_, DDS::DATA_AVAILABLE_STATUS | DDS::DATA_ON_READERS_STATUS);
    sub_qos_, NULL, DDS::STATUS_MASK_NONE);
  check_handle(subscriber_, "DDS::DomainParticipant::create_subscriber");

  if (!subscriber_ || !publisher_)
  {
    context_.get_logger ().log (logger::LOG_ERROR,
      "Splice_DDS_Transport::setup:" \
      " pub or sub could not be created. Try 'ospl stop; ospl start'...\n");

    exit (-2);
  }

  // Create datawriter
  publisher_->get_default_datawriter_qos (datawriter_qos_);
  publisher_->copy_from_topic_qos(datawriter_qos_, topic_qos_);

  if (madara::transport::RELIABLE == this->settings_.reliability)
  {
    context_.get_logger ().log (logger::LOG_DETAILED,
      "Splice_DDS_Transport::setup:" \
      " Enabling reliable transport for (%s) datawriters\n",
      Madara::Utility::dds_topicify (settings_.domains).c_str ());

    datawriter_qos_.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    datawriter_qos_.history.depth = this->settings_.queue_length;
    datawriter_qos_.resource_limits.max_samples = this->settings_.queue_length;
    datawriter_qos_.resource_limits.max_samples_per_instance = 
      this->settings_.queue_length;
    datawriter_qos_.destination_order.kind = 
      DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
  }
  else
  {
    context_.get_logger ().log (logger::LOG_DETAILED,
      "Splice_DDS_Transport::setup:" \
      " Enabling unreliable transport for (%s) datawriters\n",
      Madara::Utility::dds_topicify (settings_.domains).c_str ());
  }

  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Creating datawriter for topic (%s)\n",
    Madara::Utility::dds_topicify (settings_.domains).c_str ());

  // Create Update writer
  datawriter_ = publisher_->create_datawriter (update_topic_, 
    datawriter_qos_, NULL, DDS::STATUS_MASK_NONE);
  check_handle(datawriter_, "DDS::Publisher::create_datawriter (Update)");
  update_writer_ = dynamic_cast<Knowledge::UpdateDataWriter_ptr> (datawriter_.in ());
  check_handle(update_writer_, "Knowledge::UpdateDataWriter_ptr::narrow");

  // Create Latency Update writer for Read Thread
  latencywriter_ = publisher_->create_datawriter (update_topic_, 
    datawriter_qos_, NULL, DDS::STATUS_MASK_NONE);
  check_handle(latencywriter_, "DDS::Publisher::create_datawriter (Update)");
  latency_update_writer_ = dynamic_cast<Knowledge::UpdateDataWriter_ptr> (latencywriter_.in ());
  check_handle(latency_update_writer_, "Knowledge::UpdateDataWriter_ptr::narrow");


  // Create datareader
  status = subscriber_->get_default_datareader_qos (datareader_qos_);
  subscriber_->copy_from_topic_qos (datareader_qos_, topic_qos_);
  //publisher_->copy_from_topic_qos(datawriter_qos_, topic_qos_);
  check_status(status, "DDS::Subscriber::get_default_datareader_qos");

  datareader_qos_.reader_data_lifecycle.enable_invalid_samples = FALSE;

  if (madara::transport::RELIABLE == this->settings_.reliability)
  {
    context_.get_logger ().log (logger::LOG_DETAILED,
      "Splice_DDS_Transport::setup:" \
      " Enabling reliable transport for (%s) datareaders\n",
      Madara::Utility::dds_topicify (settings_.domains).c_str ());

    datareader_qos_.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
    datareader_qos_.history.depth = this->settings_.queue_length;
    datareader_qos_.resource_limits.max_samples = this->settings_.queue_length;
    datareader_qos_.destination_order.kind = 
      DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;

    // unlike the other qos, we do not set max_samples_per_instance here.
    // that shouldn't be as necessary, since we are using take on the reader
  }
  else
  {
    context_.get_logger ().log (logger::LOG_DETAILED,
      "Splice_DDS_Transport::setup:" \
      " Enabling unreliable transport for (%s) datareaders\n",
      Madara::Utility::dds_topicify (settings_.domains).c_str ());
  }

  context_.get_logger ().log (logger::LOG_DETAILED,
    "Splice_DDS_Transport::setup:" \
    " Creating datareader for topic (%s)\n",
    Madara::Utility::dds_topicify (settings_.domains).c_str ());

  // Create Update datareader
  datareader_ = subscriber_->create_datareader (update_topic_, 
    //datareader_qos_, &dr_listener_, DDS::STATUS_MASK_NONE);
    datareader_qos_, NULL, DDS::STATUS_MASK_NONE);

  // notes: we set the mask to none because the listener will be called
  // by the subscriber listener with notify_datareaders. This is the Splice
  // way of doing this, since we require subscription information and they
  // have so far not implemented on_subscription_matched.

  check_handle(datareader_, "DDS::Subscriber::create_datareader (Update)");
  update_reader_ = dynamic_cast<Knowledge::UpdateDataReader_ptr>(datareader_.in ());
  check_handle(update_reader_, "Knowledge::UpdateDataReader_ptr::narrow");

  // Create Mutex datareader
  //datareader_ = subscriber_->create_datareader (mutex_topic_, 
  //  datareader_qos_, NULL, DDS::STATUS_MASK_NONE);
  //check_handle(datareader_, "DDS::Subscriber::create_datareader (Mutex)");
  //mutex_reader_ = dynamic_cast<Knowledge::MutexDataReader_ptr>(datareader_);
  //check_handle(mutex_reader_, "Knowledge::MutexDataReader_ptr::narrow");  

  thread_ = new madara::transport::Splice_Read_Thread (id_, this->settings_,
    context_, update_reader_, latency_update_writer_, send_monitor_,
    receive_monitor_, packet_scheduler_);
  
  this->validate_transport ();

  return 0;
}

long
madara::transport::Splice_DDS_Transport::send_data (
  const Madara::Knowledge_Records & updates)
{
  long result =
    prep_send (updates, "Splice_DDS_Transport::send_data:");

  // get the maximum quality from the updates
  uint32_t quality = Madara::max_quality (updates);

  /// get current lamport clock. 
  unsigned long long cur_clock = context_.get_clock ();

  DDS::ReturnCode_t      dds_result;
  DDS::InstanceHandle_t  handle;

  Knowledge::Update data;
  
  data.buffer = Knowledge::seq_oct (result, result, (unsigned char *)buffer_.get_ptr ());
  data.clock = cur_clock;
  data.quality = quality;
  data.updates = DDS::ULong (updates.size ());
  data.originator = DDS::string_dup(id_.c_str ());
  data.type = madara::transport::MULTIASSIGN;
  data.ttl = settings_.get_rebroadcast_ttl ();
  data.timestamp = time (NULL);
  data.madara_id = DDS::string_dup(MADARA_IDENTIFIER);

  context_.get_logger ().log (logger::LOG_MAJOR,
    "Splice_DDS_Transport::send:" \
    " sending multiassignment: %d updates, time=llu, quality=%d\n",
    data.updates, cur_clock, quality);

  handle = update_writer_->register_instance (data);
  dds_result = update_writer_->write (data, handle); 
  //update_writer_->unregister_instance (data, handle);

  return dds_result;
}

#ifdef _USE_CID_

long
madara::transport::Splice_DDS_Transport::start_latency (void)
{
  // check to see if we are shutting down
  long ret = this->check_transport ();
  if (-1 == ret)
  {
    context_.get_logger ().log (logger::LOG_MAJOR,
      "Splice_DDS_Transport::start_latency:"
      " transport has been told to shutdown");
    return ret;
  }
  else if (-2 == ret)
  {
    context_.get_logger ().log (logger::LOG_MAJOR,
      "Splice_DDS_Transport::start_latency:"
      " transport is not valid");
    return ret;
  }

  /// update the clock 
  unsigned long long cur_clock = context_.inc_clock ();

  DDS::ReturnCode_t      dds_result;
  DDS::InstanceHandle_t  handle;

  Knowledge::Update data;
  data.key = "";
  data.value = 0;
  data.clock = cur_clock;
  data.quality = this->settings_.id;
  data.originator = id_.c_str ();
  data.type = madara::transport::LATENCY;

  context_.get_logger ().log (logger::LOG_MAJOR,
    "Splice_DDS_Transport::start_latency:" \
    " originator=%s, time=%llu\n", 
    id_.c_str (), cur_clock);

  settings_.reset_timers ();
  settings_.start_all_timers ();

  handle = update_writer_->register_instance (data);
  dds_result = update_writer_->write (data, handle); 
  //update_writer_->unregister_instance (data, handle);

  return dds_result;
}

long
madara::transport::Splice_DDS_Transport::vote (void)
{
  // check to see if we are shutting down
  long ret = this->check_transport ();
  if (-1 == ret)
  {
    context_.get_logger ().log (logger::LOG_MAJOR,
      "Splice_DDS_Transport::vote:"
      " transport has been told to shutdown");

    return ret;
  }
  else if (-2 == ret)
  {
    context_.get_logger ().log (logger::LOG_MAJOR,
      "Splice_DDS_Transport::vote:"
      " transport is not valid");
    return ret;
  }

  /// update the clock 
  unsigned long long cur_clock = context_.inc_clock ();

  DDS::ReturnCode_t      dds_result;
  DDS::InstanceHandle_t  handle;

  Madara::Cid::Algorithm_Results & results = settings_.latencies.results;

  if (results.size () == 0)
  {
    context_.get_logger ().log (logger::LOG_MAJOR,
      "Splice_DDS_Transport::vote:"
      " Unable to vote. No algorithm results present");

    return ret;
  }

  std::sort (results.begin (), results.end (),
    Madara::Cid::Increasing_Algorithm_Latency);

  Knowledge::Update data;
  data.key = results[0].deployment.c_str ();
  data.value = (long long) results[0].latency;
  data.clock = cur_clock;
  data.quality = this->settings_.id;
  data.originator = id_.c_str ();
  data.type = madara::transport::VOTE;

  context_.get_logger ().log (logger::LOG_MAJOR,
    "Splice_DDS_Transport::vote:" \
    " originator=%s, time=%llu, best=%llu\n", 
    id_.c_str (), cur_clock, results[0].latency);

  handle = update_writer_->register_instance (data);
  dds_result = update_writer_->write (data, handle); 
  //update_writer_->unregister_instance (data, handle);

  return dds_result;
}

#endif // #ifdef _USE_CID_


void
madara::transport::Splice_DDS_Transport::check_handle (void * handle, 
                                                      const char * info)
{
  if (!handle)
  {
    context_.get_logger ().log (logger::LOG_ERROR,
      "Splice_DDS_Transport::check_handle:" \
      " error in %s: Creation failed: invalid handle\n", info);

    exit (-2);
  }
}

void
madara::transport::Splice_DDS_Transport::check_status (DDS::ReturnCode_t status,
                                                       const char * info)
{
  // if the status is okay, then return without issue
  if ((status == DDS::RETCODE_OK) || (status == DDS::RETCODE_NO_DATA)) 
    return;

  context_.get_logger ().log (logger::LOG_ERROR,
    "Splice_DDS_Transport::check_status:" \
    " error in %s: Creation failed: %s\n",
    info, get_error_name (status));

  exit (-2);
}

const char *
madara::transport::Splice_DDS_Transport::get_error_name(DDS::ReturnCode_t status)
{
    return ret_code_names[status];
}
