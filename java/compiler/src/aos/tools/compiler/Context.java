package aos.tools.compiler;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
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
	 * 语法分析表。
	 */
	final LRTable table;
	
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
	private int line;
	
	/**
	 * 当前行文本。
	 */
	private List<Integer> text;
	
	/**
	 * 状态栈。
	 */
	Deque<Integer> states;
	
	/**
	 * 符号栈。
	 */
	Deque<SyntaxSymbol> symbols;
	
	/**
	 * 变量域。存储层级为<code>非终结符.变量名.变量值</code>，为全局不询问直接覆盖。
	 */
	private Map<String,Map<String,String>> variables;
	
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
		states=new ArrayDeque<>();
		symbols=new ArrayDeque<>();
		variables=new HashMap<>();
		
		table=LRTable.create(this);
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
	 * 重置上下文。
	 */
	void reset()
	{
		text.clear();
		line=1;
		column=1;
		length=0;
		states.clear();
		symbols.clear();
		variables.clear();
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
	
	/**
	 * 移入状态。
	 * 
	 * @param state 状态。
	 * @param symbol 符号。
	 */
	void shift(int state,SyntaxSymbol symbol)
	{
		states.push(state);
		symbols.push(symbol);
	}
	
	/**
	 * 归约非终结符。
	 * 
	 * @param noterminal 非终结符。
	 * @param symbol 向前看符号。
	 */
	void reduce(String noterminal,SyntaxSymbol symbol)
	{
		
	}
}
