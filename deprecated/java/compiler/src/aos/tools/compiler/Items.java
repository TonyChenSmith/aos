/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
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
final class Items implements Serializable
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=-8363082184008015867L;
	
	/**
	 * 项集。以所有开始符号分别列出的项进行排列。
	 */
	private final Map<String,Set<Item>> items;
	
	/**
	 * 项集中包含的产生式。
	 */
	private final Set<String> productions;
	
	/**
	 * 构造初始项集。
	 * 
	 * @param context 上下文。
	 */
	Items(Context context)
	{
		this(context,Set.of(new Item(context.productions.get("%start"),"$eof")));
	}
	
	/**
	 * 以输入项作为根源项，搜寻其等价项构造该项集。
	 * 
	 * @param context 上下文。
	 * @param item 主项。
	 */
	Items(Context context,Set<Item> items)
	{
		Map<String,Set<Item>> ritems=new HashMap<>();
		//非终结符号集。用于提取出需要添加的项。
		Set<String> noterminals=new HashSet<>();
		productions=new HashSet<>();
		boolean nochange=true;
		while(true)
		{
			noterminals.clear();
			Set<Item> citems=new HashSet<>();
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
				for(Item item:items)
				{
					if(item.first().contains(noterminal))
					{
						Set<String> nfollow=item.follow(noterminal,context);
						Production np=context.productions.get(noterminal);
						for(String nlookahead:nfollow)
						{
							if(citems.add(new Item(np,nlookahead)))
							{
								nochange=false;
							}
						}
					}
				}
			}
			items=citems;
			if(nochange)
			{
				break;
			}
			else
			{
				nochange=true;
			}
		}
		for(Item each:items)
		{
			productions.add(each.getProduction().getSymbol());
			Set<String> first=each.first();
			for(String symbol:first)
			{
				if(ritems.containsKey(symbol))
				{
					ritems.get(symbol).add(each);
				}
				else
				{
					Set<Item> list=new HashSet<>();
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
	
	/**
	 * 获得首符号集。
	 * 
	 * @return 总首符号集。
	 */
	Set<String> first()
	{
		return new HashSet<>(items.keySet());
	}
	
	/**
	 * 获得可归约集。
	 * 
	 * @return 可归约集。
	 */
	Set<Item> reducedItems()
	{
		return items.getOrDefault("$end",Set.of());
	}
	
	/**
	 * 获得下一项集。调用前要求确认非全规约集。
	 * 
	 * @param context 上下文。
	 * @param symbol 匹配符号。
	 * @return 下一项集。
	 */
	Items match(Context context,String symbol)
	{
		Set<Item> set=new HashSet<>();
		for(Item item:items.get(symbol))
		{
			set.add(item.match(symbol));
		}
		return new Items(context,set);
	}
	
	/**
	 * 查询项集是否包含该产生式。
	 * 
	 * @param symbol 产生式名。
	 * @return
	 */
	boolean contains(String symbol)
	{
		return productions.contains(symbol);
	}
	
	@Override
	public int hashCode()
	{
		return items.hashCode();
	}
	
	@Override
	public String toString()
	{
		return items.toString();
	}
	
	@Override
	public boolean equals(Object obj)
	{
		if(obj instanceof Items i)
		{
			return items.equals(i.items);
		}
		return false;
	}
}
