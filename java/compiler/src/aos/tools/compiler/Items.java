package aos.tools.compiler;

import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;

/**
 * 项集，代表一个产生式及其等价产生式的状态。
 * 
 * <p>在2024-08-16时生成。
 *
 * @author Tony Chen Smith
 */
final class Items
{
	/**
	 * 项集。以所有开始符号分别列出的项进行排列。
	 */
	private final Map<String,List<Item>> items;
	
	/**
	 * 以输入项作为根源项，搜寻其等价项构造该项集。
	 * 
	 * @param context 上下文。
	 * @param item 主项。
	 */
	Items(Context context,List<Item> items)
	{
		Map<String,List<Item>> ritems=new HashMap<>();
		Set<Item> citems=new HashSet<>();
		//非终结符号集。用于提取出需要添加的项。
		Set<String> noterminals=new HashSet<>();
		for(Item item:items)
		{
			citems.add(item);
			//非终结符号集。用于提取出需要添加的项。
			Set<String> all=new TreeSet<>(context.productions.keySet());
			all.retainAll(item.first());
			noterminals.addAll(all);
		}
		for(String noterminal:noterminals)
		{
			Set<String> current=new HashSet<>();
			for(Item item:items)
			{
				if(item.first().contains(noterminal))
				{
					Set<String> nfollow=item.follow(noterminal);
					Production np=context.productions.get(noterminal);
					for(String nlookahead:nfollow)
					{
						if(!current.contains(nlookahead))
						{
							items.add(new Item(np,nlookahead));
							current.add(nlookahead);
						}
					}
				}
			}
		}
		for(Item each:citems)
		{
			Set<String> first=each.first();
			for(String symbol:first)
			{
				if(ritems.containsKey(symbol))
				{
					ritems.get(symbol).add(each);
				}
				else
				{
					List<Item> list=new LinkedList<>();
					list.add(each);
					ritems.put(symbol,list);
				}
			}
		}
		this.items=ritems;
	}
	
	/**
	 * 匹配符号，除了结束符号。
	 * 
	 * @param symbol 符号。
	 * @param context 上下文。
	 */
	void process(SyntaxSymbol symbol,Context context)
	{
		for(Item item:items.get(symbol.name()))
		{
			item.process(symbol,context);
		}
	}
}
