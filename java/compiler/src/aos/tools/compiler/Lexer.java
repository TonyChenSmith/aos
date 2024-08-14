package aos.tools.compiler;

import java.io.Closeable;
import java.io.IOException;
import java.io.PushbackReader;
import java.io.Reader;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import aos.library.regex.Matcher;
import aos.library.regex.Pattern;

/**
 * 词法器。
 * 
 * <p>在2024-08-14时生成。
 *
 * @author Tony Chen Smith
 */
final class Lexer implements Closeable
{
	/**
	 * 行结尾。
	 */
	private static final int EOF=-1;
	
	/**
	 * 字符输入。
	 */
	private final PushbackReader input;
	
	/**
	 * 缓存。
	 */
	private Integer buffer;
	
	/**
	 * 构造词法器。
	 * 
	 * @param input
	 */
	Lexer(Reader input)
	{
		this.input=new PushbackReader(input);
		buffer=null;
	}
	
	/**
	 * 读入一个码元。
	 * 
	 * @return 码元，或行结尾。
	 */
	private int read()
	{
		try
		{
			char result=(char)input.read();
			if(result==-1)
			{
				return EOF;
			}
			if(Character.isSurrogate(result))
			{
				char b=(char)input.read();
				if(Character.isSurrogatePair(result,b))
				{
					return Character.toCodePoint(result,b);
				}
				else if(Character.isSurrogatePair(b,result))
				{
					return Character.toCodePoint(b,result);
				}
				else
				{
					input.unread(b);
					return result;
				}
			}
			return result;
		}
		catch(IOException e)
		{
			throw new RuntimeException(e);
		}
	}
	
	/**
	 * 扫描获得单词。如果同时有多个匹配，按表的前后顺序进行仲裁，小行标的优先。
	 * 
	 * @param context 上下文。
	 * @return 对应单词。
	 */
	Token scan(Context context)
	{
		Token result=rawScan(context);
		while(context.banTokens.contains(result.name()))
		{
			//下一个。
			result=rawScan(context);
		}
		return result;
	}
	
	/**
	 * 扫描获得单词。如果同时有多个匹配，按表的前后顺序进行仲裁，小行标的优先。
	 * 
	 * @param context 上下文。
	 * @return 对应单词。
	 */
	private Token rawScan(Context context)
	{
		Set<Matcher> matchers=new HashSet<>();
		context.patterns.getColumnValues("pattern").forEach(p->matchers.add(((Pattern)p).matcher()));
		//最长匹配。
		Set<Matcher> uncalc=new HashSet<>();
		List<Integer> chars=new LinkedList<>();
		while(true)
		{
			//清空结算集。
			uncalc.clear();
			int ch;
			if(buffer!=null)
			{
				ch=buffer;
				buffer=null;
			}
			else
			{
				ch=read();
			}
			chars.add(ch);
			Iterator<Matcher> itr=matchers.iterator();
			while(itr.hasNext())
			{
				Matcher m=itr.next();
				if(!m.scan(ch))
				{
					if(m.match())
					{
						uncalc.add(m);
					}
					itr.remove();
				}
			}
			if(matchers.isEmpty())
			{
				buffer=ch;
				break;
			}
		}
		if(uncalc.isEmpty())
		{
			//毫无结果。
			if(chars.size()==1&&buffer==EOF)
			{
				//内置文件结束词。
				return new Token("$eof","$");
			}
			else
			{
				throw new RuntimeException("有不识别的字符，无词法单元与之匹配。%s".formatted(chars.toString()));
			}
		}
		else
		{
			Iterator<Matcher> itr=uncalc.iterator();
			Matcher cm=itr.next();
			int clevel=Integer.valueOf(context.patterns.getRow(cm.pattern()));
			while(itr.hasNext())
			{
				Matcher m=itr.next();
				int level=Integer.valueOf(context.patterns.getRow(m.pattern()));
				if(level<clevel)
				{
					cm=m;
					clevel=level;
				}
			}
			return new Token((String)context.patterns.get(clevel,"name"),cm.get());
		}
	}
	
	@Override
	public void close() throws IOException
	{
		input.close();
	}
}
