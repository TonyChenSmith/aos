package aos.library.regex;

import java.util.Scanner;

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
	 * 测试主方法。
	 * 
	 * @param args 运行参数。
	 */
	public static void main(String[] args)
	{
		System.out.println(Pattern.compile("~.\\U{Lower}\\p{Lower}\\w"));
		System.out.println(Pattern.compile(".[a-bc[^[^def:;-]]]+(0)"));
		System.exit(0);
		@SuppressWarnings("resource") final Scanner scan=new Scanner(System.in);
		while(true)
		{
			System.out.print("test:");
			final String input=scan.nextLine().strip();
			if(input.isEmpty())
			{
				System.out.println("scan reset.");
				continue;
			}
			System.out.println("scan result:");
			for(final int code:input.codePoints().toArray())
			{
				System.out.printf("%c-",(char)code);
			}
		}
	}
	
}
