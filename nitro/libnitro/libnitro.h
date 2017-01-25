#ifndef KFUNCS_H_
#define KFUNCS_H_

#include <linux/types.h>
#include <linux/kvm.h>
#include <stdbool.h>
#include "nitro.h"

int init_kvm();
int close_kvm();

//kvm functions
int get_num_vms();
int attach_vm(pid_t);

int set_syscall_trap(bool enabled);

//vm functions
int attach_vcpus();

//vcpu functions
int get_regs(int, struct kvm_regs*);
int get_sregs(int, struct kvm_sregs*);
int get_event(int, struct event*);
int continue_vm(int);
#endif //KFUNCS_H_
