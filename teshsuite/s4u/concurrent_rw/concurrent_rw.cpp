/* Copyright (c) 2008-2010, 2012-2015, 2017. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/s4u.hpp"
#include <unistd.h>

#define FILENAME1 "/home/doc/simgrid/examples/platforms/g5k.xml"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_test, "Messages specific for this s4u test");

static void host()
{
  char name[2048];
  int id = simgrid::s4u::this_actor::pid();
  snprintf(name, 2048, "%s%i", FILENAME1, id);
  simgrid::s4u::File* file = new simgrid::s4u::File(name, NULL);
  XBT_INFO("process %d is writing!", id);
  file->write(3000000);
  XBT_INFO("process %d goes to sleep for %d seconds", id, id);
  simgrid::s4u::this_actor::sleep_for(id);
  XBT_INFO("process %d is writing again!", id);
  file->write(3000000);
  XBT_INFO("process %d goes to sleep for %d seconds", id, 6 - id);
  simgrid::s4u::this_actor::sleep_for(6 - id);
  XBT_INFO("process %d is reading!", id);
  file->seek(0);
  file->read(3000000);
  XBT_INFO("process %d goes to sleep for %d seconds", id, id);
  simgrid::s4u::this_actor::sleep_for(id);
  XBT_INFO("process %d is reading again!", id);
  file->seek(0);
  file->read(3000000);

  XBT_INFO("process %d => Size of %s: %llu", id, name, file->size());
  // Close the file
  delete file;
}

int main(int argc, char** argv)
{
  simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);
  e->loadPlatform(argv[1]);

  for (int i = 0; i < 5; i++)
    simgrid::s4u::Actor::createActor("host", simgrid::s4u::Host::by_name("bob"), host);

  e->run();
  XBT_INFO("Simulation time %g", e->getClock());

  return 0;
}
