package aos.library.regex;

import java.util.Collections;
import java.util.Deque;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

/**
 * 不确定有穷自动机。
 * <p>
 * 该类封装了所有不确定有穷自动机实现的功能，但仅对其他类公开图搜索功能。
 * <p>
 * 由于进行了优化，从有向图生成NFA的过程不可逆。优化程度本质上将NFA优化成了DFA。
 * <p>
 * 其中，所有对象具有不可比较的特性。仅epsilon因为唯一的谓词可以进行判断。
 * <p>
 * 在2024-06-17时生成。<br>
 * @author Tony Chen Smith
 */
final class NFA
{
	/**
	 * 非空变换数组。
	 */
	private final List<Transform>[] transforms;
	
	/**
	 * 空变换集。
	 */
	private final Set<Integer>[] epsilons;
	
	/**
	 * 通过完整有向图变换数组，构造NFA（实际是半个DFA）。
	 * 
	 * @param transforms 变换数组。
	 */
	@SuppressWarnings("unchecked")
	private NFA(List<Transform>[] transforms)
	{
		this.transforms=(List<Transform>[])new List<?>[transforms.length+1];
		this.epsilons=(Set<Integer>[])new Set<?>[transforms.length+1];
		List<Transform> nonull=new LinkedList<>();
		Deque<Integer> stack=new LinkedList<>();
		for(int index=0;index<transforms.length;index++)
		{
			this.epsilons[index]=new HashSet<>();
			for(Transform trans:transforms[index])
			{
				if(trans.isEpsilonTransform())
				{
					stack.push(trans.transform(index));
				}
				else
				{
					nonull.add(trans);
				}
			}
			//自可达
			this.epsilons[index].add(index);
			while(!stack.isEmpty())
			{
				//栈不为空视为有epsilon变换。
				int node=stack.pop();
				if(this.epsilons[index].add(node))
				{
					if(node==transforms.length)
					{
						//尾结点，不存在后续变换
					}
					else
					{
						for(Transform trans:transforms[node])
						{
							if(trans.isEpsilonTransform())
							{
								stack.push(trans.transform(node));
							}
						}
					}
				}
				else
				{
					//已经搜索过的结点跳过处理。
					continue;
				}
			}
			this.transforms[index]=List.copyOf(nonull);
			this.epsilons[index]=Set.copyOf(this.epsilons[index]);
			nonull.clear();
			stack.clear();
		}
		//构造尾结点
		epsilons[transforms.length]=Set.of(transforms.length);
		this.transforms[transforms.length]=Collections.emptyList();
	}
	
	/**
	 * 获得指定结点可达结点集。
	 * 
	 * @param node 指定结点。
	 * @return 可达集。
	 */
	private Set<Integer> get(int node)
	{
		if(node>=epsilons.length)
		{
			//异常情况，返回空集。
			return Collections.emptySet();
		}
		else
		{
			return epsilons[node];
		}
	}
	
	/**
	 * 获得指定结点对指定码元的变换结果，结果为一个可达集。
	 * 
	 * @param node 指定结点。
	 * @param codePoint 码元。
	 * @return 变换后结点的可达集。
	 */
	private Set<Integer> transform(int node,int codePoint)
	{
		if(node>=epsilons.length)
		{
			//异常情况，返回空集。
			return Collections.emptySet();
		}
		for(Transform trans:transforms[node])
		{
			if(trans.match(codePoint))
			{
				return epsilons[trans.transform(node)];
			}
		}
		//按照构造规则，每个结点至多有一个非epsilon谓词，此处直接返回空集即可。
		return Collections.emptySet();
	}
	
	/**
	 * 初始化一个状态用于匹配。
	 * 
	 * @return 状态对象。
	 */
	State init()
	{
		return new State();
	}
	
	@Override
	public String toString()
	{
		StringBuilder result=new StringBuilder();
		result.append("Node:").append(transforms.length).append('\n');
		for(int i=0;i<transforms.length;i++)
		{
			result.append(i).append(":({");
			Iterator<Transform> iter=transforms[i].iterator();
			while(iter.hasNext())
			{
				result.append(iter.next().toString(i));
				if(iter.hasNext())
				{
					result.append(',');
				}
			}
			result.append("},[");
			Iterator<Integer> itr=epsilons[i].stream().sorted().iterator();
			while(itr.hasNext())
			{
				result.append(itr.next());
				if(itr.hasNext())
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
	 * NFA有向图。用于NFA的构造，其构造出NFA后不可逆。
	 * <p>
	 * 在2024-07-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	final static class Digraph
	{	
		/**
		 * 变换数组。其长度为结点数目-1。
		 */
		private final List<Transform>[] transforms;
		
		/**
		 * 构造对应结点数量的有向图。
		 * 
		 * @param count 结点数。应大于1。
		 */
		@SuppressWarnings("unchecked")
		private Digraph(int count)
		{
			transforms=(List<Transform>[])new List<?>[count-1];
			for(int index=0;index<count-1;index++)
			{
				transforms[index]=new LinkedList<>();
			}
		}
		
		/**
		 * 在一个结点添加变换。该结点不能是最后一个结点。
		 * 
		 * @param index 结点索引。
		 * @param transform 变换。
		 */
		private void addTransform(int index,Transform transform)
		{
			transforms[index].add(transform);
		}
		
		/**
		 * 复制有向图的转换到对应结点。
		 * 
		 * @param start 目标起始结点。程序应注意避免越界。
		 * @param digraph 有向图。
		 */
		private void addTransforms(int start,Digraph digraph)
		{
			for(int b=0;b<digraph.transforms.length;b++)
			{
				transforms[b+start].addAll(digraph.transforms[b]);
			}
		}
		
		/**
		 * 返回结点数。
		 * 
		 * @return 结点数。
		 */
		private int count()
		{
			return transforms.length+1;
		}
		
		/**
		 * 构造NFA。
		 * 
		 * @return NFA实例。
		 */
		NFA build()
		{
			return new NFA(transforms);
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append("Node:").append(count()).append('\n');
			for(int i=0;i<transforms.length;i++)
			{
				result.append(i).append(":{");
				Iterator<Transform> iter=transforms[i].iterator();
				while(iter.hasNext())
				{
					result.append(iter.next().toString(i));
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
		 * 构造单个变换图。
		 * 
		 * @param predicate 变换谓词。这里不应为空。
		 * @return 由两个结点连接构成，变换条件为谓词。
		 */
		static Digraph transform(CharPredicate predicate)
		{
			//2结点，s-p->e
			Digraph result=new Digraph(2);
			result.addTransform(0,Transform.create(predicate,1));
			return result;
		}
		
		/**
		 * 将a图与b图连接。
		 * 
		 * @param a a有向图，在结果有向图前面。
		 * @param b b有向图，在结果有向图后面。
		 * @return 组合后的结果有向图。
		 */
		static Digraph concat(Digraph a,Digraph b)
		{
			Digraph result=new Digraph(a.count()-1+b.count());
			result.addTransforms(0,a);
			result.addTransforms(a.transforms.length,b);
			return result;
			
		}
		
		/**
		 * 将a图和b图并联。
		 * 
		 * @param a a有向图。
		 * @param b b有向图。
		 * @return 组合后的结果有向图。
		 */
		static Digraph union(Digraph a,Digraph b)
		{
			List<Transform>[] transA=a.transforms;
			List<Transform>[] transB=b.transforms;
			Digraph result=new Digraph(a.count()+b.count());
			int end,index,offset;
			//先添加a图。
			for(end=transA.length,offset=1,index=0;index<transA.length;index++)
			{
				for(Transform trans:transA[index])
				{
					if(trans.transform(index)==end)
					{
						//到达结尾的变换。
						result.transforms[index+offset].add(Transform.offset(trans,index+offset,result.transforms.length));
					}
					else
					{
						//不改变，偏移属性不随定位变化而变化。
						result.transforms[index+offset].add(trans);
					}
				}
			}
			//添加初始结点变化
			result.transforms[0].add(Transform.create(CharPredicate.EPSILON,1));
			result.transforms[0].add(Transform.create(CharPredicate.EPSILON,1+transA.length));
			for(end=transB.length,offset=1+transA.length,index=0;index<transB.length;index++)
			{
				for(Transform trans:transB[index])
				{
					if(trans.transform(index)==end)
					{
						//到达结尾的变换。
						result.transforms[index+offset].add(Transform.offset(trans,index+offset,result.transforms.length));
					}
					else
					{
						//不改变，偏移属性不随定位变化而变化。
						result.transforms[index+offset].add(trans);
					}
				}
			}
			return result;
		}
		
		/**
		 * a的零边界闭包。
		 * 
		 * @param a a有向图。
		 * @return 具有闭包结构的有向图。
		 */
		static Digraph closure(Digraph a)
		{
			Digraph result=new Digraph(a.count()+2);
			result.addTransforms(1,a);
			result.addTransform(0,Transform.create(CharPredicate.EPSILON,1));
			result.addTransform(0,Transform.create(CharPredicate.EPSILON,result.transforms.length));
			result.addTransform(a.count(),Transform.create(CharPredicate.EPSILON,1-a.count()));
			result.addTransform(a.count(),Transform.create(CharPredicate.EPSILON,1));
			return result;
		}
	}
	
	/**
	 * 变换类。实现将谓词判断转换为结点变换。
	 * <p>
	 * 在2024-07-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	private final static class Transform
	{
		/**
		 * 谓词。
		 */
		private final CharPredicate predicate;
		
		/**
		 * 偏移。
		 */
		private final int offset;
		
		/**
		 * 构造目标索引跳转的变换。
		 * 
		 * @param predicate 谓词。不应为空。
		 * @param offset 偏移。
		 */
		private Transform(CharPredicate predicate,int offset)
		{
			this.predicate=predicate;
			this.offset=offset;
		}
		
		/**
		 * 匹配谓词。
		 * 
		 * @param codePoint 码元。
		 * @return 匹配返回真，其余情况返回假。
		 */
		boolean match(int codePoint)
		{
			return predicate.match(codePoint);
		}
		
		/**
		 * 变换索引。应在匹配谓词成功后调用。
		 * 
		 * @param index 索引。理论上该索引经过该变换后依然合理。
		 * @return 变换后索引。
		 */
		int transform(int index)
		{
			return index+offset;
		}
		
		/**
		 * 是否为空变换。
		 * 
		 * @return 空变换返回真。
		 */
		boolean isEpsilonTransform()
		{
			return predicate==CharPredicate.EPSILON;
		}
		
		/**
		 * 以当前结点索引，返回该变换字符串表示。
		 * 
		 * @param index 当前索引。由于仅表示，不要求有范围限制。
		 * @return 变换字符串表示。
		 */
		public String toString(int index)
		{
			return new StringBuilder().append('[').append(predicate).append("]->").append(index+offset).toString();
		}
		
		@Override
		public String toString()
		{
			return new StringBuilder().append('[').append(predicate).append("]->(index)+(").append(offset).append(')').toString();
		}
		
		/**
		 * 创造一个变换。
		 * 
		 * @param predicate 谓词。
		 * @param offset 偏移。
		 * @return 一个新的变换。
		 */
		private static Transform create(CharPredicate predicate,int offset)
		{
			return new Transform(predicate,offset);
		}
		
		/**
		 * 偏移一个变换。
		 * 
		 * @param transform 原变换。
		 * @param index 索引。这里指新图给该变换的索引。
		 * @param dest 目标。这里指新图给该变换的新目标。
		 * @return 偏移后的新变换。
		 */
		private static Transform offset(Transform transform,int index,int dest)
		{
			return new Transform(transform.predicate,dest-index);
		}
	}
	
	/**
	 * 状态机的状态类。
	 * <p>
	 * 在2024-07-22时生成。<br>
	 * @author Tony Chen Smith
	 */
	final class State
	{	
		/**
		 * 尾结点索引。
		 */
		private final int tail;
		
		/**
		 * 当前状态所在结点。
		 */
		private Set<Integer> current;
		
		/**
		 * 已获得码元。
		 */
		private List<Integer> codePoints;
		
		/**
		 * 可能结尾。
		 */
		private Deque<Integer> ends;
		
		/**
		 * 初始化状态类。对于所有正常构造的NFA，均有结点0和无对象对应的尾结点。
		 */
		private State()
		{
			tail=NFA.this.epsilons.length-1;
			current=NFA.this.get(0);
			codePoints=new LinkedList<>();
			ends=new LinkedList<>();
		}
		
		/**
		 * 扫描码元。若可继续匹配码元即返回真。当前可达集变为空时返回假。
		 * 
		 * @param codePoint 码元。
		 * @return 可继续扫描返回真。
		 */
		boolean scan(int codePoint)
		{
			Set<Integer> reachables=new HashSet<>();
			for(int node:current)
			{
				reachables.addAll(NFA.this.transform(node,codePoint));
			}
			if(reachables.isEmpty())
			{
				//还是正常更新当前结点状态。
				current=Collections.emptySet();
				return false;
			}
			else
			{
				current=Set.copyOf(reachables);
				codePoints.add(codePoint);
				if(current.contains(tail))
				{
					//包含尾结点，为一可能结点。该列表自然有序。
					ends.add(codePoints.size());
				}
				return true;
			}
		}
		
		/**
		 * 获得当前所有匹配结果。
		 * 
		 * @return 已匹配成功的字符串数组。
		 */
		String[] get()
		{
			List<String> result=new LinkedList<>();
			int[] text=codePoints.stream().mapToInt(Integer::intValue).toArray();
			for(int end:ends)
			{
				result.add(new String(text,0,end));
			}
			return result.toArray(String[]::new);
		}
		
		/**
		 * 获取最后的有效结果。即最大匹配。
		 * 
		 * @return 最大匹配结果。
		 */
		String getLast()
		{
			Integer end=ends.peekLast();
			if(end==null)
			{
				return null;
			}
			else
			{
				int[] text=codePoints.stream().mapToInt(Integer::intValue).toArray();
				return new String(text,0,end);
			}
		}
		
		/**
		 * 获取最先的有效结果。即最小匹配。
		 * 
		 * @return 最小匹配结果。
		 */
		String getFirst()
		{
			Integer end=ends.peekFirst();
			if(end==null)
			{
				return null;
			}
			else
			{
				int[] text=codePoints.stream().mapToInt(Integer::intValue).toArray();
				return new String(text,0,end);
			}
		}
		
		/**
		 * 重置状态。
		 */
		void reset()
		{
			current=NFA.this.get(0);
			codePoints.clear();
			ends.clear();
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder("currnt:[");
			Iterator<Integer> itr=current.stream().sorted().iterator();
			while(itr.hasNext())
			{
				result.append(itr.next());
				if(itr.hasNext())
				{
					result.append(',');
				}
			}
			result.append("]\ntext:{");
			itr=codePoints.iterator();
			while(itr.hasNext())
			{
				int codePoint=itr.next();
				if(Character.isAlphabetic(codePoint))
				{
					result.appendCodePoint(codePoint);
				}
				else
				{
					result.append("[U+%04X]".formatted(codePoint));
				}
			}
			result.append("}\nend:[");
			itr=ends.iterator();
			while(itr.hasNext())
			{
				result.append(itr.next());
				if(itr.hasNext())
				{
					result.append(',');
				}
			}
			return result.append(']').toString();
		}
	}
}
