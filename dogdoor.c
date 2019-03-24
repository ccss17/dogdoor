#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/cred.h>
#include <linux/types.h>

MODULE_LICENSE("GPL");

char user_uid[128] = { 0x0, } ;
void ** sctable ;
uid_t username_uid = -1;

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

asmlinkage int dogdoor_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256] ;

	copy_from_user(fname, filename, 256) ;

    const struct cred * my_cred = current_cred();
    long i_user_uid = simple_strtol(user_uid, NULL, 10);

    if (my_cred->uid.val == (unsigned int)i_user_uid && i_user_uid != -1) {
        printk("OS Homework : %s, Current UID: %d, Current EUID: %d, Current USER: %s\n", fname, my_cred->uid, my_cred->euid, my_cred->user);
    }

	return orig_sys_open(filename, flags, mode) ;
}


static 
int dogdoor_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static 
int dogdoor_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static
ssize_t dogdoor_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[256] ;
	ssize_t toread ;

    sprintf(buf, "%s", user_uid);

	toread = strlen(buf) >= *offset + size ? size : strlen(buf) - *offset ;
	if (copy_to_user(ubuf, buf + *offset, toread))
		return -EFAULT ;	
	*offset = *offset + toread ;
	return toread ;
}

static 
ssize_t dogdoor_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] ;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;

	sscanf(buf,"%s", user_uid) ;
	*offset = strlen(buf) ;

	return *offset ;
}

static const struct file_operations dogdoor_fops = {
	.owner = 	THIS_MODULE,
	.open = 	dogdoor_proc_open,
	.read = 	dogdoor_proc_read,
	.write = 	dogdoor_proc_write,
	.llseek = 	seq_lseek,
	.release = 	dogdoor_proc_release,
} ;

static 
int __init dogdoor_init(void) {
	unsigned int level ; 
	pte_t * pte ;

	proc_create("dogdoor", S_IRUGO | S_IWUGO, NULL, &dogdoor_fops) ;

	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;		
	sctable[__NR_open] = dogdoor_sys_open ;

	return 0;
}

static 
void __exit dogdoor_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	remove_proc_entry("dogdoor", NULL) ;

	sctable[__NR_open] = orig_sys_open ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(dogdoor_init);
module_exit(dogdoor_exit);
