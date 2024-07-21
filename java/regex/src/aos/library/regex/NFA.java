package aos.library.regex;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Deque;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

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
		this.transforms=(List<Transform>[])new List<?>[transforms.length];
		this.epsilons=(Set<Integer>[])new Set<?>[transforms.length];
		List<Transform> nonull=new LinkedList<>();
		Deque<Integer> stack=new LinkedList<>();
		for(int index=0;index<transforms.length;index++)
		{
			this.epsilons[index]=new TreeSet<>();
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
	}
	
	@Override
	public String toString()
	{
		StringBuilder result=new StringBuilder();
		result.append("Node:").append(transforms.length+1).append('\n');
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
}
