/* -*- C++ -*- */
#ifndef _COMPOSITE_MODULUS_NODE_H
#define _COMPOSITE_MODULUS_NODE_H

#ifndef _MADARA_NO_KARL_

#include "madara/expression_tree/Composite_Binary_Node.h"
#include "madara/knowledge/Knowledge_Record.h"

namespace madara
{
  namespace expression_tree
  {
    class Component_Node;
    class Visitor;

    /**
     * @class Composite_Modulus_Node
     * @brief A composite node that divides a left expression by a right
     *        expression and returns the remainder of the division
     */
    class Composite_Modulus_Node : public Composite_Binary_Node
    {
    public:
      /**
       * Constructor
       * @param   logger the logger to use for printing
       * @param   left   left expression
       * @param   right  right expression
       **/
      Composite_Modulus_Node (logger::Logger & logger,
        Component_Node *left,
        Component_Node *right);

      /**
       * Destructor
       **/
      virtual ~Composite_Modulus_Node (void);

      /**
       * Returns the printable character of the node
       * @return    value of the node
       **/
      virtual madara::Knowledge_Record item (void) const;

      /** 
       * Prunes the expression tree of unnecessary nodes. 
       * @param     can_change   set to true if variable nodes are contained
       * @return    remainder of left expression divided by right expression.
       *            If right expression is zero, then a compile error is 
       *            logged and the process exits.
       **/
      virtual madara::Knowledge_Record prune (bool & can_change);

      /** 
       * Evaluates the expression tree. 
       * @param     settings     settings for evaluating the node
       * @return    remainder of left expression divided by right expression.
       *            If right expression is zero, then a compile error is 
       *            logged and the process exits.
       **/
      virtual madara::Knowledge_Record evaluate (
        const madara::knowledge::Knowledge_Update_Settings & settings);

      /** 
       * Accepts a visitor subclassed from the Visitor class
       * @param    visitor   visitor instance to use
       **/
      virtual void accept (Visitor &visitor) const;
    };
  }
}

#endif // _MADARA_NO_KARL_

#endif /* _COMPOSITE_MODULUS_NODE_H */
