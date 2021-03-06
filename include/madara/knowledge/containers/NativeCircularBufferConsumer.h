#ifndef _MADARA_KNOWLEDGE_CONTAINERS_CIRCULARBUFFERCONSUMER_H_
#define _MADARA_KNOWLEDGE_CONTAINERS_CIRCULARBUFFERCONSUMER_H_

#include <vector>
#include <string>
#include "madara/LockType.h"
#include "madara/knowledge/KnowledgeBase.h"
#include "madara/knowledge/ThreadSafeContext.h"
#include "madara/knowledge/KnowledgeUpdateSettings.h"
#include "Vector.h"
#include "Integer.h"

/**
 * @file NativeCircularBufferConsumer.h
 * @author David Kyle <david.kyle@shield.ai>
 * @author James Edmondson <jedmondson@gmail.com>
 *
 * This file provides an interface similar to CircularBufferConsumer, which
 * uses the internal history buffer of KnowledgeRecord. This allows threads
 * sharing the same KnowledgeBase to view the same circular buffer, but this
 * buffer is not visible to other KnowledgeBases across transports.
 **/

namespace madara { namespace knowledge { namespace containers {

/**
 * @class NativeCircularBufferConsumer
 * @brief This class provides an interface similar to CircularBufferConsumer,
 * which uses the internal history buffer of KnowledgeRecord. This allows
 * threads sharing the same KnowledgeBase to view the same circular
 * buffer, but this buffer is not visible to other KnowledgeBases across
 * transports.
 *
 * This class uses a local index into the underlying circular buffer, and
 * is meant for one-time access of buffer elements.
 *
 * This class provides a subset of the capabilities of CircularBufferConsumer,
 * as KnowledgeRecord itself provides equivalents of the rest. Use get_record()
 * to access the KnowledgeRecord backing a NativeCircularBufferConsumer.
 */
class MADARA_EXPORT NativeCircularBufferConsumer
{
public:
  /**
   * Default constructor
   **/
  NativeCircularBufferConsumer ();

  /**
   * Constructor
   * @param  name       name of the integer in the knowledge base
   * @param  knowledge  the knowledge base that will contain the vector
   * @param  settings   settings for evaluating the vector
   * @throw exceptions::NameException  bad name ("")
   **/
  NativeCircularBufferConsumer (const std::string & name,
          KnowledgeBase & knowledge);

  /**
   * Constructor
   * @param  name      the name of the map within the variable context
   * @param  knowledge the variable context
   * @param  settings  settings to apply by default
   * @throw exceptions::NameException  bad name ("")
   **/
  NativeCircularBufferConsumer (const std::string & name,
          Variables & knowledge);

  /**
   * Destructor
   **/
  virtual ~NativeCircularBufferConsumer () = default;

  /**
   * Returns the name of the variable
   * @return name of the variable
   **/
  std::string get_name (void) const;

  /**
   * Sets the variable name that this refers to
   * @param name  the name of the variable in the knowledge base
   * @param knowledge  the knowledge base the variable is housed in
   * @throw exceptions::NameException  bad name ("")
   **/
  void set_name (const std::string & name,
    KnowledgeBase & knowledge);

  /**
   * Sets the variable name that this refers to
   * @param name  the name of the variable in the knowledge base
   * @param knowledge  the knowledge base the variable is housed in
   * @throw exceptions::NameException  bad name ("")
   **/
  void set_name (const std::string & name,
    Variables & knowledge);

  /**
   * Sets the variable name that this refers to
   * @param name  the name of the variable in the knowledge base
   * @param knowledge  the ThreadSafeContext the variable is housed in
   * @throw exceptions::NameException  bad name ("")
   **/
  void set_name (const std::string & name,
    ThreadSafeContext & context);

  /**
   * Checks for equality
   * @param  value  the value to compare to
   * @return true if equal, false otherwise
   **/
  bool operator== (const NativeCircularBufferConsumer & value) const;

  /**
   * Checks for inequality
   * @param  value  the value to compare to
   * @return true if inequal, false otherwise
   **/
  bool operator!= (const NativeCircularBufferConsumer & value) const;

  /**
   * Consumes the record at the local index (not the producer index)
   * @return the last added record. exists() will return false if the
   *         record is invalid
   * @throw exceptions::IndexException  buffer has zero size
   **/
  KnowledgeRecord consume (void) const;

  /**
   * Consumes the record at the local index (not the producer index)
   * @return the last added record. exists() will return false if the
   *         record is invalid
   * @param  dropped the number of dropped packets. Drops can
   *                 occur when the producer produces faster than the
   *                 consumer can consume. This value is essentially
   *                 index_ - local_index - size ().
   * @throw exceptions::IndexException  buffer has zero size
   **/
  KnowledgeRecord consume (size_t & dropped) const;

  /**
   * Consumes the record at the local index (not the producer index)
   * @param value  the last added value.
   * @throw exceptions::IndexException  if no data to consume
   **/
  template <typename T>
  void consume (T & value) const;

  /**
   * Consumes the record at the local index (not the producer index)
   * @param value   the last added value.
   * @param dropped the number of dropped packets. Drops can
   *                occur when the producer produces faster than the
   *                consumer can consume. This value is essentially
   *                index_ - local_index - size ().
   * @throw exceptions::IndexException  if no data to consume
   **/
  template <typename T>
  void consume (T & value, size_t & dropped) const;

  /**
   * Returns the number of known drops since last consume
   * @return the number of drops
   **/
  size_t get_dropped (void) const;

  /**
   * Returns the number of records remaining that have not been consumed.
   * This includes records which have been dropped.
   * @return the number of records remaining for consume
   * @throw exceptions::ContextException  if name or context haven't
   *                      been set appropriately
   **/
  size_t remaining (void) const;

  /**
   * Returns the number of records in the NativeCircularBufferConsumer
   * @return the number of records in the NativeCircularBufferConsumer
   * @throw exceptions::ContextException  if name or context haven't
   *                      been set appropriately
   **/
  size_t count (void) const;

  /**
   * Returns the maximum size of the NativeCircularBufferConsumer
   * @return the size of the NativeCircularBufferConsumer
   **/
  size_t size (void) const;

  /**
   * Resizes the buffer size to the producer's buffer size
   **/
  void resize (void);

  /**
   * Gets the local index.
   * @param  index   the new index to use
   **/
  size_t get_index () { return local_index_; }

  /**
   * Sets the local index to an arbitrary position.
   * @param  index   the new index to use
   **/
  void set_index (size_t index);

  /**
   * Get the KnowledgeRecord this container refers to. While this returns by
   * copy, it will share the same circular buffer for read operations, but
   * any modificatiosn will result in a copy and not be reflected in the
   * original inside the KnowledgeBase.
   **/
  KnowledgeRecord get_record () const;

private:
  /**
   * Variable context that we are modifying
   **/
  mutable ThreadSafeContext * context_;

  /**
   * Reference to underlying record we are reading
   **/
  VariableReference ref_;

  /**
   * Index for latest item read by  
   **/
  mutable size_t local_index_;

  // Call to throw if preconditions aren't met
  static void check_name (const char * func, const char *name);
  void check_context (const char * func) const;
};

} } }


#include "NativeCircularBufferConsumer.inl"

#endif // _MADARA_KNOWLEDGE_CONTAINERS_CIRCULARBUFFERCONSUMER_H_
