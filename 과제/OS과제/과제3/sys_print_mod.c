#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<asm/uaccess.h>
asmlinkage long sys_print_mod(int a, int b, int *ptr){
	int mod = a % b;
	put_user(mod,ptr);
	return 0;
}
SYSCALL_DEFINE3(print_mod,int,a,int,b,int*,ptr){
	return sys_print_mod(a,b,ptr);
}
