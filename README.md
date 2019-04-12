### This code repository stops updating and moves to the PetiteDrv code repository
### 本代码仓库停止更新，转移到PetiteDrv代码仓库

---

# stm32f407
some device driver base on stm32f407
在STM32F407的基础上做一些驱动设计：
个人认为比较有参考意义的是：SPI,SPI FLASH, LCD。
整个代码按照一个实际项目的模式层层递进：
本仓库是一个完整的代码。

# 分支
- master
master 分支是从无到有，经历了驱动编写的过程。

- sw_arch
sw_arch是软件架构，在master分支完成了所有驱动功能验证的基础上，对spi, I2C，LCD，LCD接口,
SPI FLASH等关键设备进行架构优化。并尽可能的对所有设备进行设备&驱动分离的方式设计。

# BASE
硬件：屋脊雀F407（STM32F407ZG）
环境：MDK

# 教程
每个驱动开发都有配套源码和教程：
https://pan.baidu.com/s/12o0Vh4Tv4z_O8qh49JwLjg

官网：www.wujique.com
github: https://github.com/wujique/stm32f407
淘宝：https://shop316863092.taobao.com/?spm=2013.1.1000126.2.3a8f4e6eb3rBdf
技术支持邮箱：code@wujique.com、github@wujique.com
资料下载：https://pan.baidu.com/s/12o0Vh4Tv4z_O8qh49JwLjg
QQ群：767214262