**ASOR**, 是一个简单的虚拟化管理软件(VMM)，用来帮助人们了解硬件辅助虚拟化的基本概念以及虚拟化管理软件的实现。

参考  [**ASOR - 基于x86架构的虚拟机实现**](https://calinyara.github.io/asor/2019/08/05/asor-hypervisor.html)

**编译及运行**

```
make                    // 编译asor二进制
make img                // 制作asor QEMU启动硬盘
make qemu               // 加载asor二进制启动盘到QEMU运行
x86-run x86/asor.flat   // 直接在QEMU中运行asor
```
