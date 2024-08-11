package aos.tools.compiler;

import java.io.Serializable;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.stream.IntStream;

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
	private final Transform[][] transforms;
	
	/**
	 * 以两数组构造产生式。
	 * 
	 * @param transforms
	 * @param epsilons
	 */
	private Production(String name,Transform[][] transforms,int[][] epsilons)
	{
		this.transforms=transforms;
		this.epsilons=epsilons;
		this.name=name;
	}
	
	/**
	 * 给项调用的。将当前状态进行匹配，得下一结点的集。
	 * 
	 * @param current 当前结点集。
	 * @param symbol 将用于匹配的语法符号。
	 * @return 下一状态集。
	 */
	Set<Integer> match(Set<Integer> current,SyntaxSymbol symbol)
	{
		Set<Integer> result=new HashSet<>();
		for(int node:current)
		{
			for(Transform transform:transforms[node])
			{
				switch(transform.match(symbol))
				{
					case MATCH:
						result.addAll(IntStream.of(epsilons[transform.toDest()]).boxed().toList());
						break;
					case MISMATCH:
						//无事发生。
						break;
					default:
						//不可达。
						break;
				}
			}
		}
		return result;
	}
	
	@Override
	public String toString()
	{
		final StringBuilder result=new StringBuilder();
		result.append("Production:").append(name).append('\n');
		result.append("Node:").append(transforms.length).append('\n');
		for(int i=0;i<transforms.length;i++)
		{
			result.append(i).append(":({");
			for(int j=0;j<transforms[i].length;j++)
			{
				result.append(transforms[i][j]);
				if(j<transforms[i].length-1)
				{
					result.append(',');
				}
			}
			result.append("},[");
			for(int j=0;j<epsilons[i].length;j++)
			{
				result.append(epsilons[i][j]);
				if(j<epsilons[i].length-1)
				{
					result.append(',');
				}
			}
			result.append("])");
			if(i<transforms.length-1)
			{
				result.append('\n');
			}
		}
		return result.toString();
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
			final Transform[][] rts=new Transform[transforms.length][];
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
				rts[index]=transforms[index].toArray(Transform[]::new);
			}
			return new Production(name,rts,res);
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
		 * @return 对应产生式图。
		 */
		Builder union(Builder builder)
		{
			return union(this,builder);
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
		 */
		private Transform(String symbol,boolean isEpsilon,int dest)
		{
			this.symbol=symbol;
			this.isEpsilon=isEpsilon;
			this.dest=dest;
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
				return symbol.equals(transform.symbol)&&dest==transform.dest;
			}
			return false;
		}
		
		@Override
		public int hashCode()
		{
			return symbol.hashCode()^Integer.hashCode(dest);
		}
		
		@Override
		public String toString()
		{
			return new StringBuilder().append('[').append(symbol).append("]->").append(dest).toString();
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
		 * 比较输入符号是否匹配该变换。调用该方法时，正常不存在空变换。
		 * 
		 * @param symbol 语法符号。
		 * 
		 * @return 返回匹配结果。
		 */
		MatchResult match(SyntaxSymbol symbol)
		{
			return this.symbol.equals(symbol.name())?MatchResult.MATCH:MatchResult.MISMATCH;
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
		 * 获得期望符号。
		 * 
		 * @return 期望符号名。
		 */
		String getSymbol()
		{
			return symbol;
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
			return new Transform(EPSILON,true,dest);
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
			return new Transform(symbol,EPSILON.equals(symbol),dest);
		}
	}
	
	/**
	 * 变换进行匹配后的结果。
	 * 
	 * <p>在2024-08-11时生成。
	 *
	 * @author Tony Chen Smith
	 */
	static enum MatchResult
	{
		/**
		 * 匹配。
		 */
		MATCH,
		
		/**
		 * 不匹配。
		 */
		MISMATCH
	}
}
