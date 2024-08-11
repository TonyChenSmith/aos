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
}
