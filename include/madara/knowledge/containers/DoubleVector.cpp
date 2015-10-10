#include "DoubleVector.h"
#include "madara/knowledge/ContextGuard.h"


madara::knowledge::containers::DoubleVector::DoubleVector (
  const KnowledgeUpdateSettings & settings,
  const std::string & delimiter)
  : BaseContainer ("", settings), context_ (0), delimiter_ (delimiter)
{
}
   
madara::knowledge::containers::DoubleVector::DoubleVector (
  const std::string & name,
  KnowledgeBase & knowledge,
  int size,
  bool delete_vars,
  const KnowledgeUpdateSettings & settings,
  const std::string & delimiter)
  : BaseContainer (name, settings), context_ (&(knowledge.get_context ())),
  delimiter_ (delimiter)
{
  size_ = get_size_ref ();
  resize (size, delete_vars);
}
  
madara::knowledge::containers::DoubleVector::DoubleVector (
  const std::string & name,
  Variables & knowledge,
  int size,
  bool delete_vars,
  const KnowledgeUpdateSettings & settings,
  const std::string & delimiter)
  : BaseContainer (name, settings), context_ (knowledge.get_context ()),
  delimiter_ (delimiter)
{
  size_ = get_size_ref ();
  resize (size, delete_vars);
}

madara::knowledge::containers::DoubleVector::DoubleVector (
  const DoubleVector & rhs)
: BaseContainer (rhs), context_ (rhs.context_),
  vector_ (rhs.vector_),
  delimiter_ (rhs.delimiter_),
  size_ (rhs.size_)
{
}


madara::knowledge::containers::DoubleVector::~DoubleVector ()
{

}
  
void
madara::knowledge::containers::DoubleVector::modify (void)
{
  if (context_ && name_ != "")
  {
    ContextGuard context_guard (*context_);
    for (size_t index = 0; index < vector_.size (); ++index)
      context_->mark_modified (vector_[index]);

    context_->mark_modified (size_);
  }
}

std::string
madara::knowledge::containers::DoubleVector::get_debug_info (void)
{
  std::stringstream result;

  result << "Double Vector: ";

  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    size_t elements = vector_.size ();

    result << this->name_;
    result << " [" << elements << "]";
    result << " = [";

    if (elements > 0)
    {
      result << context_->get (vector_[0]).to_string ();

      for (size_t index = 1; index < elements; ++index)
      {
        result << ", " << context_->get (vector_[index]).to_string ();
      }
    }

    result << "]";
  }

  return result.str ();
}

void
madara::knowledge::containers::DoubleVector::modify_ (void)
{
  modify ();
}

std::string
madara::knowledge::containers::DoubleVector::get_debug_info_ (void)
{
  return get_debug_info ();
}

madara::knowledge::containers::BaseContainer *
madara::knowledge::containers::DoubleVector::clone (void) const
{
  return new DoubleVector (*this);
}

void
madara::knowledge::containers::DoubleVector::modify (size_t index)
{
  if (context_ && name_ != "" && index < vector_.size ())
  {
    ContextGuard context_guard (*context_);
    context_->mark_modified (vector_[index]);
  }
}

void
madara::knowledge::containers::DoubleVector::operator= (
  const DoubleVector & rhs)
{
  if (this != &rhs)
  {
    Guard guard (mutex_), guard2 (rhs.mutex_);

    this->context_ = rhs.context_;
    this->name_ = rhs.name_;
    this->settings_ = rhs.settings_;
    this->size_ = rhs.size_;
    this->vector_ = rhs.vector_;
    this->delimiter_ = rhs.delimiter_;
  }
}

void
madara::knowledge::containers::DoubleVector::push_back (type value)
{
  if (context_ && name_ != "")
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    if (!size_.is_valid ())
    {
      size_ = get_size_ref ();
    }

    size_t i = size ();
    resize ((int)i + 1);
    set (i, value);
  }
}

madara::knowledge::VariableReference
madara::knowledge::containers::DoubleVector::get_size_ref (void)
{
  VariableReference ref;
  
  if (context_ && name_ != "")
  {
    KnowledgeUpdateSettings keep_local (true);
    std::stringstream buffer;

    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    buffer << name_;
    buffer << delimiter_;
    buffer << "size";

    ref = context_->get_ref (buffer.str (), keep_local);
  }

  return ref;
}

void
madara::knowledge::containers::DoubleVector::resize (
  int size, bool delete_vars)
{
  if (context_ && name_ != "")
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    if (!size_.is_valid ())
    {
      size_ = get_size_ref ();
    }

    if (size >= 0)
    {
      size_t old_size = vector_.size ();

      if (old_size != size)
      {
        vector_.resize (size);
        
        context_->set (size_, knowledge::KnowledgeRecord::Integer (size), settings_);

        if ((size_t)size > old_size)
        {
          for (; old_size < (size_t)size; ++old_size)
          {
            std::stringstream buffer;
            buffer << name_;
            buffer << delimiter_;
            buffer << old_size;
            vector_[old_size] = context_->get_ref (buffer.str (), settings_);
          }
        }
        else if (delete_vars)
        {
          for (; (size_t)size < old_size; ++size)
          {
            std::stringstream buffer;
            buffer << name_;
            buffer << delimiter_;
            buffer << size;

            context_->delete_variable (buffer.str (), settings_);
          }
        }
      }
    }
    else
    {
      // dynamically allocate size from the context
      size_t cur_size =
        (size_t) context_->get (size_, settings_).to_integer ();

      size_t old_size = vector_.size ();

      if (old_size != cur_size)
      {
        vector_.resize (cur_size);

        if (cur_size > old_size)
        {
          for (; old_size < (size_t)cur_size; ++old_size)
          {
            std::stringstream buffer;
            buffer << name_;
            buffer << delimiter_;
            buffer << old_size;
            vector_[old_size] = context_->get_ref (buffer.str (), settings_);
          }
        }
        else if (delete_vars)
        {
          for (; (size_t)cur_size < old_size; ++cur_size)
          {
            std::stringstream buffer;
            buffer << name_;
            buffer << delimiter_;
            buffer << cur_size;

            context_->delete_variable (buffer.str (), settings_);
          }
        }
      }
    }
  }
}

size_t
madara::knowledge::containers::DoubleVector::size (void) const
{
  Guard guard (mutex_);
  return vector_.size ();
}

void
madara::knowledge::containers::DoubleVector::set_name (
  const std::string & var_name,
  KnowledgeBase & knowledge, int size)
{
  if (context_ != &(knowledge.get_context ()) || name_ != var_name)
  {
    context_ = &(knowledge.get_context ());

    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    name_ = var_name;

    vector_.clear ();

    size_ = get_size_ref ();
    
    resize (size);
  }
}

void
madara::knowledge::containers::DoubleVector::set_name (
  const std::string & var_name,
  Variables & knowledge, int size)
{
  if (context_ != knowledge.get_context () || name_ != var_name)
  {
    context_ = knowledge.get_context ();

    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    name_ = var_name;

    vector_.clear ();
    resize (size);
  }
}

void
madara::knowledge::containers::DoubleVector::set_name (
  const std::string & var_name,
  ThreadSafeContext & knowledge, int size)
{
  if (context_ != &knowledge || name_ != var_name)
  {
    context_ = &knowledge;

    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    name_ = var_name;

    vector_.clear ();
    resize (size);
  }
}

void
madara::knowledge::containers::DoubleVector::set_delimiter (
const std::string & delimiter)
{
  delimiter_ = delimiter;
  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    vector_.clear ();
    resize (-1);
  }
}


std::string
madara::knowledge::containers::DoubleVector::get_delimiter (void)
{
  return delimiter_;
}

void
madara::knowledge::containers::DoubleVector::exchange (
  DoubleVector & other, bool refresh_keys, bool delete_keys)
{
  if (context_ && other.context_)
  {
    ContextGuard context_guard (*context_);
    ContextGuard other_context_guard (*other.context_);
    Guard guard (mutex_), guard2 (other.mutex_);

    if (refresh_keys)
    {
      other.resize ();
      this->resize ();
    }

    size_t other_size = other.vector_.size ();
    size_t this_size = this->vector_.size ();

    for (size_t i = 0; i < this_size; ++i)
    {
      // temp = this[i];
      knowledge::KnowledgeRecord temp = context_->get (this->vector_[i], settings_);
    
      if (i < other_size)
      {
        // this[i] = other[i];
        context_->set (this->vector_[i],
          context_->get (other.vector_[i], other.settings_),
          settings_);

        // other[i] = temp;
        other.context_->set (other.vector_[i], temp, other.settings_);
      }
      else
      {
        if (delete_keys)
        {
          std::stringstream buffer;
          buffer << this->name_;
          buffer << delimiter_;
          buffer << i;
          this->context_->delete_variable (buffer.str (), other.settings_);
        }
        else
        {
          knowledge::KnowledgeRecord zero;
          this->context_->set (this->vector_[i], zero, this->settings_);
        }

        {
          std::stringstream buffer;
          buffer << other.name_;
          buffer << delimiter_;
          buffer << i;

          // other[i] = temp;
          other.context_->set (buffer.str (), temp, other.settings_);
        }
      }

    }

    // copy the other vector's elements to this vector's location
    for (size_t i = this_size; i < other_size; ++i)
    {
      std::stringstream buffer;
      buffer << this->name_;
      buffer << delimiter_;
      buffer << i;
      context_->set (buffer.str (),
        other.context_->get (other.vector_[i],
        other.settings_), this->settings_);
    }

    // set the size appropriately
    this->context_->set (this->size_,
      knowledge::KnowledgeRecord::Integer (other_size), this->settings_);
    other.context_->set (other.size_,
      knowledge::KnowledgeRecord::Integer (this_size), other.settings_);

    if (refresh_keys)
    {
      this->resize (-1, true);
      other.resize (-1, true);
    }
  }
}

void
madara::knowledge::containers::DoubleVector::transfer_to (
  DoubleVector & other)
{
  if (context_ && other.context_)
  {
    ContextGuard context_guard (*context_);
    ContextGuard other_context_guard (*other.context_);
    Guard guard (mutex_);
    Guard guard2 (other.mutex_);

    size_t other_size = other.vector_.size ();
    size_t this_size = this->vector_.size ();

    size_t size = other_size + this_size;
    other.resize ((int)size);

    for (size_t i = 0, j = other_size; i < this_size; ++i, ++j)
    {
      other.context_->set (other.vector_[j], (*this)[i], other.settings_);
    }

    this->resize (0, true);
  }
}

void
madara::knowledge::containers::DoubleVector::copy_to (
  KnowledgeVector & target) const
{
  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    target.resize (vector_.size ());
    
    for (size_t i = 0; i < vector_.size (); ++i)
    {
      target[i].deep_copy ((*this)[i]);
    }
  }
}

void
madara::knowledge::containers::DoubleVector::copy_to (
std::vector <double> & target) const
{
  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    target.resize (vector_.size ());

    for (size_t i = 0; i < vector_.size (); ++i)
    {
      target[i] = (*this)[i];
    }
  }
}

madara::knowledge::containers::DoubleVector::type
madara::knowledge::containers::DoubleVector::operator[] (
  size_t index) const
{
  knowledge::KnowledgeRecord result;
  KnowledgeUpdateSettings keep_local (true);

  if (index < vector_.size () && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    result = context_->get (vector_[index], keep_local);
  }

  return result.to_double ();
}

madara::knowledge::KnowledgeRecord
madara::knowledge::containers::DoubleVector::to_record (
  size_t index) const
{
  knowledge::KnowledgeRecord result;
  KnowledgeUpdateSettings keep_local (true);

  if (index < vector_.size () && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    result = context_->get (vector_[index], keep_local);
  }

  return result;
}

madara::knowledge::KnowledgeRecord
madara::knowledge::containers::DoubleVector::to_record (void) const
{
  knowledge::KnowledgeRecord result;
  KnowledgeUpdateSettings keep_local (true);

  // if we have something to actually set
  if (vector_.size () > 0 && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    // set last element first so we're not constantly resizing
    result.set_index (vector_.size () - 1,
      context_->get (vector_[vector_.size () - 1], keep_local).to_double ());

    for (size_t i = 0; i < vector_.size () - 1; ++i)
    {
      result.set_index (i,
        context_->get (vector_[i], keep_local).to_double ());
    }
  }

  return result;
}

bool
madara::knowledge::containers::DoubleVector::exists (
  size_t index) const
{
  bool result (false);

  if (index < vector_.size () && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    result = context_->exists (vector_[index]);
  }

  return result;
}

int
madara::knowledge::containers::DoubleVector::set (
size_t index,
type value)
{
  int result = -1;

  if (index < vector_.size () && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    result = context_->set (vector_[index], value, settings_);
  }

  return result;
}


int
madara::knowledge::containers::DoubleVector::set (
const std::vector <type> & value)
{
  int result = -1;

  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    if (vector_.size () < value.size ())
      resize ((int)value.size (), false);

    for (size_t i = 0; i < value.size (); ++i)
    {
      context_->set (vector_[i], value[i], settings_);
    }

    result = 0;
  }

  return result;
}

int
madara::knowledge::containers::DoubleVector::set (
  size_t index,
  type value, 
  const KnowledgeUpdateSettings & settings)
{
  int result = -1;
  
  if (index < vector_.size () && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    result = context_->set (vector_[index], value, settings);
  }

  return result;
}


int
madara::knowledge::containers::DoubleVector::set (
  const std::vector <type> & value,
  const KnowledgeUpdateSettings & settings)
{
  int result = -1;
  
  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    if (vector_.size () < value.size ())
      resize ((int)value.size (), false);

    for (size_t i = 0; i < value.size (); ++i)
    {
      context_->set (vector_[i], value[i], settings);
    }

    result = 0;
  }
  
  return result;
}

void
madara::knowledge::containers::DoubleVector::set_quality (
  size_t index,
  uint32_t quality,
  const KnowledgeReferenceSettings & settings)
{
  if (index < vector_.size () && context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);
    context_->set_quality (vector_[index].get_name (), quality,
      true, settings);
  }
}

bool
madara::knowledge::containers::DoubleVector::is_true (void) const
{
  bool result (false);

  madara_logger_log (context_->get_logger (), logger::LOG_MAJOR,
    "DoubleVector::is_true: Checking for truth\n");

  if (context_)
  {
    ContextGuard context_guard (*context_);
    Guard guard (mutex_);

    result = true;

    madara_logger_log (context_->get_logger (), logger::LOG_MINOR,
      "DoubleVector::is_true: context was not null. Result changed to %d\n",
      (int)result);

    for (size_t index = 0; index < vector_.size (); ++index)
    {

      madara_logger_log (context_->get_logger (), logger::LOG_DETAILED,
        "DoubleVector::is_true: checking index %d, is_false of %d. \n",
        (int)result, (int)context_->get (vector_[index]).is_false ());

      if (context_->get (vector_[index]).is_false ())
      {
        madara_logger_log (context_->get_logger (), logger::LOG_MAJOR,
          "DoubleVector::is_true: result is false, breaking\n");

        result = false;
        break;
      }
    }

    if (vector_.size () == 0)
      result = false;
  }

  madara_logger_log (context_->get_logger (), logger::LOG_MAJOR,
    "DoubleVector::is_true: final result is %d\n", (int)result);

  return result;
}

bool
madara::knowledge::containers::DoubleVector::is_false (void) const
{
  return !is_true ();
}


bool
madara::knowledge::containers::DoubleVector::is_true_ (void) const
{
  return is_true ();
}

bool
madara::knowledge::containers::DoubleVector::is_false_ (void) const
{
  return is_false ();
}
