#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<asm/uaccess.h>
asmlinkage long sys_print_min(int a, int b, int *ptr){
	int min = a - b;
	put_user(min,ptr);
	return 0;
}
SYSCALL_DEFINE3(print_min,int,a,int,b,int*,ptr){
	return sys_print_min(a,b,ptr);
}
