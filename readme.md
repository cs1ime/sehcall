## Windows X64 mode use seh in manual mapped dll or manual mapped sys

## 在手动映射dll或手动映射sys中使用seh

偶然发现的一个seh特性,只要返回地址在异常表记录范围内,Windows就会正常处理seh,把ip修改回到Handler的地址,只要找到Handler代码是直接返回的函数就能实现任意使用seh

r3可以利用IsBadReadPtr

<img src=".\image-20221009192831645.png" alt="image-20221009192831645" style="zoom:50%;" />

r0可以利用KdpSysWriteMsr

<img src=".\image-20221009192708539.png" alt="image-20221009192708539" style="zoom: 67%;" />





