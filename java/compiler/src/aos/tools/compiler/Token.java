package aos.tools.compiler;

import java.util.HashMap;
import java.util.Map;

/**
 * 单词（词法单元）类。
 * 
 * <p>定义了词法器与语法器之间对接的单词应有的属性功能。
 * 
 * <p>在2024-08-08时生成。
 *
 * @author Tony Chen Smith
 */
final class Token implements SyntaxSymbol
{
	/**
	 * 单词属性。
	 */
	private final Map<String,String> attributes;
	
	/**
	 * 单词名。
	 */
	private final String name;
	
	/**
	 * 构造单词。
	 * 
	 * @param name  单词名。
	 * @param lexem 单词词素。
	 */
	protected Token(String name,String lexem)
	{
		this.name=name;
		attributes=new HashMap<>();
		attributes.put("lexem",lexem);
	}
	
	/**
	 * 一般比较单词名。
	 */
	@Override
	public boolean equals(Object obj)
	{
		if(obj instanceof final Token token)
		{
			return token.name.equals(name);
		}
		return false;
	}
	
	@Override
	public String get(String key)
	{
		return attributes.getOrDefault(key,"");
	}
	
	/**
	 * 一般比较单词名。
	 */
	@Override
	public int hashCode()
	{
		return name.hashCode();
	}
	
	@Override
	public String name()
	{
		return name;
	}
	
	@Override
	public void set(String key,String value)
	{
		attributes.put(key,value);
	}
	
	@Override
	public String toString()
	{
		final StringBuilder result=new StringBuilder(name);
		return result.append(':').append(get("lexem")).append(attributes).toString();
	}
}
