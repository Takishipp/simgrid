/* Copyright (c) 2013-2016. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "src/plugins/vm/VirtualMachineImpl.hpp"
#include "src/surf/FileImpl.hpp"
#include <string>

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_host, surf, "Logging specific to the SURF host module");

simgrid::surf::HostModel *surf_host_model = nullptr;

/*************
 * Callbacks *
 *************/

namespace simgrid {
namespace surf {

/*********
 * Model *
 *********/

/* Each VM has a dummy CPU action on the PM layer. This CPU action works as the
 * constraint (capacity) of the VM in the PM layer. If the VM does not have any
 * active task, the dummy CPU action must be deactivated, so that the VM does
 * not get any CPU share in the PM layer. */
void HostModel::ignoreEmptyVmInPmLMM()
{
  /* iterate for all virtual machines */
  for (s4u::VirtualMachine* ws_vm : vm::VirtualMachineImpl::allVms_) {
    Cpu* cpu = ws_vm->pimpl_cpu;
    int active_tasks = lmm_constraint_get_variable_amount(cpu->constraint());

    /* The impact of the VM over its PM is the min between its vCPU amount and the amount of tasks it contains */
    int impact = std::min(active_tasks, ws_vm->pimpl_vm_->coreAmount());

    XBT_DEBUG("set the weight of the dummy CPU action of VM%p on PM to %d (#tasks: %d)", ws_vm, impact, active_tasks);
    ws_vm->pimpl_vm_->action_->setPriority(impact);
  }
}

/* Helper function for executeParallelTask */
static inline double has_cost(double* array, int pos)
{
  if (array)
    return array[pos];
  else
    return -1.0;
}

Action* HostModel::executeParallelTask(int host_nb, simgrid::s4u::Host** host_list, double* flops_amount,
    double* bytes_amount, double rate)
{
  Action* action = nullptr;
  if ((host_nb == 1) && (has_cost(bytes_amount, 0) <= 0)) {
    action = host_list[0]->pimpl_cpu->execution_start(flops_amount[0]);
  } else if ((host_nb == 1) && (has_cost(flops_amount, 0) <= 0)) {
    action = surf_network_model->communicate(host_list[0], host_list[0], bytes_amount[0], rate);
  } else if ((host_nb == 2) && (has_cost(flops_amount, 0) <= 0) && (has_cost(flops_amount, 1) <= 0)) {
    int nb = 0;
    double value = 0.0;

    for (int i = 0; i < host_nb * host_nb; i++) {
      if (has_cost(bytes_amount, i) > 0.0) {
        nb++;
        value = has_cost(bytes_amount, i);
      }
    }
    if (nb == 1) {
      action = surf_network_model->communicate(host_list[0], host_list[1], value, rate);
    } else if (nb == 0) {
      xbt_die("Cannot have a communication with no flop to exchange in this model. You should consider using the "
          "ptask model");
    } else {
      xbt_die("Cannot have a communication that is not a simple point-to-point in this model. You should consider "
          "using the ptask model");
    }
  } else
    xbt_die(
        "This model only accepts one of the following. You should consider using the ptask model for the other cases.\n"
        " - execution with one host only and no communication\n"
        " - Self-comms with one host only\n"
        " - Communications with two hosts and no computation");
  xbt_free(host_list);
  return action;
}

/************
 * Resource *
 ************/
HostImpl::HostImpl(s4u::Host* host) : piface_(host)
{
  /* The VM wants to reinstall a new HostImpl, but we don't want to leak the previously existing one */
  delete piface_->pimpl_;
  piface_->pimpl_ = this;
}

simgrid::surf::StorageImpl* HostImpl::findStorageOnMountList(const char* mount)
{
  XBT_DEBUG("Search for storage name '%s' on '%s'", mount, piface_->cname());
  if (storage_.find(mount) == storage_.end())
    xbt_die("Can't find mount '%s' for '%s'", mount, piface_->cname());

  return storage_.at(mount);
}

void HostImpl::getAttachedStorageList(std::vector<const char*>* storages)
{
  for (auto s : storage_)
    if (s.second->attach_ == piface_->cname())
      storages->push_back(s.second->piface_.name());
}

Action* HostImpl::close(surf_file_t fd)
{
  simgrid::surf::StorageImpl* st = findStorageOnMountList(fd->mount());
  XBT_DEBUG("CLOSE %s on disk '%s'", fd->cname(), st->cname());
  return st->close(fd);
}

Action* HostImpl::read(surf_file_t fd, sg_size_t size)
{
  simgrid::surf::StorageImpl* st = findStorageOnMountList(fd->mount());
  XBT_DEBUG("READ %s on disk '%s'", fd->cname(), st->cname());
  return st->read(fd, size);
}

Action* HostImpl::write(surf_file_t fd, sg_size_t size)
{
  simgrid::surf::StorageImpl* st = findStorageOnMountList(fd->mount());
  XBT_DEBUG("WRITE %s on disk '%s'", fd->cname(), st->cname());
  return st->write(fd, size);
}

int HostImpl::unlink(surf_file_t fd)
{
  if (not fd) {
    XBT_WARN("No such file descriptor. Impossible to unlink");
    return -1;
  } else {

    simgrid::surf::StorageImpl* st = findStorageOnMountList(fd->mount());
    /* Check if the file is on this storage */
    if (st->content_->find(fd->cname()) == st->content_->end()) {
      XBT_WARN("File %s is not on disk %s. Impossible to unlink", fd->cname(), st->cname());
      return -1;
    } else {
      XBT_DEBUG("UNLINK %s on disk '%s'", fd->cname(), st->cname());
      st->usedSize_ -= fd->size();

      // Remove the file from storage
      st->content_->erase(fd->cname());

      return 0;
    }
  }
}

int HostImpl::fileSeek(surf_file_t fd, sg_offset_t offset, int origin)
{
  switch (origin) {
  case SEEK_SET:
    fd->setPosition(offset);
    return 0;
  case SEEK_CUR:
    fd->incrPosition(offset);
    return 0;
  case SEEK_END:
    fd->setPosition(fd->size() + offset);
    return 0;
  default:
    return -1;
  }
}

int HostImpl::fileMove(surf_file_t fd, const char* fullpath)
{
  /* Check if the new full path is on the same mount point */
  if (not strncmp(fd->mount(), fullpath, strlen(fd->mount()))) {
    std::map<std::string, sg_size_t>* content = findStorageOnMountList(fd->mount())->content_;
    if (content->find(fd->name()) != content->end()) { // src file exists
      sg_size_t new_size = content->at(fd->name());
      content->erase(fd->name());
      std::string path = std::string(fullpath).substr(strlen(fd->mount()), strlen(fullpath));
      content->insert({path.c_str(), new_size});
      XBT_DEBUG("Move file from %s to %s, size '%llu'", fd->cname(), fullpath, new_size);
      return 0;
    } else {
      XBT_WARN("File %s doesn't exist", fd->cname());
      return -1;
    }
  } else {
    XBT_WARN("New full path %s is not on the same mount point: %s. Action has been canceled.", fullpath, fd->mount());
    return -1;
  }
}

}
}
