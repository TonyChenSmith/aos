package aos.library.regex;

import java.io.Serializable;

import aos.library.regex.NFA.State;

/**
 * 正则表达式的模式类。
 * 
 * <p>负责将正则表达式翻译成NFA，再将NFA的状态包装成模式。
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
	 * 正则表达式字符串。
	 */
	private final String regex;
	
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
		this.regex=regex;
		nfa=new Parser(regex).parse().build();
	}
	
	@Override
	public String toString()
	{
		return regex;
	}
	
	/**
	 * 初始化状态机的一个状态。
	 * 
	 * @return 初始状态。
	 */
	State init()
	{
		return nfa.init();
	}
	
	/**
	 * 获取模式的一个匹配器。
	 * 
	 * @return 匹配器。
	 */
	public Matcher matcher()
	{
		return new Matcher(this);
	}
	
	/**
	 * 将正则表达式的字符串编译为模式。
	 * 
	 * @param regex 正则表达式。
	 * @return 对应的模式。
	 */
	public static Pattern compile(CharSequence regex)
	{
		return new Pattern(regex.toString());
	}
}
