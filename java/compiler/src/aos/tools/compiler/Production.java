/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

import java.io.Serializable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

/**
 * 产生式，一个DFA条件有向图。
 * 
 * <p>它的状态在语法分析中称为项。将匹配的语法符号输入到产生式可以构造一个非终结符号。
 * 
 * <p>其有编译后保留数据需求，实现序列化接口。
 * 
 * <p>在2024-08-10时生成。
 *
 * @author Tony Chen Smith
 */
final class Production implements Serializable
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=-4469724252320070110L;
	
	/**
	 * 空变换集。
	 */
	private final int[][] epsilons;
	
	/**
	 * 产生式产生的非终结符号名。
	 */
	private final String name;
	
	/**
	 * 变换数组。
	 */
	private final Transform[] transforms;
	
	/**
	 * 以两数组构造产生式。
	 * 
	 * @param transforms
	 * @param epsilons
	 */
	private Production(String name,Transform[] transforms,int[][] epsilons)
	{
		this.transforms=transforms;
		this.epsilons=epsilons;
		this.name=name;
	}
	
	@Override
	public String toString()
	{
		final StringBuilder result=new StringBuilder();
		result.append("Production:").append(name).append('\n');
		result.append("Node:").append(transforms.length).append('\n');
		for(int i=0;i<transforms.length;i++)
		{
			result.append(i).append(":({").append(transforms[i]==null?"":transforms[i]).append("},{");
			for(int j=0;j<epsilons[i].length;j++)
			{
				result.append(epsilons[i][j]);
				if(j<epsilons[i].length-1)
				{
					result.append(',');
				}
			}
			result.append("})");
			if(i<transforms.length-1)
			{
				result.append('\n');
			}
		}
		return result.toString();
	}
	
	@Override
	public int hashCode()
	{
		return name.hashCode();
	}
	
	@Override
	public boolean equals(Object obj)
	{
		if(obj instanceof Production p)
		{
			return name.equals(p.name);
		}
		return false;
	}
	
	/**
	 * 获得产生式对应的非终结符号。
	 * 
	 * @return 非终结符号名。
	 */
	String getSymbol()
	{
		return name;
	}
	
	/**
	 * 获得结点数量。
	 * 
	 * @return 结点数量。
	 */
	int count()
	{
		return transforms.length;
	}
	
	/**
	 * 获得当前结点集对应首符号。
	 * 
	 * @param current 当前结点集。
	 * 
	 * @return 首符号集。
	 */
	Set<String> first(int[] current)
	{
		Set<String> result=new TreeSet<>();
		for(final int node:symbols(current))
		{
			if(node==transforms.length-1)
			{
				//内部结束符号。
				result.add("$end");
			}
			else
			{
				result.add(transforms[node].symbol);
			}
		}
		return result;
	}
	
	/**
	 * 获得有符号变换的集。
	 * 
	 * @param current 当前集。
	 * @return 符号变换集。
	 */
	int[] symbols(int[] current)
	{
		final IntSet result=new IntSet();
		for(final int node:current)
		{
			if(node==transforms.length-1)
			{
				result.add(node);
			}
			else if(transforms[node].isAction())
			{
				result.add(symbols(epsilons[transforms[node].toDest()]));
			}
			else
			{
				result.add(node);
			}
		}
		return result.get();
	}
	
	/**
	 * 判断当前状态是否到达结尾。
	 * 
	 * @param current 当前结点集。
	 * 
	 * @return 如果有结尾结点返回真。
	 */
	boolean isEnd(int[] current)
	{
		return new IntSet(symbols(current)).contain(transforms.length-1);
	}
	
	/**
	 * 执行产生式。仅在成功匹配该产生式时调用。
	 * 
	 * @param current 当前结点集。
	 * @param symbol 符号。
	 * @param context 上下文。
	 */
	void process(int[] current,SyntaxSymbol symbol,Context context)
	{
		for(int node:current)
		{
			if(node==transforms.length-1)
			{
				continue;
			}
			if(transforms[node].process(symbol,context))
			{
				process(epsilons[transforms[node].toDest()],symbol,context);
			}
		}
	}
	
	/**
	 * 获得当前结点集情况下，匹配到输入符号后的后续集。
	 * 
	 * @param current 当前结点集。
	 * @param symbol 目标符号。
	 * @return 后续结点集。
	 */
	int[] follow(int[] current,String symbol)
	{
		final IntSet set=new IntSet();
		for(int node:symbols(current))
		{
			if(node==transforms.length-1)
			{
				//跳过尾结点。调用该方法说明选择继续移入。
				continue;
			}
			else if(transforms[node].isAction())
			{
				set.add(follow(epsilons[transforms[node].toDest()],symbol));
			}
			else if(transforms[node].symbol.equals(symbol))
			{
				set.add(epsilons[transforms[node].toDest()]);
			}
		}
		return set.get();
	}
	
	/**
	 * 获得当前结点集情况下，匹配到输入符号后的后续符号集。如果遇到结尾则替换为向前看符号。
	 * 
	 * @param current 当前结点集。
	 * @param symbol 目标符号。
	 * @param lookahead 向前看符号。
	 * @param context 上下文。
	 * @return 后续符号集。
	 */
	Set<String> followSymbols(int[] current,String symbol,String lookahead,Context context)
	{
		Set<String> set=first(follow(current,symbol));
		Set<String> result=new HashSet<>();
		if(set.contains("$end"))
		{
			set.remove("$end");
			set.add(lookahead);
		}
		for(String sym:set)
		{
			if(context.productions.containsKey(sym))
			{
				result.addAll(context.productions.get(sym).firstSymbol(context));
			}
			else
			{
				result.add(sym);
			}
		}
		return result;
	}
	
	/**
	 * 开始结点的终结符号集。
	 * 
	 * @param context 上下文。
	 * @return 开始终结符集。
	 */
	Set<String> firstSymbol(Context context)
	{
		Set<String> set=first(epsilons[0]);
		Set<String> result=new HashSet<>();
		for(String symbol:set)
		{
			if(context.productions.containsKey(symbol))
			{
				result.addAll(context.productions.get(symbol).firstSymbol(context));
			}
			else
			{
				result.add(symbol);
			}
		}
		return result;
	}
	
	/**
	 * 获得起始结点集。
	 * 
	 * @return 索引0的结点集。
	 */
	int[] start()
	{
		return epsilons[0].clone();
	}
	
	/**
	 * 变换第二选项。语法翻译行为。
	 * 
	 * <p>在2024-08-13时生成。
	 *
	 * @author Tony Chen Smith
	 */
	static final class Action implements Serializable
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=-3185826060855540265L;
		
		void action(Context context)
		{
			
		}
	}
	
	/**
	 * 产生式建造器。用于建造产生式体。
	 * 
	 * <p>其为带空变换的有向图集。
	 * 
	 * <p>在2024-08-10时生成。
	 *
	 * @author Tony Chen Smith
	 */
	static final class Builder
	{
		/**
		 * 变换数组。
		 */
		private final List<Transform>[] transforms;
		
		/**
		 * 以结点数构造有向图。
		 * 
		 * @param count 结点数
		 */
		@SuppressWarnings("unchecked")
		private Builder(int count)
		{
			transforms=(List<Transform>[])new List<?>[count];
			for(int index=0;index<count;index++)
			{
				transforms[index]=new LinkedList<>();
			}
		}
		
		@Override
		public String toString()
		{
			final StringBuilder result=new StringBuilder();
			result.append("Node:").append(count()).append('\n');
			for(int i=0;i<transforms.length;i++)
			{
				result.append(i).append(":{");
				final Iterator<Transform> iter=transforms[i].iterator();
				while(iter.hasNext())
				{
					result.append(iter.next());
					if(iter.hasNext())
					{
						result.append(',');
					}
				}
				result.append('}');
				if(i<transforms.length-1)
				{
					result.append('\n');
				}
			}
			return result.toString();
		}
		
		/**
		 * 构造产生式。
		 * 
		 * @param name 产生式所产生的非终结符号名。
		 * 
		 * @return 返回产生式。
		 */
		@SuppressWarnings("unchecked")
		Production build(String name)
		{
			final List<Transform>[] transforms=(List<Transform>[])new List<?>[this.transforms.length];
			final Set<Integer>[] epsilons=(Set<Integer>[])new Set<?>[this.transforms.length];
			for(int index=0;index<this.transforms.length;index++)
			{
				transforms[index]=new LinkedList<>();
				epsilons[index]=new HashSet<>();
				for(final Transform transform:this.transforms[index])
				{
					if(transform.isEpsilon())
					{
						dfs(epsilons[index],transform.toDest());
					}
					else
					{
						transforms[index].add(transform);
					}
				}
				epsilons[index].add(index);
			}
			final Transform[] rts=new Transform[transforms.length];
			final int[][] res=new int[epsilons.length][];
			for(int index=0;index<this.transforms.length;index++)
			{
				final Set<Integer> subset=new HashSet<>();
				for(final int node:epsilons[index])
				{
					if(!transforms[node].isEmpty()||node==transforms.length-1)
					{
						subset.add(node);
					}
				}
				res[index]=subset.stream().mapToInt(Integer::intValue).sorted().toArray();
				final Transform[] tts=transforms[index].toArray(Transform[]::new);
				rts[index]=tts.length==0?null:tts[0];
			}
			return new Production(name,rts,res);
		}
		
		/**
		 * 产生该图闭包。
		 * 
		 * @return 对应产生式图。
		 */
		Builder closure()
		{
			return closure(this);
		}
		
		/**
		 * 将该图和另一个图连接。
		 * 
		 * @param builder 另一个图。
		 * 
		 * @return 对应产生式图。
		 */
		Builder concat(Builder builder)
		{
			return concat(this,builder);
		}
		
		/**
		 * 将该图和另一个图并联。
		 * 
		 * @param builder 另一个图。
		 * 
		 * @return 对应产生式图。
		 */
		Builder union(Builder builder)
		{
			return union(this,builder);
		}
		
		/**
		 * 在指定位置添加一个变换。
		 * 
		 * @param index     索引。
		 * @param transform 变换。
		 */
		private void addTransform(int index,Transform transform)
		{
			transforms[index].add(transform);
		}
		
		/**
		 * 在指定开始位置添加一系列的变换。
		 * 
		 * @param start      开始索引。
		 * @param transforms 变换数组。
		 */
		private void addTransforms(int start,List<Transform>[] transforms)
		{
			for(int offset=0;offset<transforms.length;offset++)
			{
				for(final Transform transform:transforms[offset])
				{
					if(transform.isEpsilon())
					{
						this.transforms[start+offset].add(Transform.epsilon(transform.toDest()+start));
					}
					else if(transform.isAction())
					{
						this.transforms[start+offset].add(Transform.action(transform.action,transform.toDest()+start));
					}
					else
					{
						this.transforms[start+offset].add(Transform.transform(transform.symbol,transform.toDest()+start));
					}
				}
			}
		}
		
		/**
		 * 返回结点数目。
		 * 
		 * @return 结点数。
		 */
		private int count()
		{
			return transforms.length;
		}
		
		/**
		 * 深度优先遍历空变换，将目标索引存入集中。
		 * 
		 * @param nodes 结点集。
		 */
		private void dfs(Set<Integer> nodes,int node)
		{
			if(nodes.add(node))
			{
				for(final Transform transform:transforms[node])
				{
					if(transform.isEpsilon())
					{
						dfs(nodes,transform.toDest());
					}
				}
			}
		}
		
		static Builder action()
		{
			// 开始 结束。
			final Builder result=new Builder(2);
			result.addTransform(0,Transform.action(null,1));
			return result;
		}
		
		/**
		 * 将a图闭包。
		 * 
		 * @param a 有向图a。结果的前端。
		 * 
		 * @return 对应的产生式有向图。
		 */
		static Builder closure(Builder a)
		{
			final Builder result=new Builder(a.count()+2);
			result.addTransform(0,Transform.epsilon(1));
			result.addTransform(0,Transform.epsilon(1+a.count()));
			result.addTransforms(1,a.transforms);
			result.addTransform(a.count(),Transform.epsilon(1));
			result.addTransform(a.count(),Transform.epsilon(a.count()+1));
			return result;
		}
		
		/**
		 * 将a图和b图连接。
		 * 
		 * @param a 有向图a。结果的前端。
		 * @param b 有向图b。结果的后端。
		 * 
		 * @return 对应的产生式有向图。
		 */
		static Builder concat(Builder a,Builder b)
		{
			final Builder result=new Builder(a.count()+b.count()-1);
			result.addTransforms(0,a.transforms);
			result.addTransforms(a.count()-1,b.transforms);
			return result;
		}
		
		/**
		 * 构造一个符号变换。符号应该是个终结符或者非终结符。
		 * 
		 * @param symbol 符号名。
		 * 
		 * @return 对应的产生式有向图。
		 */
		static Builder create(String symbol)
		{
			// 开始 结束。
			final Builder result=new Builder(2);
			result.addTransform(0,Transform.transform(symbol,1));
			return result;
		}
		
		/**
		 * 将a图和b图并联。
		 * 
		 * @param a 有向图a。结果的前分支。
		 * @param b 有向图b。结果的后分支。
		 * 
		 * @return 对应的产生式有向图。
		 */
		static Builder union(Builder a,Builder b)
		{
			final Builder result=new Builder(a.count()+b.count()+1);
			result.addTransform(0,Transform.epsilon(1));
			result.addTransform(0,Transform.epsilon(1+a.count()));
			result.addTransforms(1,a.transforms);
			result.addTransform(a.count(),Transform.epsilon(a.count()+b.count()));
			result.addTransforms(1+a.count(),b.transforms);
			return result;
		}
	}
	
	/**
	 * 变换。这里指语法符号变换。
	 * 
	 * <p>其有编译后保留数据需求，实现序列化接口。
	 * 
	 * <p>在2024-08-10时生成。
	 *
	 * @author Tony Chen Smith
	 */
	static final class Transform implements Serializable
	{
		/**
		 * 空变换符号名。
		 */
		private static final String EPSILON="$epsilon";
		
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=1838123642740761439L;
		
		/**
		 * 语法制导行为。
		 */
		private final Action action;
		
		/**
		 * 目的索引。变换后状态数值。
		 */
		private final int dest;
		
		/**
		 * 是否为空变换。
		 */
		private final boolean isEpsilon;
		
		/**
		 * 符号名。用于变换条件比对。
		 */
		private final String symbol;
		
		/**
		 * 构造一个语法符号变换。
		 * 
		 * @param symbol    语法符号名。
		 * @param isEpsilon 是否为空变换。
		 * @param dest      目的索引。
		 * @param action    语法翻译行为。
		 */
		private Transform(String symbol,boolean isEpsilon,int dest,Action action)
		{
			this.symbol=symbol;
			this.isEpsilon=isEpsilon;
			this.dest=dest;
			this.action=action;
		}
		
		@Override
		public boolean equals(Object obj)
		{
			if(obj instanceof final Transform transform)
			{
				if(isEpsilon)
				{
					return isEpsilon==transform.isEpsilon&&dest==transform.dest;
				}
				if(action!=null)
				{
					return symbol.equals(transform.symbol)&&dest==transform.dest&&action.equals(transform.action);
				}
				return symbol.equals(transform.symbol)&&dest==transform.dest;
			}
			return false;
		}
		
		@Override
		public int hashCode()
		{
			if(action!=null)
			{
				return symbol.hashCode()^Integer.hashCode(dest)^action.hashCode();
			}
			return symbol.hashCode()^Integer.hashCode(dest);
		}
		
		@Override
		public String toString()
		{
			final StringBuilder result=new StringBuilder().append('[').append(symbol).append("]->").append(dest);
			if(action!=null)
			{
				result.append('{').append(action).append('}');
			}
			return result.toString();
		}
		
		/**
		 * 是否为语法翻译行为。
		 * 
		 * @return 行为返回真。
		 */
		boolean isAction()
		{
			return action!=null;
		}
		
		/**
		 * 是否为空变换。
		 * 
		 * @return 空变换返回真。
		 */
		boolean isEpsilon()
		{
			return isEpsilon;
		}
		
		/**
		 * 匹配过程。
		 * 
		 * @param symbol 语法符号。
		 * @param context 上下文。
		 * @return 有动作则返回真。
		 */
		boolean process(SyntaxSymbol symbol,Context context)
		{
			if(action!=null)
			{
				action.action(context);
				return true;
			}
			return false;
		}
		
		/**
		 * 匹配成功时调用，返回目的索引。
		 * 
		 * @return 目标索引。
		 */
		int toDest()
		{
			return dest;
		}
		
		/**
		 * 构造一个语法翻译行为。
		 * 
		 * @param action 行为。
		 * @param dest   目标索引。
		 * 
		 * @return 对应的行为变换。
		 */
		static Transform action(Action action,int dest)
		{
			return new Transform("$action",false,dest,action);
		}
		
		/**
		 * 构造一个空变化。
		 * 
		 * @param dest 目标索引。
		 * 
		 * @return 对应的空变换。
		 */
		static Transform epsilon(int dest)
		{
			return new Transform(EPSILON,true,dest,null);
		}
		
		/**
		 * 构造一个符号变换。
		 * 
		 * @param symbol 语法符号名。
		 * @param dest   目标索引。
		 * 
		 * @return 对应的符号变换。
		 */
		static Transform transform(String symbol,int dest)
		{
			return new Transform(symbol,EPSILON.equals(symbol),dest,null);
		}
	}
}
