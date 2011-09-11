/*
 *  Copyright (C) 2011 Petr Benes.
 *  Copyright (C) 2011 On-Line Applications Research Corporation (OAR).
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 *
 *  $Id$
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <rtems/system.h>
#include <rtems/score/isr.h>
#include <rtems/score/scheduler.h>
#include <rtems/score/scheduleredf.h>
#include <rtems/score/thread.h>

void _Scheduler_EDF_Yield(void)
{
  Scheduler_EDF_Per_thread *first_info;
  RBTree_Node              *first_node;
  ISR_Level                 level;

  Thread_Control *executing  = _Thread_Executing;
  Scheduler_EDF_Per_thread *executing_info =
    (Scheduler_EDF_Per_thread *) executing->scheduler_info;
  RBTree_Node *executing_node = &(executing_info->Node);

  _ISR_Disable( level );

  if ( !_RBTree_Has_only_one_node(&_Scheduler_EDF_Ready_queue) ) {
    /*
     * The RBTree has more than one node, enqueue behind the tasks
     * with the same priority in case there are such ones.
     */
    _RBTree_Extract( &_Scheduler_EDF_Ready_queue, executing_node );
    _RBTree_Insert( &_Scheduler_EDF_Ready_queue, executing_node );

    _ISR_Flash( level );

    if ( _Thread_Is_heir( executing ) ) {
      first_node = _RBTree_Peek( &_Scheduler_EDF_Ready_queue, RBT_LEFT );
      first_info =
        _RBTree_Container_of(first_node, Scheduler_EDF_Per_thread, Node);
      _Thread_Heir = first_info->thread;
    }
    _Thread_Dispatch_necessary = true;
  }
  else if ( !_Thread_Is_heir( executing ) )
    _Thread_Dispatch_necessary = true;

  _ISR_Enable( level );
}
