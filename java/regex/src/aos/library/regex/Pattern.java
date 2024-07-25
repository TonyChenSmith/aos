package aos.library.regex;

import java.io.Serializable;

/**
 * 正则表达式的模式类。
 * 
 * <p>负责将正则表达式翻译成NFA，再将NFA的状态包装成匹配器。
 * 
 * <p>在2024-07-23时生成。
 * 
 * @author Tony Chen Smith
 */
public final class Pattern implements Serializable
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=786815322508535916L;
	
	/**
	 * 状态机。
	 */
	private final NFA nfa;
	
	/**
	 * 使用正则表达式字符串构造一个模式。
	 * 
	 * @param regex 正则表达式字符串。
	 */
	private Pattern(String regex)
	{
		nfa=new Parser(regex).parse().build();
	}
	
	@Override
	public String toString()
	{
		return nfa.toString();
	}
	
	/**
	 * 将正则表达式的字符串编译为模式。
	 * 
	 * @param regex 正则表达式。
	 * @return 对应的模式。
	 */
	public static Pattern compile(String regex)
	{
		return new Pattern(regex);
	}
}
