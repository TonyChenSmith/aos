package aos.tools.compiler;

import java.io.Serializable;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * LR(1)语法分析表。
 * 
 * <p>在2024-08-17时生成。
 *
 * @author Tony Chen Smith
 */
final class LRTable implements Serializable
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=7753171956125291759L;
	
	/**
	 * 分析表。
	 */
	private final Map<String,Action>[] table;
	
	/**
	 * 项集族。
	 */
	private final Items[] items;
	
	/**
	 * 非终结符集，用于信息整理输出。
	 */
	private final Set<String> noterminals;
	
	/**
	 * 构造LR(1)分析表。
	 * 
	 * @param table 分析表。
	 * @param items 项集族。
	 * @param noterminals 非终结符集。
	 */
	private LRTable(Map<String,Action>[] table,Items[] items,Set<String> noterminals)
	{
		this.table=table;
		this.items=items;
		this.noterminals=noterminals;
	}
	
	@Override
	public String toString()
	{
		Table<Integer,String,String> result=new Table<>(true,false);
		for(int index=0;index<items.length;index++)
		{
			for(Map.Entry<String,Action> entry:table[index].entrySet())
			{
				if(!noterminals.contains(entry.getKey()))
				{
					result.put(index,entry.getKey(),entry.getValue().toText());
				}
			}
		}
		for(int index=0;index<items.length;index++)
		{
			for(Map.Entry<String,Action> entry:table[index].entrySet())
			{
				if(noterminals.contains(entry.getKey()))
				{
					result.put(index,entry.getKey(),entry.getValue().toText());
				}
			}
		}
		return result.toString();
	}
	
	/**
	 * 获取行为。
	 * 
	 * @param state  状态。
	 * @param symbol 符号。
	 * 
	 * @return 表中的行为，或者未知错误。
	 */
	Action get(int state,String symbol)
	{
		return table[state].getOrDefault(symbol,null);
	}
	
	/**
	 * 获取项集。
	 * 
	 * @param state 状态。
	 * 
	 * @return 对应项集。
	 */
	Items getState(int state)
	{
		return items[state];
	}
	
	/**
	 * 设置错误行为。
	 * 
	 * @param state   状态。
	 * @param symbol  符号。
	 * @param message 错误信息。
	 */
	void setError(int state,String symbol,String message)
	{
		table[state].putIfAbsent(symbol,null);
	}
	
	/**
	 * 由上下文构造LR(1)分析表。
	 * 
	 * @param context
	 * 
	 * @return
	 */
	@SuppressWarnings("unchecked")
	static LRTable create(Context context)
	{
		Node start=new Node(new Items(context));
		List<Node> states=new LinkedList<>();
		start.dfs(context,states);
		
		// 构造LR1分析表。
		Map<String,Action>[] table=(Map<String,Action>[])new Map<?,?>[states.size()];
		Items[] items=new Items[table.length];
		Set<String> set=new HashSet<>();
		for(int index=0;index<table.length;index++)
		{
			Node n=states.get(index);
			items[index]=n.items;
			Map<String,Action> m=new HashMap<>();
			table[index]=m;
			for(Map.Entry<String,Integer> entry:n.map.entrySet())
			{
				if(context.productions.containsKey(entry.getKey()))
				{
					set.add(entry.getKey());
					m.put(entry.getKey(),new Action(ActionType.GOTO,entry.getValue()));
				}
				else
				{
					m.put(entry.getKey(),new Action(ActionType.SHIFT,entry.getValue()));
				}
			}
			for(Item item:n.items.reducedItems())
			{
				if(item.getProduction().getSymbol().equals("%start"))
				{
					m.put("$eof",new Action(ActionType.ACCEPT,"%start"));
				}
				else
				{
					m.put(item.lookahead(),new Action(ActionType.REDUCE,item.getProduction().getSymbol()));
				}
			}
		}
		return new LRTable(table,items,Set.copyOf(set));
	}
	
	/**
	 * 结点，用于深度遍历搜索时存储记录。
	 * 
	 * <p>在2024-08-17时生成。
	 *
	 * @author Tony Chen Smith
	 */
	private static final class Node
	{
		/**
		 * 结点映射。
		 */
		Map<String,Integer> map;
		
		/**
		 * 项集。
		 */
		Items items;
		
		/**
		 * 构造结点。
		 * 
		 * @param items 项集。
		 */
		Node(Items items)
		{
			this.items=items;
			map=new HashMap<>();
		}
		
		/**
		 * 深度优先遍历。
		 * 
		 * @param context 上下文。
		 * @param states  状态列表。
		 */
		void dfs(Context context,List<Node> states)
		{
			states.add(this);
			for(String symbol:items.first())
			{
				if(symbol.equals("$end"))
				{
					// 归约终结。
					continue;
				}
				else
				{
					Items i=items.match(context,symbol);
					boolean included=false;
					for(int index=0;index<states.size();index++)
					{
						if(i.equals(states.get(index).items))
						{
							map.put(symbol,index);
							included=true;
							break;
						}
					}
					if(!included)
					{
						Node n=new Node(i);
						map.put(symbol,states.size());
						n.dfs(context,states);
					}
				}
			}
		}
		
		@Override
		public String toString()
		{
			return new StringBuilder(items.toString()).append('\n').append(map).toString();
		}
	}
	
	/**
	 * 行为枚举。
	 * 
	 * <p>在2024-08-19时生成。
	 *
	 * @author Tony Chen Smith
	 */
	static enum ActionType
	{
		/**
		 * 移入。
		 */
		SHIFT,
		
		/**
		 * 归约。
		 */
		REDUCE,
		
		/**
		 * 跳转。
		 */
		GOTO,
		
		/**
		 * 接受。
		 */
		ACCEPT,
		
		/**
		 * 错误。
		 */
		ERROR
	}
	
	/**
	 * 行为记录。
	 * 
	 * <p>在2024-08-19时生成。
	 *
	 * @author Tony Chen Smith
	 */
	static record Action(ActionType type,Object param)
	{
		/**
		 * 未知错误。
		 */
		static Action UNKNOWN=new Action(ActionType.ERROR,"Unknown error.");
		
		/**
		 * 变成字符串。
		 * 
		 * @return 行为对应字符串。
		 */
		 String toText()
		 {
			 switch(type)
			 {
				case ACCEPT:
					return "acc";
				case ERROR:
					return "err\"%s\"".formatted(param);
				case GOTO:
					return "g%d".formatted(param);
				case REDUCE:
					return "r%s".formatted(param);
				case SHIFT:
					return "s%d".formatted(param);
				default:
					return "";
			 }
		 }
	}
}
