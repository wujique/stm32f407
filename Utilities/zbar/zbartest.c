/*
	二维码解码接口

	http://zbar.sourceforge.net/ 这是主页，稳定的版本是0.10
	代码到下面地址的仓库拿
	http://sourceforge.net/projects/zbar
	在github上也有一个镜像仓库
	https://github.com/ZBar/ZBar 也是七八年没人更新了。

	ZBar is licensed under the GNU LGPL 2.1 to enable development of both open source and commercial projects.
	ZBar使用GNU LGPL 2.1授权协议，对于ZBar源码的修改和优化，必须开源
	
*/
#include "stm32f4xx.h"
#include "wujique_log.h"
#include "zbar.h"
#include "image.h"
/*
	raw：图像数据，灰度数据
	width：图像宽度
	height：图像高度
*/
int Zbar_Test(void* raw, int width, int height)
{
	zbar_image_scanner_t *scanner = NULL;
	
    /* create a reader */
    scanner = zbar_image_scanner_create();
    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);
	
    /* scan the image for barcodes */
    int n = zbar_scan_image(scanner, image);
	
    printf("n = %d\r\n", n);
    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);

    for(; symbol; symbol = zbar_symbol_next(symbol)) 
	{
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
	
        printf("\r\n--------------decoded %s symbol \"%s\"\r\n", zbar_get_symbol_name(typ), data);
        printf("len = %d\r\n",strlen(data));
		
    }

    /* clean up */
    zbar_image_destroy(image);

    zbar_image_scanner_destroy(scanner);

    return(0);
}   



