package aos.library.regex;

import aos.library.regex.NFA.Digraph;

/**
 * 测试用。
 * 在2024-07-17时生成。<br>
 * @author Tony Chen Smith
 */
public class Test
{
	
	/**
	 * @param args
	 */
	public static void main(String[] args)
	{
		Digraph a=Digraph.transform(CharPredicate.create(1,' '));
		System.out.println(a);
		Digraph b=Digraph.closure(a);
		System.out.println(b);
		Digraph c=Digraph.concat(a,b);
		System.out.println(c);
		Digraph d=Digraph.union(b,c);
		System.out.println(d);
		
		NFA ta=a.build();
		NFA tb=b.build();
		NFA tc=c.build();
		NFA td=d.build();
		System.out.println(ta);
		System.out.println(tb);
		System.out.println(tc);
		System.out.println(td);
	}
	
}
