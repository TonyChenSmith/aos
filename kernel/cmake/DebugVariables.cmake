# 
# 调试变量输出。
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 
macro(print_all_variables)
    # 获取所有变量名
    get_cmake_property(_vars VARIABLES)
    # 按字母顺序排序
    list(SORT _vars)
    # 打印每个变量及其值
    foreach(_var ${_vars})
        message(STATUS "${_var} = ${${_var}}")
    endforeach()
endmacro()