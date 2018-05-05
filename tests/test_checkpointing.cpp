

#include "madara/knowledge/FileHeader.h"
#include "madara/knowledge/ThreadSafeContext.h"
#include "madara/knowledge/KnowledgeBase.h"
#include "madara/logger/GlobalLogger.h"

#include "madara/utility/Utility.h"

#include "madara/filters/ssl/AESBufferFilter.h"
#include "madara/knowledge/containers/Integer.h"

#include <stdio.h>
#include <iostream>
#include <string.h>

namespace logger = madara::logger;
namespace knowledge = madara::knowledge;
namespace filters = madara::filters;
namespace utility = madara::utility;
namespace containers = knowledge::containers;

#define BUFFER_SIZE    1000

void test_checkpointing (void)
{
  std::cerr << "\n*********** CHECKPOINTING TO FILE *************.\n";

  std::cerr << "Test 1: creating a 50k knowledge record.\n";

  madara::knowledge::ThreadSafeContext context;
  madara::knowledge::ThreadSafeContext context_copy;
  context.read_file ("file_var", madara::utility::expand_envs (
      "$(MADARA_ROOT)/tests/images/manaus_hotel_225x375.jpg"));

  context.set ("int_var", madara::knowledge::KnowledgeRecord::Integer (15));
  context.set ("str_var", std::string ("some string"));
  context.set ("double_var", 3.14159);

  std::vector <madara::knowledge::KnowledgeRecord::Integer> integers;
  std::vector <double> doubles;
  std::string id = "orig_context";

  integers.push_back (10);
  integers.push_back (20);
  integers.push_back (30);

  doubles.push_back (10.1);
  doubles.push_back (20.2);
  doubles.push_back (30.3);
  doubles.push_back (40.4);

  context.set ("int_array", integers);
  context.set ("double_array", doubles);

  context.print (0);

  knowledge::KnowledgeRecord::set_precision (1);
  
  std::cerr << "Test 1: saving 50k knowledge record to test1_orig.kb.\n";

  context.save_as_karl ("test1_orig.karl");
  context.save_context ("test1_orig.kb", "test_context");

  std::cerr << "Test 2: loading test1_orig.kb.\n";

  context_copy.clear ();
  context_copy.load_context ("test1_orig.kb", id);

  std::cerr << "Test 2: loaded context " << id << "\n";

  context_copy.print (0);

  std::cerr << "Test 2: Results were ";
  if (context_copy.get ("int_var") == 15
      && context_copy.get ("double_var") == 3.14159
      && context_copy.get ("str_var") == "some string"
      && context_copy.get ("int_array").to_string (", ") == "10, 20, 30"
      && context_copy.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && context_copy.get ("file_var").size () > 40000)
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";

  std::cerr << "Test 3: reading 500 KB file to force buffer resize\n";

  context_copy.read_file ("file_var", madara::utility::expand_envs (
      "$(MADARA_ROOT)/tests/images/manaus_hotel_900x1500.jpg"));
  
  context_copy.print (0);
  
  std::cerr << "Test 3: saving context\n";

  context_copy.save_as_karl ("test3_bigger.karl");
  context_copy.save_context ("test3_bigger.kb", id);
  
  std::cerr << "Test 3: loading context\n";
  
  context_copy.clear ();
  context_copy.load_context ("test3_bigger.kb", id);
  
  std::cerr << "Test 3: Results were ";
  if (context_copy.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && context_copy.get ("double_var") == 3.14159
      && context_copy.get ("str_var") == "some string"
      && context_copy.get ("int_array").to_string (", ") == "10, 20, 30"
      && context_copy.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && context_copy.get ("file_var").size () > 500000)
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";

  context_copy.print (0);
  
  std::cerr << "Test 3: saving context\n";

  context_copy.save_as_karl ("test4_resave.karl");
  context_copy.save_context ("test4_resave.kb", id);
  
  std::cerr << "Test 4: loading context into a knowledge base\n";
  
  madara::knowledge::KnowledgeBase knowledge;
  knowledge.load_context ("test4_resave.kb", false);
  
  std::cerr << "Test 4: Results were ";
  if (knowledge.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && knowledge.get ("double_var") == 3.14159
      && knowledge.get ("str_var") == "some string"
      && knowledge.get ("int_array").to_string (", ") == "10, 20, 30"
      && knowledge.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && knowledge.get ("file_var").size () > 500000)
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";
  
  knowledge.print ();
  
  std::cerr << "Test 5: Adding extra_var to context and saving\n";

  context_copy.save_as_karl ("test4_bigger.karl");
  context_copy.save_context ("test4_bigger.kb", "Extra Var Context");

  context_copy.set ("extra_var", 5.0,
    madara::knowledge::KnowledgeUpdateSettings (false, false, false, false));

  context_copy.save_checkpoint ("test4_bigger.kb", "Extra Var Context");
  
  std::cerr << "Test 5: Loading new context\n";
  
  knowledge.clear ();
  knowledge.load_context ("test4_bigger.kb", true);

  std::cerr << "Test 5: Results were ";
  if (knowledge.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && knowledge.get ("double_var") == 3.14159
      && knowledge.get ("str_var") == "some string"
      && knowledge.get ("int_array").to_string (", ") == "10, 20, 30"
      && knowledge.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && knowledge.get ("file_var").size () > 500000
      && knowledge.get ("extra_var") == 5.0)
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";
  
  knowledge.print ();

  knowledge.set ("additional_var", 6.0,
    madara::knowledge::EvalSettings (true, false, false, false, false));
  
  knowledge.save_checkpoint ("test4_bigger.kb", "Additional Var Context");
  
  std::cerr << "Test 6: Loading new context\n";
  
  knowledge.clear ();
  knowledge.load_context ("test4_bigger.kb", true);

  std::cerr << "Test 6: Results were ";
  if (knowledge.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && knowledge.get ("double_var") == 3.14159
      && knowledge.get ("str_var") == "some string"
      && knowledge.get ("int_array").to_string (", ") == "10, 20, 30"
      && knowledge.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && knowledge.get ("file_var").size () > 500000
      && knowledge.get ("extra_var") == 5.0
      && knowledge.get ("additional_var") == 6.0)
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";
  
  knowledge.print ();

  
  std::cerr << "Test 7: Adding local variable without track local changes\n";
  knowledge.set (".invisible", "tee hee");
  
  std::cerr << "Test 7: Printing .invisible value\n";
  knowledge.print (".invisible = {.invisible}\n");
  knowledge.save_checkpoint ("test4_bigger.kb", "Invisible Var Context");
  
  std::cerr << "Test 7: Loading checkpoint\n";
  knowledge.clear ();
  knowledge.load_context ("test4_bigger.kb", true);
  
  std::cerr << "Test 7: Printing .invisible value (should be 0)\n";
  knowledge.print (".invisible = {.invisible}\n");
  
  std::cerr << "Test 7: Results were ";
  if (knowledge.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && knowledge.get ("double_var") == 3.14159
      && knowledge.get ("str_var") == "some string"
      && knowledge.get ("int_array").to_string (", ") == "10, 20, 30"
      && knowledge.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && knowledge.get ("file_var").size () > 500000
      && knowledge.get ("extra_var") == 5.0
      && knowledge.get ("additional_var") == 6.0
      && knowledge.get (".invisible").status ()
           == madara::knowledge::KnowledgeRecord::UNCREATED)
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";
  
  
  std::cerr << "Test 8: Adding local variable with track local changes\n";
  knowledge.set (".invisible", "tee hee",
    madara::knowledge::EvalSettings (false, false, false, false, false, true));
  
  
  std::cerr << "Test 8: Printing .invisible value\n";
  knowledge.print (".invisible = {.invisible}\n");
  knowledge.save_as_karl ("test4_bigger.karl");
  knowledge.save_checkpoint ("test4_bigger.kb", "Invisible Var Context");
  
  std::cerr << "Test 8: Loading checkpoint\n";
  knowledge.clear ();
  knowledge.load_context ("test4_bigger.kb", true);
  
  std::cerr << "Test 8: Printing .invisible value (should be 'tee hee')\n";
  knowledge.print (".invisible = {.invisible}\n");
  std::cerr << "Test 8: Results were ";
  if (knowledge.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && knowledge.get ("double_var") == 3.14159
      && knowledge.get ("str_var") == "some string"
      && knowledge.get ("int_array").to_string (", ") == "10, 20, 30"
      && knowledge.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4"
      && knowledge.get ("file_var").size () > 500000
      && knowledge.get ("extra_var") == 5.0
      && knowledge.get ("additional_var") == 6.0
      && knowledge.get (".invisible") == "tee hee")
  {
    std::cerr << "SUCCESS\n\n";
  }
  else
    std::cerr << "FAIL\n\n";
}

void test_checkpoint_settings (void)
{

  std::cerr << "\n*********** TEST CHECKPOINT SETTINGS *************.\n";

  knowledge::KnowledgeRecord::set_precision (1);
  
  knowledge::CheckpointSettings settings;
  knowledge::KnowledgeBase saver, loader;


  saver.set ("int_var", knowledge::KnowledgeRecord::Integer (15));
  saver.set ("str_var", std::string ("some string"));
  saver.set ("double_var", 3.14159);

  std::vector <knowledge::KnowledgeRecord::Integer> integers;
  std::vector <double> doubles;
  std::string id = "orig_context";

  integers.push_back (10);
  integers.push_back (20);
  integers.push_back (30);

  doubles.push_back (10.1);
  doubles.push_back (20.2);
  doubles.push_back (30.3);
  doubles.push_back (40.4);

  saver.set ("int_array", integers);
  saver.set ("double_array", doubles);

  // set the system lamport clock to something arbitrary
  saver.evaluate ("#set_clock (2001)");

  settings.filename = "test_context_save_1.kb";
  settings.clear_knowledge = true;

  saver.save_context (settings);
  
  loader.load_context (settings);

  std::cerr << "Test 1: Loading a context and checking values: ";
  if (loader.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && loader.get ("double_var") == 3.14159
      && loader.get ("str_var") == "some string"
      && loader.get ("int_array").to_string (", ") == "10, 20, 30"
      && loader.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4")
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 2: Checking expected settings with last load: ";
  if (settings.initial_lamport_clock == 2001 &&
      settings.last_lamport_clock == 2001)
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL\n";
    std::cerr << "  Expected clocks (2001:2001) but got (" <<
      settings.initial_lamport_clock << ":" << settings.last_lamport_clock <<
      ")\n";
  }

  

  settings.prefixes.push_back ("int");
  settings.initial_timestamp = 1234;
  settings.last_timestamp = 4321;
  settings.override_timestamp = true;
  settings.override_lamport = true;
  settings.initial_lamport_clock = 7;
  settings.last_lamport_clock = 7;
  settings.filename = "test_context_save_2.kb";

  //logger::global_logger->set_level (logger::LOG_MINOR);

  saver.save_context (settings);

  loader.load_context (settings);

  std::cerr << "Test 3: Loading and checking for settings restrictions: ";
  if (loader.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && loader.get ("double_var").is_false ()
      && loader.get ("str_var").is_false ()
      && loader.get ("int_array").to_string (", ") == "10, 20, 30"
      && loader.get ("double_array").is_false ()
      && loader.get ("file_var").is_false ()
      && loader.get ("extra_var").is_false ()
      && loader.get ("additional_var").is_false ()
      && loader.get (".invisible").is_false ())
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 4: Checking expected settings with last load: ";
  if (settings.initial_lamport_clock == 7 &&
      settings.last_lamport_clock == 7 &&
      settings.initial_timestamp == 1234 &&
      settings.last_timestamp == 4321)
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL\n";
    std::cerr << "  Expected clocks (7:7) but got (" <<
      settings.initial_lamport_clock << ":" << settings.last_lamport_clock <<
      ")\n";
    std::cerr << "  Expected timestamps (1234:4321) but got (" <<
      settings.initial_timestamp << ":" << settings.last_timestamp <<
      ")\n";
  }

#ifdef _USE_SSL_

  filters::AESBufferFilter filter;
  filter.generate_key ("testPassword#tofile359");

  std::string test_message = "my test message";
  unsigned char buffer[200000];
  buffer[63] = 0;

  // copy current message into the buffer
  strncpy ((char *)buffer, test_message.c_str (), test_message.size () + 1);

  int encode_length (0), decode_length (0);

  std::cerr << "Test 5.1: buffer = : " << buffer << "\n";

  encode_length = filter.encode (buffer, (int)test_message.size () + 1, 63);

  std::cerr << "Test 5.1: Attempting 256bit AES on simple message: ";

  if (strncmp ((char *)buffer, test_message.c_str (), 63) != 0)
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Should be garbled but is the same. " << buffer << "\n";
  }

  std::cerr << "Test 5.2: buffer = : " << buffer << "\n";

  decode_length = filter.decode (buffer, encode_length, 64);

  std::cerr << "Test 5.2: Looking at unencoded message: ";

  if (strncmp ((char *)buffer, test_message.c_str (), 63) == 0)
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Should be same but is garbled. " << buffer << "\n";
  }

  std::cerr << "Test 5.3: encoding README.txt: ";

  test_message = utility::file_to_string (
    utility::expand_envs ("$(MADARA_ROOT)/README.txt"));
  strncpy ((char *)buffer, test_message.c_str (), test_message.size ());
  
  encode_length = filter.encode (buffer, (int)test_message.size (), sizeof(buffer));

  utility::write_file ("README_encrypted.bin", (void *)buffer, encode_length);

  test_message = utility::file_to_string ("README_encrypted.bin");
  memcpy ((char *)buffer, test_message.c_str (), test_message.size ());
  
  decode_length = filter.decode (buffer, test_message.size (), sizeof(buffer));

  utility::write_file ("README_decrypted.txt", (void *)buffer, decode_length);

  std::cerr << "SUCCESS\n";

  settings.filename = "test_encryption_6.kb";
  settings.buffer_filters.push_back (&filter);
  settings.prefixes.clear ();

  saver.save_context (settings);

  loader.load_context (settings);

  std::cerr << "Test 6: Saving and loading with 256 bit AES: ";
  if (loader.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && loader.get ("double_var") == 3.14159
      && loader.get ("str_var") == "some string"
      && loader.get ("int_array").to_string (", ") == "10, 20, 30"
      && loader.get ("double_array").to_string (", ") ==
          "10.1, 20.2, 30.3, 40.4")
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 7: Loading without 256 bit AES filter: ";
  settings.filename = "test_encryption_7.kb";
  
  saver.save_context (settings);

  settings.buffer_filters.clear ();

  loader.load_context (settings);

  if (loader.get ("int_var").is_false ()
      && loader.get ("double_var").is_false ()
      && loader.get ("str_var").is_false ()
      && loader.get ("int_array").is_false ()
      && loader.get ("double_array").is_false ()
      && loader.get ("file_var").is_false ()
      && loader.get ("extra_var").is_false ()
      && loader.get ("additional_var").is_false ()
      && loader.get (".invisible").is_false ())
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL Knowledge was:\n";
    loader.print ();
  }

  
  std::cerr << "Test 8: Saving with prefixes 256 bit AES filter: ";

  settings.filename = "test_encryption_8.kb";
  settings.prefixes.push_back ("int");
  settings.buffer_filters.push_back (&filter);
  
  saver.save_context (settings);

  loader.load_context (settings);


  if (loader.get ("int_var") == madara::knowledge::KnowledgeRecord::Integer (15)
      && loader.get ("double_var").is_false ()
      && loader.get ("str_var").is_false ()
      && loader.get ("int_array").to_string (", ") == "10, 20, 30"
      && loader.get ("double_array").is_false ()
      && loader.get ("file_var").is_false ()
      && loader.get ("extra_var").is_false ()
      && loader.get ("additional_var").is_false ()
      && loader.get (".invisible").is_false ())
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }


#endif

}

void test_checkpoints_diff (void)
{
  std::cerr << "\n*********** TEST CHECKPOINT DIFFS *************.\n";

  knowledge::KnowledgeRecord::set_precision (1);
  
  knowledge::VariableReferences current_modifieds;
  knowledge::CheckpointSettings checkpoint_settings;
  knowledge::KnowledgeBase saver, loader;
  knowledge::KnowledgeUpdateSettings track_changes;
  track_changes.track_local_changes = true;

  // make sure all checkpoint containers use checkpoint tracking
  containers::Integer
    var1 (".1", saver, track_changes),
    var2 (".2", saver, track_changes),
    var3 (".3", saver, track_changes),
    varglob1 ("var1", saver, track_changes),
    varglob2 ("var2", saver, track_changes),
    varglob3 ("var3", saver, track_changes),
    untracked ("var4", saver);

  // reset the checkpoint on save and on load, clear knowledge
  checkpoint_settings.clear_knowledge = true;
  checkpoint_settings.reset_checkpoint = true;
  checkpoint_settings.filename = "test_checkpoints1.kb";

  std::cerr << "Test 1: Saving .1 and var1: ";

  // modify the two vars with the track_changes update settings
  var1 = 1;
  varglob1 = 1;
  untracked = 4;

  //logger::global_logger->set_level (logger::LOG_MINOR);

  saver.save_checkpoint (checkpoint_settings);

  loader.load_context (checkpoint_settings);

  if (loader.get ("var1").to_integer () == 1 &&
      loader.get (".1").to_integer () == 1 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 2: Modify var1 and reload: ";

  varglob1 = 2;
  untracked = 4;

  saver.save_checkpoint (checkpoint_settings);

  loader.load_context (checkpoint_settings);

  if (loader.get ("var1").to_integer () == 2 &&
      loader.get (".1").to_integer () == 1 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 3: Change all values and save to new file: ";

  checkpoint_settings.filename = "test_checkpoints2.kb";
  var1 = 1;
  varglob1 = 1;
  var2 = 2;
  varglob2 = 2;
  var3 = 3;
  varglob3 = 3;
  untracked = 4;

  saver.save_checkpoint (checkpoint_settings);

  loader.load_context (checkpoint_settings);

  current_modifieds = saver.save_modifieds ();

  if (loader.get ("var1").to_integer () == 1 &&
      loader.get (".1").to_integer () == 1 &&
      loader.get ("var2").to_integer () == 2 &&
      loader.get (".2").to_integer () == 2 &&
      loader.get ("var3").to_integer () == 3 &&
      loader.get (".3").to_integer () == 3 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 4: Increment a variable to each checkpoint: ";

  checkpoint_settings.filename = "test_checkpoints_stack.kb";

  var1 = 1;

  saver.save_checkpoint (checkpoint_settings);

  var1 = 2;

  saver.save_checkpoint (checkpoint_settings);

  var1 = 3;
  varglob1 = 3;

  saver.save_checkpoint (checkpoint_settings);

  var1 = 4;
  varglob1 = 4;

  saver.save_checkpoint (checkpoint_settings);

  var1 = 5;
  varglob1 = 5;

  saver.save_checkpoint (checkpoint_settings);

  var1 = 6;
  varglob1 = 0;

  saver.save_checkpoint (checkpoint_settings);

  loader.load_context (checkpoint_settings);

  if (loader.get (".1").to_integer () == 6 &&
      loader.get ("var1").to_integer () == 0 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 5: Loading range 0-0 from stack: ";

  checkpoint_settings.initial_state = 0;
  checkpoint_settings.last_state = 0;

  loader.load_context (checkpoint_settings);

  if (loader.get (".1").to_integer () == 1 &&
      !loader.exists ("var1") &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 6: Loading range 2-2 from stack: ";

  checkpoint_settings.initial_state = 2;
  checkpoint_settings.last_state = 2;

  loader.load_context (checkpoint_settings);

  if (loader.get (".1").to_integer () == 3 &&
      loader.get ("var1").to_integer () == 3 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 7: Loading range 2-4 from stack: ";

  checkpoint_settings.initial_state = 2;
  checkpoint_settings.last_state = 4;

  loader.load_context (checkpoint_settings);

  if (loader.get (".1").to_integer () == 5 &&
      loader.get ("var1").to_integer () == 5 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

  std::cerr << "Test 8: Loading range 2-5 from stack: ";

  checkpoint_settings.initial_state = 2;
  checkpoint_settings.last_state = 5;

  loader.load_context (checkpoint_settings);

  if (loader.get (".1").to_integer () == 6 &&
      loader.get ("var1").to_integer () == 0 &&
      !loader.exists ("var4"))
  {
    std::cerr << "SUCCESS\n";
  }
  else
  {
    std::cerr << "FAIL. Knowledge was:\n";
    loader.print ();
  }

}

void handle_arguments (int argc, char ** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    std::string arg1 (argv[i]);
    if (arg1 == "-l" || arg1 == "--level")
    {
      if (i + 1 < argc)
      {
        int level;
        std::stringstream buffer (argv[i + 1]);
        buffer >> level;
        logger::global_logger->set_level (level);
      }

      ++i;
    }
    else
    {
      madara_logger_ptr_log (logger::global_logger.get(), logger::LOG_ALWAYS,
        "\nProgram summary for %s:\n\n" \
        "  Test the checkpointing functionality.\n\n" \
        " [-l|--level level]       the logger level (0+, higher is higher detail)\n" \
        "\n",
        argv[0]);
      exit (0);
    }
  }
}

int main (int argc, char * argv[])
{
  handle_arguments (argc, argv);

  // buffer for holding encoding results
  char buffer[BUFFER_SIZE];
  int64_t buffer_remaining;
  std::string key;
  bool header_decoded;
  
  std::cerr <<
    "\n*********** TESTING ENCODING FILE HEADER TO FILE *************.\n";


  // message headers for encoding and decoding
  madara::knowledge::FileHeader source_header;
  madara::knowledge::FileHeader dest_header;

  // Test 1: create the source header
  memset (buffer, 0, BUFFER_SIZE);
  source_header.karl_version += 10;
  source_header.last_timestamp = source_header.initial_timestamp + 100;
  source_header.states += 50;
  source_header.size += 50 * 32;
  buffer_remaining = BUFFER_SIZE;
  strncpy (source_header.file_type, "KaRL1.0\0", 8);
  strncpy (source_header.originator, "localhost:34000\0", 16);
  
  std::cerr << "Test 1: encoding source_header to buffer.\n";

  // Test 1: encode source header into the buffer
  source_header.write (buffer, buffer_remaining);
  
  std::cerr << "Test 1: encoded source_header to buffer. Bytes remaining=" << 
    buffer_remaining << std::endl;

  // Test 2: decode source header from buffer
  
  std::cerr << "Test 2: decoding dest_header from buffer.\n";

  buffer_remaining = BUFFER_SIZE - buffer_remaining;
  dest_header.read (buffer, buffer_remaining);
  buffer_remaining = BUFFER_SIZE - buffer_remaining;
  
  std::cerr << "Test 3: decoded dest_header from buffer. Bytes remaining=" << 
    buffer_remaining << std::endl;
  
  header_decoded = dest_header.equals (source_header);

  std::cerr << "Test 4: decoded dest_header is equal to source_header? " <<
    (header_decoded ? "true" : "false") << std::endl;
  
  std::cerr << "\nEncoding results were " << 
    (header_decoded ?
    "SUCCESS\n" : "FAIL\n");
  
  test_checkpointing ();

  test_checkpoint_settings ();

  test_checkpoints_diff ();

  return 0;
}
