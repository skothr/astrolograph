#ifndef UNDO_HPP
#define UNDO_HPP

#include <deque>
#include <any>

namespace astro
{
  // TODO: Action stack for undoing with Ctrl-Z
  enum ActionType
    {
     ACTION_INVALID = -1,
     ACTION_ADD_NODES,
     //ACTION_REMOVE_NODES,
     ACTION_MOVE_NODES,   // (set after full node move -- mouse press, drag, release)
     ACTION_COPY_NODES,
     
     ACTION_CUT,
     ACTION_COPY,
     ACTION_PASTE,
     ACTION_CHANGE_VALUE,
    };
  struct Action
  {
    ActionType type = ACTION_INVALID;
    std::any data;
  };
  typedef std::deque<Action> UndoStack;
}


#endif // UNDO_HPP
