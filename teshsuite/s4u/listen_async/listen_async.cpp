/* Bug report: https://github.com/simgrid/simgrid/issues/40
 *
 * Task.listen used to be on async mailboxes as it always returned false.
 * This occures in Java and C, but is only tested here in C.
 */

#include "simgrid/s4u.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_test, "Messages specific for this s4u example");

static void server()
{
  simgrid::s4u::MailboxPtr mailbox = simgrid::s4u::Mailbox::byName("mailbox");

  simgrid::s4u::this_actor::isend(mailbox, xbt_strdup("Some data"), 0);

  xbt_assert(mailbox->listen()); // True (1)
  XBT_INFO("Task listen works on regular mailboxes");
  char* res = static_cast<char*>(simgrid::s4u::this_actor::recv(mailbox));

  xbt_assert(!strcmp("Some data", res), "Data received: %s", res);
  XBT_INFO("Data successfully received from regular mailbox");
  xbt_free(res);

  simgrid::s4u::MailboxPtr mailbox2 = simgrid::s4u::Mailbox::byName("mailbox2");
  mailbox2->setReceiver(simgrid::s4u::Actor::self());

  simgrid::s4u::this_actor::isend(mailbox2, xbt_strdup("More data"), 0);

  xbt_assert(mailbox2->listen()); // used to break.
  XBT_INFO("Task listen works on asynchronous mailboxes");

  res = static_cast<char*>(simgrid::s4u::this_actor::recv(mailbox2));
  xbt_assert(!strcmp("More data", res));
  xbt_free(res);

  XBT_INFO("Data successfully received from asynchronous mailbox");
}

int main(int argc, char* argv[])
{
  simgrid::s4u::Engine* e = new simgrid::s4u::Engine(&argc, argv);
  e->loadPlatform(argv[1]);

  simgrid::s4u::Actor::createActor("test", simgrid::s4u::Host::by_name("Tremblay"), server);

  e->run();
  return 0;
}