package aos.tools.compiler;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import aos.library.regex.Pattern;

/**
 * 编译器前端上下文。
 * 
 * <p>提供整个编译器的当前环境，回调方法执行引擎。
 * 
 * <p>在2024-08-13时生成。
 *
 * @author Tony Chen Smith
 */
final class Context
{
	/**
	 * 无动作回调。
	 */
	private static final LexerCallback NONACTION=(t,c)->{};
	
	/**
	 * 全局禁用单词名。
	 */
	final Set<String> banTokens;
	
	/**
	 * 单词回调。
	 */
	final Map<String,LexerCallback> callbacks;
	
	/**
	 * 词法器模式映射。
	 */
	final Map<String,Pattern> patterns;
	
	/**
	 * 单词优先级映射。
	 */
	final Map<String,Integer> priorityMap;
	
	/**
	 * 单词映射。
	 */
	final Map<Pattern,String> tokenMap;
	
	/**
	 * 语法器产生式映射。
	 */
	final Map<String,Production> productions;
	
	/**
	 * 列号。
	 */
	private int column;
	
	/**
	 * 文本长度。
	 */
	private long length;
	
	/**
	 * 行号。
	 */
	private final int line;
	
	/**
	 * 当前行文本。
	 */
	private final List<Integer> text;
	
	Context(Map<String,Pattern> patterns,Map<String,LexerCallback> callbacks,Map<Pattern,String> tokenMap,Map<String,Integer> priorityMap,Set<String> banTokens,Map<String,Production> productions)
	{
		this.patterns=Map.copyOf(patterns);
		this.callbacks=Map.copyOf(callbacks);
		this.tokenMap=Map.copyOf(tokenMap);
		this.priorityMap=Map.copyOf(priorityMap);
		this.banTokens=Set.copyOf(banTokens);
		this.productions=Map.copyOf(productions);
		
		text=new ArrayList<>();
		line=1;
		column=1;
		length=0;
	}
	
	/**
	 * 扫描时接受输入码元。
	 * 
	 * @param input 输入码元。
	 */
	void acceptInput(int input)
	{
		text.add(input);
		column++;
		length++;
	}
	
	/**
	 * 返回当前列号。
	 * 
	 * @return 当前列号。
	 */
	int currentColumn()
	{
		return column;
	}
	
	/**
	 * 返回当前已读文本长度。
	 * 
	 * @return 当前已读文本长度。
	 */
	long currentLength()
	{
		return length;
	}
	
	/**
	 * 返回当前行号。
	 * 
	 * @return 当前行号。
	 */
	int currentLine()
	{
		return line;
	}
	
	/**
	 * 获取单词优先级。
	 * 
	 * @param pattern 模式。
	 *
	 * @return 对应单词优先级。
	 */
	int getTokenPriority(Pattern pattern)
	{
		return priorityMap.get(tokenMap.get(pattern));
	}
	
	/**
	 * 在扫描时，调用单词回调。
	 * 
	 * @param token 单词。
	 */
	void onScan(Token token)
	{
		callbacks.getOrDefault(token.name(),NONACTION).process(token,this);
	}
}
