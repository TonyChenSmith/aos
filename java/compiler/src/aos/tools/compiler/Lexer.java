package aos.tools.compiler;

import java.io.Closeable;
import java.io.IOException;
import java.io.PushbackReader;
import java.io.Reader;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import aos.library.regex.Matcher;

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
	 * 缓存。
	 */
	private Integer buffer;
	
	/**
	 * 字符输入。
	 */
	private final PushbackReader input;
	
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
	
	@Override
	public void close() throws IOException
	{
		input.close();
	}
	
	/**
	 * 扫描获得单词。如果同时有多个匹配，按优先级的前后顺序进行仲裁，小数值的优先。
	 * 
	 * @param context 上下文。
	 * 
	 * @return 对应单词。返回空值时语法器应准备报错。
	 */
	Token scan(Context context)
	{
		Token result=rawScan(context);
		if(result==null)
		{
			return null;
		}
		context.onScan(result);
		while(context.banTokens.contains(result.name()))
		{
			// 下一个。
			result=rawScan(context);
			context.onScan(result);
		}
		return result;
	}
	
	/**
	 * 扫描获得单词。如果同时有多个匹配，按优先级的前后顺序进行仲裁，小数值的优先。
	 * 
	 * @param context 上下文。
	 * 
	 * @return 对应单词。返回空值时语法器应准备报错。
	 */
	private Token rawScan(Context context)
	{
		final Set<Matcher> matchers=new HashSet<>();
		context.patterns.forEach((s,p)->matchers.add(p.matcher()));
		// 最长匹配。
		final Set<Matcher> uncalc=new HashSet<>();
		// 当前已接受行号。
		final int mark=context.currentColumn();
		while(true)
		{
			// 清空结算集。
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
			final Iterator<Matcher> itr=matchers.iterator();
			while(itr.hasNext())
			{
				final Matcher m=itr.next();
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
			context.acceptInput(ch);
		}
		if(uncalc.isEmpty())
		{
			// 毫无结果。
			if(mark==context.currentColumn()&&buffer==EOF)
			{
				// 内置文件结束词。该类型为最低级。
				return new Token("$eof","");
			}
			else
			{
				// 交由语法器报错。
				return null;
			}
		}
		final Iterator<Matcher> itr=uncalc.iterator();
		Matcher cm=itr.next();
		int cpriority=context.getTokenPriority(cm.pattern());
		while(itr.hasNext())
		{
			final Matcher m=itr.next();
			final int priority=context.getTokenPriority(m.pattern());
			if(priority<cpriority)
			{
				cm=m;
				cpriority=priority;
			}
		}
		return new Token(context.tokenMap.get(cm.pattern()),cm.get());
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
			final char result=(char)input.read();
			if(result==-1)
			{
				return EOF;
			}
			if(Character.isSurrogate(result))
			{
				final char b=(char)input.read();
				if(Character.isSurrogatePair(result,b))
				{
					return Character.toCodePoint(result,b);
				}
				if(Character.isSurrogatePair(b,result))
				{
					return Character.toCodePoint(b,result);
				}
				else
				{
					input.unread(b);
				}
			}
			return result;
		}
		catch(final IOException e)
		{
			throw new RuntimeException(e);
		}
	}
}
