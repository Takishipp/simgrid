/* Copyright (c) 2007-2017. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_MC_PRIVATE_H
#define SIMGRID_MC_PRIVATE_H

#include "mc/mc.h"
#include "xbt/automaton.h"

#ifdef __cplusplus
#include "src/mc/mc_forward.hpp"
#include "src/xbt/memory_map.hpp"
#endif

#ifdef __cplusplus
namespace simgrid {
namespace mc {

struct DerefAndCompareByActorsCountAndUsedHeap {
  template<class X, class Y>
  bool operator()(X const& a, Y const& b)
  {
    return std::make_pair(a->actors_count, a->heap_bytes_used) < std::make_pair(b->actors_count, b->heap_bytes_used);
  }
};

}
}
#endif

SG_BEGIN_DECL()

/********************************* MC Global **********************************/

XBT_PRIVATE void MC_init_dot_output();

XBT_PRIVATE extern FILE *dot_output;

XBT_PRIVATE void MC_show_deadlock(void);

/********************************** Snapshot comparison **********************************/

//#define MC_DEBUG 1
#define MC_VERBOSE 1

/********************************** Miscellaneous **********************************/

SG_END_DECL()

#ifdef __cplusplus

namespace simgrid {
namespace mc {

XBT_PRIVATE void find_object_address(
  std::vector<simgrid::xbt::VmMap> const& maps, simgrid::mc::ObjectInformation* result);

XBT_PRIVATE
int snapshot_compare(int num1, simgrid::mc::Snapshot* s1, int num2, simgrid::mc::Snapshot* s2);

// Move is somewhere else (in the LivenessChecker class, in the Session class?):
extern XBT_PRIVATE xbt_automaton_t property_automaton;

}
}

#endif

#endif
