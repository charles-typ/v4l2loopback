#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/cdev.h>

// File writing
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guojun Chen");
MODULE_DESCRIPTION("A simple example Linux module.");
MODULE_VERSION("0.01");

#define BUFFER_LENGTH (512 * 1024)

static void *mem_ptr = NULL;

extern ssize_t custom_v4l2_loopback_read(char *buf, size_t count);

struct file* writeToFile(void* buf, size_t size, loff_t *offset)
{
    struct file* destFile;
    char* filePath = "/tmp/output";
    size_t res;

    // Add the O_LARGEFILE flag here
    destFile = filp_open(filePath, O_CREAT | O_WRONLY | O_APPEND | O_LARGEFILE, 0666);
    if (IS_ERR(destFile))
    {
        printk("Error in opening: <%ld>", (long)destFile);
        return destFile;
    }
    if (destFile == NULL)
    {
        printk("Error in opening: null");
        return destFile;
    }

    res = kernel_write(destFile, buf, size, offset);
    printk("CODE: <%ld>", res);
    filp_close(destFile, NULL);

    return destFile;
}


static int __init hello_init(void) {
    printk(KERN_INFO "Hello, World!\n");
    mem_ptr = kmalloc(BUFFER_LENGTH, GFP_KERNEL);
    if (mem_ptr == NULL) {
        printk(KERN_INFO "mymem_init: kmalloc [FAILED]\n");
        return ENOMEM;
    } else {
        custom_v4l2_loopback_read(mem_ptr, BUFFER_LENGTH);
        //writeToFile(mem_ptr, BUFFER_LENGTH, );
        printk (KERN_ERR "Reading data from camera driver\n");
    }
    return 0;
}
static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, World!\n");
    kfree(mem_ptr);
}

module_init(hello_init);
module_exit(hello_exit);
