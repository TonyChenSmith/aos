/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

/**
 * 语法符号接口。其下有两种实现：单词和非终结符号。
 * 
 * <p>在2024-08-10时生成。
 *
 * @author Tony Chen Smith
 */
interface SyntaxSymbol
{
	/**
	 * 获取属性值。
	 * 
	 * @param key 属性键。
	 * 
	 * @return 对应属性值，或空字符串。
	 */
	abstract String get(String key);
	
	/**
	 * 获取语法符号名。
	 * 
	 * @return 符号名字符串。
	 */
	abstract String name();
	
	/**
	 * 设置单词的属性值。
	 * 
	 * @param key   属性键。
	 * @param value 属性值。
	 */
	abstract void set(String key,String value);
	
	/**
	 * 获取下属符号数目。终结符返回0。
	 * 
	 * @return 下属符号数目。
	 */
	abstract int count();
	
	/**
	 * 获取语法符号。0为自身。
	 * 
	 * @param index 索引。
	 * @return 对应语法符号。
	 */
	abstract SyntaxSymbol getSymbol(int index);
}
