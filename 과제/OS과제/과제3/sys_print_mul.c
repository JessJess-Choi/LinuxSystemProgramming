#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<asm/uaccess.h>
asmlinkage long sys_print_mul(int a, int b, int *ptr){
	int mul = a * b;
	put_user(mul,ptr);
	return 0;
}
SYSCALL_DEFINE3(print_mul,int,a,int,b,int*,ptr){
	return (sys_print_mul(a,b,ptr));
}
