/* Copyright (c) 2012-2017. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "s4u_bittorrent.hpp"
#include "s4u_peer.hpp"
#include "s4u_tracker.hpp"

simgrid::xbt::Extension<simgrid::s4u::Host, HostBittorrent> HostBittorrent::EXTENSION_ID;

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);

  /* Check the arguments */
  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file", argv[0]);

  e->loadPlatform(argv[1]);

  HostBittorrent::EXTENSION_ID = simgrid::s4u::Host::extension_create<HostBittorrent>();

  std::vector<simgrid::s4u::Host*> list;
  simgrid::s4u::Engine::getInstance()->getHostList(&list);
  for (auto host : list)
    host->extension_set(new HostBittorrent(host));

  e->registerFunction<Tracker>("tracker");
  e->registerFunction<Peer>("peer");
  e->loadDeployment(argv[2]);

  e->run();

  delete e;
  return 0;
}
