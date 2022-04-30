#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<asm/uaccess.h>
asmlinkage long sys_print_add(int a, int b, int *ptr){
	int add = a + b;
	put_user(add,ptr);
	return 0;
}
SYSCALL_DEFINE3(print_add,int,a,int,b,int*,ptr){
	return sys_print_add(a,b,ptr);
}
