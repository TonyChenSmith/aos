package aos.library.regex;

import aos.library.regex.NFA.State;

/**
 * 正则表达式的匹配器类。
 * 
 * <p>负责将NFA产生的状态包装为匹配器，并提供流匹配与字符串匹配两种接口。
 * 
 * <p>在2024-07-26时生成。
 * 
 * @author Tony Chen Smith
 */
public final class Matcher
{
	/**
	 * 匹配器对应模式。
	 */
	private final Pattern pattern;
	
	/**
	 * 状态机状态。
	 */
	private final State state;
	
	/**
	 * 用模式构造匹配器。
	 * 
	 * @param pattern 模式。
	 */
	Matcher(Pattern pattern)
	{
		this.pattern=pattern;
		state=pattern.init();
	}
	
	/**
	 * 获得有效结果。应该确定有结果时才进行调用。
	 * 
	 * @return 最长匹配结果，或空值。
	 */
	public String get()
	{
		return state.getLast();
	}
	
	/**
	 * 如果有有效结果，比较已扫描内容是否完全为最长匹配。
	 * 
	 * @return 如果最长匹配内容为已扫描内容，返回真。
	 */
	public boolean match()
	{
		return state.match();
	}
	
	/**
	 * 获取匹配器使用的模式。
	 * 
	 * @return 模式。
	 */
	public Pattern pattern()
	{
		return pattern;
	}
	
	/**
	 * 扫描码元。
	 * 
	 * @param codePoint 码元。
	 * 
	 * @return 匹配返回真，并期待下一个字符。
	 */
	public boolean scan(int codePoint)
	{
		return state.scan(codePoint);
	}
	
	@Override
	public String toString()
	{
		return state.toString();
	}
}
