/* -*- C++ -*- */
#ifndef _VARIABLE_DECREMENT_NODE_H_
#define _VARIABLE_DECREMENT_NODE_H_

#ifndef _MADARA_NO_KARL_

#include <string>
#include <vector>

#include "madara/expression_tree/Component_Node.h"
#include "madara/expression_tree/Variable_Node.h"
#include "madara/expression_tree/Composite_Array_Reference.h"
#include "madara/knowledge/Thread_Safe_Context.h"
#include "madara/knowledge/Knowledge_Record.h"

namespace madara
{
  namespace expression_tree
  {
    // Forward declarations.
    class Visitor;

    /**
     * @class Variable_Decrement_Node
     * @brief Composite node that subtracts a variable by some right hand side
     */

    class Variable_Decrement_Node : public Component_Node
    {
    public:
      /// Ctor.
      Variable_Decrement_Node (Component_Node * lhs,
        madara::Knowledge_Record value,
        Component_Node * rhs,
        madara::knowledge::Thread_Safe_Context &context);

      /// Dtor.
      virtual ~Variable_Decrement_Node (void);

      /// Return the item stored in the node.
      virtual madara::Knowledge_Record item (void) const;

      /// Prune the tree of unnecessary nodes. 
      /// Returns evaluation of the node and sets can_change appropriately.
      /// if this node can be changed, that means it shouldn't be pruned.
      virtual madara::Knowledge_Record prune (bool & can_change);

      /// Evaluates the node and its children. This does not prune any of
      /// the expression tree, and is much faster than the prune function
      virtual madara::Knowledge_Record evaluate (
        const madara::knowledge::Knowledge_Update_Settings & settings);

      /// Define the @a accept() operation used for the Visitor pattern.
      virtual void accept (Visitor &visitor) const; 

    private:
      /// variable holder
      Variable_Node * var_;

      /// variable index holder
      Composite_Array_Reference * array_;

      /// amount to increment by. Note that this can also do decrement.
      madara::Knowledge_Record value_;
      
      /// holds a right hand side argument if it is not value_
      Component_Node * rhs_;

      madara::knowledge::Thread_Safe_Context & context_;
    };
  }
}

#endif // _MADARA_NO_KARL_

#endif /* _VARIABLE_DECREMENT_NODE_H_ */
