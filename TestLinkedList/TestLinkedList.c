#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>

/* This fucntion is called when the module is loaded*/
int simple_init(void)
{
    printk(KERN_INFO "Loading Module\n");
    return 0;
}

/* This function is called when the module is removed.*/
void simple_exit(void)
{
    printk(KERN_INFO "Removing Module\n");
}
/*宏定义模块入口点和出口点*/
module_init(simple_init);
module_exit(simple_exit);

/*模块信息基本配置*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("Rovin");

