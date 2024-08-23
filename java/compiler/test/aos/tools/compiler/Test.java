package aos.tools.compiler;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

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
		Map<String,Production> p=new HashMap<>();
		p.put("%start",Builder.create("S").build("%start"));
		p.put("S",Builder.create("C").concat(Builder.create("C")).build("S"));
		p.put("C",Builder.union(Builder.create("c").concat(Builder.create("C")),Builder.create("d")).build("C"));
		Context context=new Context(Map.of(),Map.of(),Map.of(),Map.of(),Set.of(),p);
		LRTable t1=LRTable.create(context);
		
		Map<String,Production> p1=new HashMap<>();
		p1.put("%start",Builder.create("S").build("%start"));
		p1.put("S",Builder.union(Builder.create("L").concat(Builder.create("=")).concat(Builder.create("R")),Builder.create("R")).build("S"));
		p1.put("L",Builder.union(Builder.create("*").concat(Builder.create("R")),Builder.create("id")).build("L"));
		p1.put("R",Builder.create("L").build("R"));
		Context context1=new Context(Map.of(),Map.of(),Map.of(),Map.of(),Set.of(),p1);
		LRTable t2=LRTable.create(context1);
		System.out.println(t1);
		System.out.println(t2);
		
		context1.onError("a","test.clay");
	}
}
