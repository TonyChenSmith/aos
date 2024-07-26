package aos.library.regex;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Arrays;
import java.util.Scanner;

import aos.library.regex.NFA.Digraph;
import aos.library.regex.NFA.State;

/**
 * 测试用。
 * 
 * <p>在2024-07-17时生成。
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
		System.out.println(Pattern.compile("\\w\\S\\d\\h\\V"));
		System.out.println(Pattern.compile("[abc[^def]]+(0)"));
		System.exit(0);
		ByteArrayOutputStream stream=new ByteArrayOutputStream();
		try
		{
			ObjectOutputStream ts=new ObjectOutputStream(stream);
			ts.writeObject(Pattern.compile("[a-c]*acc"));
			ts.writeObject(Pattern.compile("a{4,6}"));
			ByteArrayInputStream content=new ByteArrayInputStream(stream.toByteArray());
			ObjectInputStream rs=new ObjectInputStream(content);
			System.out.println(rs.readObject());
			System.out.println(rs.readObject());
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
		//构造[a-c]*acc
		Digraph a=Digraph.transform(CharPredicate.create('a','c'));
		a=Digraph.closure(a);
		Digraph b=Digraph.transform(CharPredicate.create('a'));
		Digraph c=Digraph.transform(CharPredicate.create('c'));
		c=Digraph.concat(c,c);
		b=Digraph.concat(b,c);
		a=Digraph.concat(a,b);
		NFA test=new Parser("[^a-c&e-f-d]{0,}acc").parse().build();
		System.out.println("NFA");
		System.out.println(test);
		@SuppressWarnings("resource")
		Scanner scan=new Scanner(System.in);
		State state=test.init();
		while(true)
		{
			System.out.print("test:");
			String input=scan.nextLine().strip();
			if(input.isEmpty())
			{
				System.out.println("scan reset.");
				state.reset();
				continue;
			}
			System.out.println("scan result:");
			for(int code:input.codePoints().toArray())
			{
				System.out.printf("%c-",(char)code);
				System.out.println(state.scan(code));
			}
			System.out.println(state);
			System.out.println(Arrays.toString(state.get()));
		}
	}
	
}
