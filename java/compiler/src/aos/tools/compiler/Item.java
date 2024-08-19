package aos.tools.compiler;

import java.util.Arrays;
import java.util.Set;

/**
 * 项。代表一个产生式的当前状态。
 * 
 * <p>在2024-08-15时生成。
 *
 * @author Tony Chen Smith
 */
final class Item
{
	/**
	 * 归约向前看符号。
	 */
	private final String lookahead;
	
	/**
	 * 期待的首符号集。
	 */
	private final Set<String> first;
	
	/**
	 * 对应产生式。
	 */
	private final Production production;
	
	/**
	 * 结点集。
	 */
	private final int[] nodes;
	
	/**
	 * 构造一个产生式的初始项。
	 * 
	 * @param production 产生式。
	 * @param lookahead 向前看符号。
	 */
	Item(Production production,String lookahead)
	{
		this(production,production.start(),lookahead);
	}
	
	/**
	 * 构造一个产生式的项。
	 * 
	 * @param production 产生式。
	 * @param nodes 结点集。
	 * @param lookahead 向前看符号。
	 */
	private Item(Production production,int[] nodes,String lookahead)
	{
		this.lookahead=lookahead;
		this.production=production;
		this.nodes=nodes;
		first=Set.copyOf(production.first(nodes));
	}
	
	/**
	 * 获得首符号集。
	 * 
	 * @return 首符号集。
	 */
	Set<String> first()
	{
		return first;
	}
	
	/**
	 * 获得向前看符号。
	 * 
	 * @return 向前看符号。
	 */
	String lookahead()
	{
		return lookahead;
	}
	
	/**
	 * 匹配符号。
	 * 
	 * @param symbol 语法符号。
	 * @param context 上下文。
	 */
	void process(SyntaxSymbol symbol,Context context)
	{
		production.process(nodes,symbol,context);
	}
	
	/**
	 * 是否为归约项。
	 * 
	 * @param lookahead 向前看符号。
	 * @return 为规约项则返回真。
	 */
	boolean isReducedItem(String lookahead)
	{
		return production.isEnd(nodes)&&this.lookahead.equals(lookahead);
	}
	
	/**
	 * 获得非终结符号后续终结符号集。
	 * 
	 * @param symbol 非终结符号。
	 * @param context 上下文。
	 * @return 终结符号集。
	 */
	Set<String> follow(String symbol,Context context)
	{
		return production.followSymbols(nodes,symbol,lookahead,context);
	}
	
	/**
	 * 获得对应的产生式。
	 * 
	 * @return 产生式。
	 */
	Production getProduction()
	{
		return production;
	}
	
	/**
	 * 获得匹配终结符号后的下一项。
	 * 
	 * @param symbol 终结符号。
	 * @return 后续项。
	 */
	Item match(String symbol)
	{
		return new Item(production,production.follow(nodes,symbol),lookahead);
	}
	
	@Override
	public String toString()
	{
		StringBuilder result=new StringBuilder();
		result.append('{').append(production.getSymbol()).append(',');
		result.append(Arrays.toString(nodes)).append(',').append(lookahead).append('}');
		return result.toString();
	}
	
	@Override
	public int hashCode()
	{
		return Arrays.hashCode(nodes)^production.hashCode()^lookahead.hashCode();
	}
	
	@Override
	public boolean equals(Object obj)
	{
		if(obj instanceof Item i)
		{
			return production.equals(i.production)&&Arrays.equals(nodes,i.nodes)&&lookahead.equals(i.lookahead);
		}
		return false;
	}
}
