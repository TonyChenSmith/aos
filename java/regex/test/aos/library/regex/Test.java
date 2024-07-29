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
	 * @param args
	 */
	public static void main(String[] args)
	{
		System.out.println(Pattern.compile("~.\\U{Lower}\\p{Lower}\\w"));
		System.out.println(Pattern.compile(".[a-bc[^[^def:;-]]]+(0)"));
		System.exit(0);
		@SuppressWarnings("resource")
		Scanner scan=new Scanner(System.in);
		while(true)
		{
			System.out.print("test:");
			String input=scan.nextLine().strip();
			if(input.isEmpty())
			{
				System.out.println("scan reset.");
				continue;
			}
			System.out.println("scan result:");
			for(int code:input.codePoints().toArray())
			{
				System.out.printf("%c-",(char)code);
			}
		}
	}
	
}
