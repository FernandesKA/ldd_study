/**
 * @file schar_udev.c
 * @author FernandezKA (fernandes.kir@yandex.ru)
 * @brief
 * @version 0.1
 * @date 2025-10-22
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "char_udev"
#define BUF_SIZE 1024u

static int major = -1;
static struct class *char_udev_class = NULL;
static struct device *char_udev_device = NULL;
static char device_buffer[BUF_SIZE];
static size_t open_count = 0;

static int schar_open(struct inode *inode, struct file *file) {
    open_count++;
    pr_info("%s: device_opened %d time(s) \n", DEVICE_NAME, open_count);
    return 0;
}

static ssize_t schar_read(struct file* file, char __user *user_buf, size_t count, loff_t *offset) {
    size_t data_available = open_count > 0 ? strlen(device_buffer) - *offset : 0;
    size_t to_copy;

    if (data_available == 0)
        return 0; // EOF

    to_copy = min(count, data_available);

    if (copy_to_user(user_buf, device_buffer + *offset, to_copy))
        return -EFAULT;

    *offset += to_copy;
    pr_info("%s: read %zu bytes\n", DEVICE_NAME, to_copy);
    return to_copy;
}

static ssize_t schar_write(struct file *file, const char __user *user_buf, size_t count, loff_t *offset) {
    size_t to_copy = min(count, BUF_SIZE - 1);

    if (copy_from_user(device_buffer, user_buf, to_copy))
        return -EFAULT;

    device_buffer[to_copy] = '\0';
    *offset = 0;
    pr_info("%s: wrote %zu bytes\n", DEVICE_NAME, to_copy);
    open_count++;
    return to_copy;
}

static int schar_release(struct inode *inode, struct file *file) {
    pr_info("%s: device closed\n", DEVICE_NAME);
    return 0;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = schar_open,
    .read = schar_read, 
    .write = schar_write,
    .release = schar_release
};

static int __init schar_udev_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        return major;
    }

    char_udev_class = class_create(DEVICE_NAME);
    if (IS_ERR(char_udev_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(char_udev_class);
    }

    char_udev_device = device_create(char_udev_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(char_udev_device)) {
        class_destroy(char_udev_class);
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(char_udev_device);
    }

    pr_info("%s: registered with major number %d\n", DEVICE_NAME, major);
    return 0;
}

static void __exit schar_udev_exit(void) {
    device_destroy(char_udev_class, MKDEV(major, 0));
    class_destroy(char_udev_class);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("%s: unregistered\n", DEVICE_NAME);
}

module_init(schar_udev_init);
module_exit(schar_udev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FernandesKA");
MODULE_DESCRIPTION("Simple Character Device Driver with udev");
