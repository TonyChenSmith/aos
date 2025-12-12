# 
# 目标功能函数。
# @date 2025-12-02
# 
# Copyright (c) 2025 Tony Chen Smith
# 
# SPDX-License-Identifier: MIT
# 

# 添加AOS目标
function(add_aos_target TARGET_FILE)
    get_filename_component(TARGET_NAME ${TARGET_FILE} NAME)

    add_custom_target(
        ${TARGET_NAME}.d.txt
        ALL
        COMMAND llvm-objdump --all-headers --full-contents -l -d ${TARGET_FILE} ">" ${TARGET_FILE}.d.txt
        DEPENDS ${TARGET_FILE}
        COMMENT "Disassemble ${TARGET_NAME}."
    )

    add_custom_target(
        ${TARGET_NAME}.h.txt
        ALL
        COMMAND llvm-readobj --all --program-headers -g --gnu-hash-table -d ${TARGET_FILE} ">" ${TARGET_FILE}.h.txt
        DEPENDS ${TARGET_FILE}
        COMMENT "Read the ELF heads of ${TARGET_NAME}."
    )
endfunction()

# 复制AOS目标
function(copy_aos_target OUTPUT_DIR SRC_FILE)
    get_filename_component(GROUP_NAME ${OUTPUT_DIR} NAME)
    get_filename_component(SRC_NAME ${SRC_FILE} NAME)

    add_custom_target(
        copy_${GROUP_NAME}_${SRC_NAME}
        ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OUTPUT_DIR}/${SRC_FILE} ${OUTPUT_DIR}/${SRC_NAME}
        DEPENDS ${OUTPUT_DIR}/${SRC_FILE}
        COMMENT "Copy ${SRC_NAME} to the ${GROUP_NAME} directory."
    )
endfunction()