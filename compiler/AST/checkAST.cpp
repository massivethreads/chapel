// checkAST.cpp
//
// Additional structural checks on the health of the AST.
//

#include "passes.h"

#include "expr.h"
#include "driver.h" // For prototypes.

//#include <unordered_set> // C++11 (not fully supported yet)
#include <set>
#include <map>


void checkForDuplicateUses()
{
  // The specific case we're looking for is where there is an argument that
  // appears in more than one function.
  // So, scan the list of functions, cache their arguments, and barf if a
  // duplicate is encountered.
  std::set<ArgSymbol*> args_seen;
  forv_Vec(FnSymbol, fn, gFnSymbols)
  {
    for_formals(formal, fn)
    {
      if (args_seen.count(formal))
        INT_FATAL("Argument is used in multiple function definitions.");
      args_seen.insert(formal);
    }
  }
}

// This check ensures that every (resolved) SymExpr
// refers to a DefExpr within the tree.
// TODO: Once this check is satisfied by all passes, it can be replaced by 
//  this->sym->defPoint == this in DefExpr::verify().
void checkForMissingDefs()
{
  std::map<Symbol*,DefExpr*> defs;

  // Populate the map with all DefExprs
  forv_Vec(DefExpr, def, gDefExprs)
  {
    Symbol* sym = def->sym;
    if (defs.count(sym))
      INT_FATAL("Multiple definitions for symbol");
    defs.insert(std::pair<Symbol*,DefExpr*>(sym, def));
  }

  // Now make sure that every symbol reference has a live def associated with
  // it.
  forv_Vec(SymExpr, se, gSymExprs)
  {
    Symbol* sym = se->var;

    // Ignore symbols that have no defPoint
    // such as _root
    if (sym->defPoint == 0)
      continue;

    if (defs.count(sym) == 0)
      INT_FATAL("Symbol has no associated DefExpr.");
    DefExpr* def = defs.find(sym)->second;
    if (sym->defPoint != def)
      INT_FATAL("defPoint of symbol does not match its declaration.");
  }
}


// Check that no unresolved symbols remain in the tree.
// This one is pretty cheap, so can be run after every pass (following
// resolution).
void checkNoUnresolveds()
{
  // TODO: This value should really be cranked down to 0.
  // But in some cases _construct__tuple remains unresolved after
  // resolution ... .
  if (gUnresolvedSymExprs.n > 1)
    INT_FATAL("Structural error: "
      "At this point, the AST should not contain any unresovled symbols.");
}

// Ensures that primitives are used only where they are expected.
void checkPrimitives()
{
  forv_Vec(CallExpr, call, gCallExprs)
  {
    // Only interested in primitives
    if (!call->primitive)
      continue;

    switch(call->primitive->tag)
    {
     default:
      INT_FATAL("Unhandled case in checkPrimitives");
      break;

      // These do not survive past parsing.
     case PRIM_ACTUALS_LIST:
      if (parsed)
        INT_FATAL("Primitive should not appear after parsing is complete.");
      break;

      // These do not survive past resolution.
     case PRIM_INIT:
      if (resolved)
        INT_FATAL("Primitive should not appear after resolution is complete.");
      break;

     case PRIM_UNKNOWN:
     case PRIM_NOOP:
     case PRIM_MOVE:
     case PRIM_REF_TO_STRING:
     case PRIM_RETURN:
     case PRIM_YIELD:
     case PRIM_UNARY_MINUS:
     case PRIM_UNARY_PLUS:
     case PRIM_UNARY_NOT:
     case PRIM_UNARY_LNOT:
     case PRIM_ADD:
     case PRIM_SUBTRACT:
     case PRIM_MULT:
     case PRIM_DIV:
     case PRIM_MOD:
     case PRIM_LSH:
     case PRIM_RSH:
     case PRIM_EQUAL:
     case PRIM_NOTEQUAL:
     case PRIM_LESSOREQUAL:
     case PRIM_GREATEROREQUAL:
     case PRIM_LESS:
     case PRIM_GREATER:
     case PRIM_AND:
     case PRIM_OR:
     case PRIM_XOR:
     case PRIM_POW:
     case PRIM_ADD_ASSIGN:
     case PRIM_SUBTRACT_ASSIGN:
     case PRIM_MULT_ASSIGN:
     case PRIM_DIV_ASSIGN:
     case PRIM_MOD_ASSIGN:
     case PRIM_LSH_ASSIGN:
     case PRIM_RSH_ASSIGN:
     case PRIM_AND_ASSIGN:
     case PRIM_OR_ASSIGN:
     case PRIM_XOR_ASSIGN:
     case PRIM_MIN:
     case PRIM_MAX:
     case PRIM_SETCID:
     case PRIM_TESTCID:
     case PRIM_GETCID:
     case PRIM_SET_UNION_ID:
     case PRIM_GET_UNION_ID:
     case PRIM_GET_MEMBER:
     case PRIM_GET_MEMBER_VALUE:
     case PRIM_SET_MEMBER:
     case PRIM_CHECK_NIL:
     case PRIM_NEW:                 // new keyword
     case PRIM_GET_REAL:            // get complex real component
     case PRIM_GET_IMAG:            // get complex imag component
     case PRIM_QUERY:               // query expression primitive
     case PRIM_ADDR_OF:             // set a reference to a value
     case PRIM_DEREF:               // dereference a reference
     case PRIM_LOCAL_CHECK:         // assert that a wide ref is on this locale
     case PRIM_SYNC_INIT:
     case PRIM_SYNC_DESTROY:
     case PRIM_SYNC_LOCK:
     case PRIM_SYNC_UNLOCK:
     case PRIM_SYNC_WAIT_FULL:
     case PRIM_SYNC_WAIT_EMPTY:
     case PRIM_SYNC_SIGNAL_FULL:
     case PRIM_SYNC_SIGNAL_EMPTY:
     case PRIM_SINGLE_INIT:
     case PRIM_SINGLE_DESTROY:
     case PRIM_SINGLE_LOCK:
     case PRIM_SINGLE_UNLOCK:
     case PRIM_SINGLE_WAIT_FULL:
     case PRIM_SINGLE_SIGNAL_FULL:
     case PRIM_WRITEEF:
     case PRIM_WRITEFF:
     case PRIM_WRITEXF:
     case PRIM_SYNC_RESET:
     case PRIM_READFE:
     case PRIM_READFF:
     case PRIM_READXX:
     case PRIM_SYNC_IS_FULL:
     case PRIM_SINGLE_WRITEEF:
     case PRIM_SINGLE_RESET:
     case PRIM_SINGLE_READFF:
     case PRIM_SINGLE_READXX:
     case PRIM_SINGLE_IS_FULL:
     case PRIM_GET_END_COUNT:
     case PRIM_SET_END_COUNT:
     case PRIM_PROCESS_TASK_LIST:
     case PRIM_EXECUTE_TASKS_IN_LIST:
     case PRIM_FREE_TASK_LIST:
     case PRIM_GET_SERIAL:              // get serial state
     case PRIM_SET_SERIAL:              // set serial state to true or false
     case PRIM_SIZEOF:
     case PRIM_TASK_ALLOC:              // Task-specific malloc
     case PRIM_TASK_REALLOC:            // Task-specific realloc
     case PRIM_TASK_FREE:               // Task-specific free.
     case PRIM_CHPL_MEMHOOK_FREE:       // The free hook function.
     case PRIM_CHPL_ALLOC:
     case PRIM_CHPL_FREE:               // only for variables on heap?
     case PRIM_INIT_FIELDS:             // initialize fields of a temporary record
     case PRIM_PTR_EQUAL:
     case PRIM_PTR_NOTEQUAL:
     case PRIM_IS_SUBTYPE:
     case PRIM_CAST:
     case PRIM_DYNAMIC_CAST:
     case PRIM_TYPEOF:
     case PRIM_GET_ITERATOR_RETURN:
     case PRIM_USE:
     case PRIM_USED_MODULES_LIST:       // used modules in BlockStmt::modUses
     case PRIM_TUPLE_EXPAND:
     case PRIM_TUPLE_AND_EXPAND:
     case PRIM_CHPL_COMM_GET:           // Direct calls to the Chapel comm layer
     case PRIM_CHPL_COMM_PUT:           // may eventually add others (e.g.: non-blocking)
     case PRIM_CHPL_COMM_GET_STRD:      // Direct calls to the Chapel comm layer for strided comm
     case PRIM_CHPL_COMM_PUT_STRD:      //  may eventually add others (e.g.: non-blocking)
     case PRIM_ARRAY_ALLOC:
     case PRIM_ARRAY_FREE:
     case PRIM_ARRAY_FREE_ELTS:
     case PRIM_ARRAY_GET:
     case PRIM_ARRAY_GET_VALUE:
     case PRIM_ARRAY_SHIFT_BASE_POINTER:
     case PRIM_ARRAY_SET:
     case PRIM_ARRAY_SET_FIRST:
     case PRIM_ERROR:
     case PRIM_WARNING:
     case PRIM_WHEN:
     case PRIM_TYPE_TO_STRING:
     case PRIM_BLOCK_PARAM_LOOP:        // BlockStmt::blockInfo - param for loop
     case PRIM_BLOCK_WHILEDO_LOOP:      // BlockStmt::blockInfo - while do loop
     case PRIM_BLOCK_DOWHILE_LOOP:      // BlockStmt::blockInfo - do while loop
     case PRIM_BLOCK_FOR_LOOP:          // BlockStmt::blockInfo - for loop
     case PRIM_BLOCK_BEGIN:             // BlockStmt::blockInfo - begin block
     case PRIM_BLOCK_COBEGIN:           // BlockStmt::blockInfo - cobegin block
     case PRIM_BLOCK_COFORALL:          // BlockStmt::blockInfo - coforall block
     case PRIM_BLOCK_XMT_PRAGMA_FORALL_I_IN_N: // BlockStmt::blockInfo - xmt prag loop
     case PRIM_BLOCK_XMT_PRAGMA_NOALIAS:       // BlockStmt::blockInfo - xmt prag for
     case PRIM_BLOCK_ON:                // BlockStmt::blockInfo - on block
     case PRIM_BLOCK_ON_NB:             // BlockStmt::blockInfo - non-blocking on block
     case PRIM_BLOCK_LOCAL:             // BlockStmt::blockInfo - local block
     case PRIM_BLOCK_UNLOCAL:           // BlockStmt::blockInfo - unlocal local block
     case PRIM_TO_LEADER:
     case PRIM_TO_FOLLOWER:
     case PRIM_DELETE:
     case PRIM_GC_CC_INIT:              // Initialize heap for copy-collecting
     case PRIM_GC_ADD_ROOT:             // Add a root variable for garbage collection
     case PRIM_GC_ADD_NULL_ROOT:        // Add a root and point it to NULL
     case PRIM_GC_DELETE_ROOT:          // Remove a root variable for garbage collection
     case PRIM_GC_CLEANUP:              // Free GC heaps
     case PRIM_CALL_DESTRUCTOR:         // call destructor on type (do not free)
     case PRIM_LOGICAL_FOLDER:          // Help fold logical && and ||
     case PRIM_WIDE_GET_LOCALE:         // Returns the "locale" portion of a wide pointer.
     case PRIM_WIDE_GET_NODE:           // Get just the node portion of a wide pointer.
     case PRIM_WIDE_GET_ADDR:           // Get just the address portion of a wide pointer.
     case PRIM_IS_HERE:                 // Returns true if the arg matches the current locale ID.
     case PRIM_ON_LOCALE_NUM:           // specify a particular localeID for an on clause.
     case PRIM_TASK_SET_LOCALE_ID:      // Set the locale ID (here ID) in task-private data.
     case PRIM_TASK_GET_LOCALE_ID:      // Get the locale ID (here ID) from task-private data.
     case PRIM_TASK_SET_HERE_PTR:       // Set the (local) address of here in task-private data.
     case PRIM_TASK_GET_HERE_PTR:       // Get the (local) address of here from task-private data.
     case PRIM_ALLOC_GVR:               // allocate space for global vars registry
     case PRIM_HEAP_REGISTER_GLOBAL_VAR:
     case PRIM_HEAP_BROADCAST_GLOBAL_VARS:
     case PRIM_PRIVATE_BROADCAST:
     case PRIM_INT_ERROR:
     case PRIM_CAPTURE_FN:
     case PRIM_CREATE_FN_TYPE:
     case PRIM_STRING_COPY:
     case PRIM_STRING_NORMALIZE:        // Set the size field in a (wide) string.
     case PRIM_CAST_TO_VOID_STAR:       // Cast the object argument to void*.
     case PRIM_RT_ERROR:
     case PRIM_RT_WARNING:
     case PRIM_NEW_PRIV_CLASS:
     case PRIM_NUM_PRIV_CLASSES:
     case PRIM_GET_PRIV_CLASS:
     case PRIM_NEXT_UINT32:
     case PRIM_GET_USER_LINE:
     case PRIM_GET_USER_FILE:
     case PRIM_FTABLE_CALL:
     case PRIM_IS_STAR_TUPLE_TYPE:
     case PRIM_SET_SVEC_MEMBER:
     case PRIM_GET_SVEC_MEMBER:
     case PRIM_GET_SVEC_MEMBER_VALUE:
     case PRIM_VMT_CALL:
     case PRIM_NUM_FIELDS:
     case PRIM_FIELD_NUM_TO_NAME:
     case PRIM_FIELD_VALUE_BY_NUM:
     case PRIM_FIELD_ID_BY_NUM:
     case PRIM_FIELD_VALUE_BY_NAME:
     case PRIM_IS_UNION_TYPE:
     case PRIM_ENUM_MIN_BITS:
     case PRIM_ENUM_IS_SIGNED:
      break;
    }
  }
}
