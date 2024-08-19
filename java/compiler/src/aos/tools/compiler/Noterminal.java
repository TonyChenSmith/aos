package aos.tools.compiler;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 非终结符。
 * 
 * <p>在2024-08-19时生成。
 *
 * @author Tony Chen Smith
 */
final class Noterminal implements SyntaxSymbol
{
	/**
	 * 非终结符属性。
	 */
	private final Map<String,String> attributes;
	
	/**
	 * 非终极符名。
	 */
	private final String name;
	
	/**
	 * 下属语法符号。
	 */
	private final SyntaxSymbol[] symbols;
	
	Noterminal(String name,List<SyntaxSymbol> symbols)
	{
		this.attributes=new HashMap<>();
		this.name=name;
		this.symbols=new SyntaxSymbol[symbols.size()+1];
		this.symbols[0]=this;
		for(int index=0;index<symbols.size();index++)
		{
			this.symbols[index+1]=symbols.get(index);
		}
	}
	
	@Override
	public boolean equals(Object obj)
	{
		if(obj instanceof final Noterminal noterminal)
		{
			return noterminal.name.equals(name);
		}
		return false;
	}
	
	@Override
	public int hashCode()
	{
		return name.hashCode();
	}
	
	@Override
	public String get(String key)
	{
		return attributes.getOrDefault(key,"");
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
	public int count()
	{
		return symbols.length;
	}

	@Override
	public SyntaxSymbol getSymbol(int index)
	{
		return symbols[index];
	}
	
	@Override
	public String toString()
	{
		final StringBuilder result=new StringBuilder(name);
		return result.append(':').append(Arrays.toString(symbols)).append(attributes).toString();
	}
}
