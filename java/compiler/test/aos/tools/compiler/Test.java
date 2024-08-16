package aos.tools.compiler;

import aos.tools.compiler.Production.Builder;

/**
 * 测试主程序。
 * 
 * <p>在2024-08-11时生成。
 *
 * @author Tony Chen Smith
 */
public class Test
{
	
	/**
	 * @param args
	 */
	public static void main(String[] args)
	{
		// a b c d* | e
		final Builder a=Builder.create("a").concat(Builder.create("b")).concat(Builder.create("c")).concat(Builder.create("d").closure());
		final Builder b=Builder.create("e");
		final Builder r=b.union(a);
		System.out.println(r);
		System.out.println(r.build("T"));
	}
}
