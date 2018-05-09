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
https://pan.baidu.com/s/1efDz3G4r462bm4YPjZtHQQ
