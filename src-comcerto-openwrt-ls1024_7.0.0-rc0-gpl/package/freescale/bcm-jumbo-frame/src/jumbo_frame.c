#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

extern int
robo_spi_read(void *cookie, uint16_t reg, uint64_t *buf, int len);

extern int
robo_spi_write(void *cookie, uint16_t reg, uint64_t *buf, int len);

#define ROBO_RREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_spi_read(NULL, \
                      (_page << 8) | (_reg), _buf, _len)
#define ROBO_WREG(_robo, _dev, _page, _reg, _buf, _len) \
        robo_spi_write(NULL, \
                       (_page << 8) | (_reg), _buf, _len)


/*** Enable Jumbo frame support on BCM53128 and BCM53134 
Register 0x40 0x1 0x1ff ***/ 
static int __init bcm_robo_jumbo_module_init(void)
{
   	printk("bcm_robo_jumbo_module_init: Entered\n");

        uint8_t   buf[8], buf1[8];

   	buf[0] = 0xff;
   	buf[1] = 0x1;
   	ROBO_WREG(0, 0, 0x40, 0x01, (uint64_t *)buf, (uint8_t)2);

   	ROBO_RREG(0, 0, 0x40, 0x01,  (uint64_t *)buf1, (uint8_t)2);
   	printk("\nJumbo frame Register 0x40 0x01 [%x]\n", buf1[0]);

  	printk(" bcm_robo_jumbo_module_init() RETURN 0\n");
   	return 0;
}

//Disable Jumbo franes on BCM53128 and BCM53134
static void __exit bcm_robo_jumbo_module_exit(void)
{

   	printk("bcm_robo_jumbo_module_exit : Entered\n");

        uint8_t   buf[8], buf1[8];

   	buf[0] = 0x0;
   	ROBO_WREG(0, 0, 0x40, 0x01, (uint64_t *)buf, (uint8_t)2);

   	ROBO_RREG(0, 0, 0x40, 0x01, (uint64_t *)buf1, (uint8_t)2);
   	printk("\nJumbo frame Register 0x40, 0x1 [%x]\n", buf1[0]);
	printk("bcm_robo_jumbo_module_exit\n");
}

module_init(bcm_robo_jumbo_module_init);
module_exit(bcm_robo_jumbo_module_exit);
MODULE_LICENSE("GPL");

