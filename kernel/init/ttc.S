# AT&T 语法，System V x86-64 ABI
# 函数名：double_value
# 输入：int 通过 %edi 传递
# 输出：翻倍后的 int 通过 %eax 返回

.text
.globl double_value
.type double_value, @function

double_value:
    movl %edi, %eax    # 将输入复制到 eax
    shll $1, %eax      # 左移1位相当于乘以2
    ret
.size double_value, .-double_value