package aos.library.regex;

import static aos.library.regex.CharPredicate.ALL;
import static aos.library.regex.CharPredicate.DOT;
import static aos.library.regex.CharPredicate.EPSILON;
import static aos.library.regex.CharPredicate.HEX;
import static aos.library.regex.CharPredicate.HSPACE;
import static aos.library.regex.CharPredicate.NUMBER;
import static aos.library.regex.CharPredicate.SPACE;
import static aos.library.regex.CharPredicate.VSPACE;
import static aos.library.regex.CharPredicate.*;

import java.util.LinkedList;

import aos.library.regex.NFA.Digraph;

/**
 * 正则表达式的语法分析器类。
 * 
 * <p>用于编译正则表达式到完整有向图。
 * 
 * <p>在2024-07-23时生成。
 * 
 * @author Tony Chen Smith
 */
final class Parser
{
	/**
	 * 字符串结尾。
	 */
	private static final int EOS=-1;
	
	/**
	 * 字符串码元。
	 */
	private final int[] codePoints;
	
	/**
	 * 光标。
	 */
	private int cursor;
	
	/**
	 * 构造语法分析器。
	 * 
	 * @param regex 正则表达式。
	 */
	Parser(String regex)
	{
		codePoints=regex.codePoints().toArray();
		cursor=0;
	}
	
	/**
	 * 向前看。
	 * 
	 * @return 当前光标所指码元，或字符串结尾。
	 */
	private int lookahead()
	{
		return cursor>=codePoints.length?EOS:codePoints[cursor];
	}
	
	/**
	 * 下一个。
	 */
	private void next()
	{
		cursor++;
	}
	
	/**
	 * 产生式：start。开始符号。
	 * 
	 * @return 对应有向图。
	 */
	private Digraph start()
	{
		return group(true);
	}
	
	/**
	 * 产生式：group。捕获组内部表达式分析处理。
	 * 
	 * @param isStart 是否是开始符号。start与group中对右括号处理方式不同。
	 * @return 对应有向图。
	 */
	private Digraph group(boolean isStart)
	{
		LinkedList<Digraph> unions=new LinkedList<>();
		LinkedList<Digraph> concats=new LinkedList<>();
		Digraph d;
		while(true)
		{
			switch(lookahead()){
				case '.':
					//通配符。
					next();
					concats.add(closure(Digraph.transform(DOT)));
					break;
				case '(':
					//分组。
					next();
					concats.add(closure(group(false)));
					break;
				case '\\':
					//转义序列。
					next();
					concats.add(closure(Digraph.transform(escape())));
					break;
				case '[':
					//字符类。
					next();
					concats.add(closure(Digraph.transform(clazz())));
					break;
				case '|':
					next();
					if(concats.isEmpty())
					{
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现分支缺乏左值。".formatted(cursor));
					}
					else
					{
						d=concats.removeFirst();
						for(Digraph digraph:concats)
						{
							d=d.concat(digraph);
						}
						concats.clear();
						unions.add(d);
					}
					break;
				case ')':
					next();
					if(isStart)
					{
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现多余括号。".formatted(cursor));
					}
					else if(concats.isEmpty())
					{
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现缺乏字符或分支缺乏右值。".formatted(cursor));
					}
					else
					{
						d=concats.removeFirst();
						for(Digraph digraph:concats)
						{
							d=d.concat(digraph);
						}
						unions.add(d);
						d=unions.removeFirst();
						for(Digraph digraph:concats)
						{
							d=d.union(digraph);
						}
						return d;
					}
				case EOS:
					if(isStart)
					{
						if(concats.isEmpty())
						{
							throw new IllegalArgumentException("分析表达式时，在下标%d处发现缺乏字符或分支缺乏右值。".formatted(cursor));
						}
						d=concats.removeFirst();
						for(Digraph digraph:concats)
						{
							d=d.concat(digraph);
						}
						unions.add(d);
						d=unions.removeFirst();
						for(Digraph digraph:unions)
						{
							d=d.union(digraph);
						}
						return d;
					}
					else
					{
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现不期待的结尾。".formatted(cursor));
					}
				default:
					//一般字符。
					d=Digraph.transform(CharPredicate.create(lookahead()));
					next();
					concats.add(closure(d));
					break;
			}
		}
	}
	
	/**
	 * 产生式：closure。检查闭包。
	 * 
	 * @param digraph 需要检查闭包的有向图。
	 * @return 对应有向图。
	 */
	private Digraph closure(Digraph digraph)
	{
		switch(lookahead())
		{
			case '*':
				//0到无穷。
				next();
				return digraph.closure();
			case '+':
				//1到无穷。
				next();
				return digraph.concat(digraph.closure());
			case '?':
				//0到1。
				next();
				return digraph.union(Digraph.transform(EPSILON));
			case '{':
				next();
				return closureRange(digraph);
			default:
				return digraph;
		}
	}
	
	
	/**
	 * 产生式：closureRange。闭包中范围结构准备。
	 * 
	 * @param digraph 需要检查闭包的有向图。
	 * @return 对应有向图。
	 */
	private Digraph closureRange(Digraph digraph)
	{
		int l,r=-1;
		if(NUMBER.match(lookahead()))
		{
			l=toNumber(lookahead());
			next();
			while(true)
			{
				if(NUMBER.match(lookahead()))
				{
					l=l*10+toNumber(lookahead());
					next();
				}
				else if(lookahead()=='}')
				{
					r=l;
					next();
					break;
				}
				else if(lookahead()==',')
				{
					next();
					break;
				}
				else
				{
					throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(cursor,toNumber(lookahead())));
				}
			}
		}
		else
		{
			throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(cursor,toNumber(lookahead())));
		}
		if(r==-1)
		{
			if(NUMBER.match(lookahead()))
			{
				r=toNumber(lookahead());
				next();
				while(true)
				{
					if(NUMBER.match(lookahead()))
					{
						r=r*10+toNumber(lookahead());
						next();
					}
					else if(lookahead()=='}')
					{
						next();
						break;
					}
					else
					{
						throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(cursor,toNumber(lookahead())));
					}
				}
			}
			else if(lookahead()=='}')
			{
				r=-2;
				next();
			}
			else
			{
				throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(cursor,toNumber(lookahead())));
			}
		}
		if(l==r)
		{
			//重复n次。{n}。
			int a=l;
			if(a==0)
			{
				//重复0次，即该图不存在。
				return Digraph.transform(EPSILON);
			}
			else
			{
				Digraph result=digraph;
				while(a>1)
				{
					result=result.concat(digraph);
					a--;
				}
				return result;
			}
		}
		else if(r==-2)
		{
			//n到无穷。{n,}。
			int a=l;
			if(a==0)
			{
				//0次，即0边界闭包。
				return digraph.closure();
			}
			else
			{
				//连接多次。
				Digraph result=digraph;
				while(a>1)
				{
					result=result.concat(digraph);
					a--;
				}
				return result.concat(digraph.closure());
			}
		}
		else
		{
			int a=l;
			int b=r;
			if(a>b)
			{
				throw new IllegalArgumentException("分析量词范围时，到下标%d为止，发现量词范围的上限%d小于下限%d。".formatted(cursor,b,a));
			}
			else
			{
				Digraph prefix=null;
				int c=a;
				if(a!=0)
				{
					prefix=digraph;
					while(c>1)
					{
						prefix=prefix.concat(digraph);
						c--;
					}
				}
				Digraph branch=digraph;
				Digraph suffix=Digraph.transform(EPSILON).union(branch);
				c=b-a;
				while(c>1)
				{
					branch=branch.concat(digraph);
					suffix=suffix.union(branch);
					c--;
				}
				if(prefix==null)
				{
					return suffix;
				}
				else
				{
					return prefix.concat(suffix);
				}
			}
		}
	}
	
	/**
	 * 产生式：escape。转义序列。正则库默认Unicode范围。
	 * 
	 * @return 对应谓词。用于后续处理。
	 */
	private CharPredicate escape()
	{
		int c;
		switch(lookahead())
		{
			case 'u':
				//uXXXX。四位Unicode。
				next();
				return uxxxx();
			case 'n':
				//换行。
				next();
				return CharPredicate.create('\n');
			case 't':
				//制表。
				next();
				return CharPredicate.create('\t');
			case 'r':
				//回车。
				next();
				return CharPredicate.create('\r');
			case 'a':
				//警铃。
				next();
				return CharPredicate.create('\u0007');
			case 'f':
				//竖直制表。
				next();
				return CharPredicate.create('\u000C');
			case 'e':
				//退出。
				next();
				return CharPredicate.create('\u001B');
			case EOS:
				//错误结尾。
				throw new IllegalArgumentException("分析转义序列时，在下标%d发现不期望的结束。".formatted(cursor));
			case 'N':
				//字符名。
				next();
				return name();
			case 'x':
				//变长码元。
				next();
				return xnumber();
			case 'c':
				next();
				return control();
			case 'd':
				//十进制数字。
				next();
				return NUMBER;
			case 'D':
				//非十进制数字。
				next();
				return CharPredicate.not(NUMBER);
			case 'h':
				//水平空白。
				next();
				return HSPACE;
			case 'H':
				//非水平空白。
				next();
				return CharPredicate.not(HSPACE);
			case 'v':
				//竖直空白。
				next();
				return VSPACE;
			case 'V':
				//非竖直空白。
				next();
				return CharPredicate.not(VSPACE);
			case 's':
				//一般空白。
				next();
				return SPACE;
			case 'S':
				//非一般空白。
				next();
				return CharPredicate.not(SPACE);
			case 'w':
				//单词。
				next();
				return WORD;
			case 'W':
				//非单词。
				next();
				return CharPredicate.not(WORD);
			case 'p':
				//POSIX字符类。
				next();
				return posix();
			case 'U':
				//Unicode字符类。
				next();
				return unicode();
			default:
				//默认转义一切字符。
				c=lookahead();
				next();
				return CharPredicate.create(c);
		}
	}
	
	/**
	 * 产生式：uxxxx。四位Unicode转义序列。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate uxxxx()
	{
		int codePoint;
		if(HEX.match(lookahead()))
		{
			//第一字符通过。
			codePoint=toNumber(lookahead());
			next();
			if(HEX.match(lookahead()))
			{
				//第二字符通过。
				codePoint=codePoint<<4+toNumber(lookahead());
				next();
				if(HEX.match(lookahead()))
				{
					//第三字符通过。
					codePoint=codePoint<<4+toNumber(lookahead());
					next();
					if(HEX.match(lookahead()))
					{
						//第四字符通过。
						codePoint=codePoint<<4+toNumber(lookahead());
						next();
						return CharPredicate.create(codePoint);
					}
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\uXXXX]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
	}
	
	/**
	 * 产生式：name。字符名转义。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate name()
	{
		if(lookahead()=='{')
		{
			StringBuilder n=new StringBuilder();
			int codePoint;
			next();
			while(true)
			{
				switch(lookahead())
				{
					case EOS:
						throw new IllegalArgumentException("分析转义序列[\\N{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
					case '}':
						next();
						try
						{
							codePoint=Character.codePointOf(n.toString());
							return CharPredicate.create(codePoint);
						}
						catch(IllegalArgumentException e)
						{
							throw new IllegalArgumentException("分析转义序列[\\N{name}]，在下标%d时发现不存在的字符名[%s]。".formatted(cursor,n.toString()));
						}
					default:
						n.appendCodePoint(lookahead());
						next();
						break;
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\N{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
	}
	
	/**
	 * 产生式：posix。POSIX定义字符类，仅限于US-ASCII。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate posix()
	{
		if(lookahead()=='{')
		{
			StringBuilder n=new StringBuilder();
			next();
			while(true)
			{
				switch(lookahead())
				{
					case EOS:
						throw new IllegalArgumentException("分析转义序列[\\p{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
					case '}':
						next();
						switch(n.toString())
						{
							case "Lower":
								return LOWER;
							case "Upper":
								return UPPER;
							case "ASCII":
								return ASCII;
							case "Alpha":
								return ALPHA;
							case "Digit":
								return NUMBER;
							case "Alnum":
								return ALNUM;
							case "Punct":
								return PUNCT;
							case "Graph":
								return GRAPH;
							case "Print":
								return PRINT;
							case "Blank":
								return BLANK;
							case "Cntrl":
								return CNTRL;
							case "XDigit":
								return HEX;
							case "Space":
								return SPACE;
							default:
								throw new IllegalArgumentException("分析转义序列[\\p{name}]，在下标%d时发现不存在的字符类名[%s]。".formatted(cursor,n.toString()));
						}
					default:
						n.appendCodePoint(lookahead());
						next();
						break;
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\p{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
	}
	
	/**
	 * 产生式：unicode。Unicode定义字符类。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate unicode()
	{
		if(lookahead()=='{')
		{
			StringBuilder n=new StringBuilder();
			next();
			while(true)
			{
				switch(lookahead())
				{
					case EOS:
						throw new IllegalArgumentException("分析转义序列[\\U{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
					case '}':
						next();
						switch(n.toString())
						{
							case "Lower":
								return LOWERCASE;
							case "Upper":
								return UPPERCASE;
							case "ASCII":
								return ASCII;
							case "Alpha":
								return ALPHABETIC;
							case "Digit":
								return DIGIT;
							case "Alnum":
								return UALNUM;
							case "Punct":
								return PUNCTUATION;
							case "Graph":
								return UGRAPH;
							case "Print":
								return null;
							case "Blank":
								return null;
							case "Cntrl":
								return null;
							case "XDigit":
								return null;
							case "Space":
								return null;
							default:
								throw new IllegalArgumentException("分析转义序列[\\U{name}]，在下标%d时发现不存在的字符类名[%s]。".formatted(cursor,n.toString()));
						}
					default:
						n.appendCodePoint(lookahead());
						next();
						break;
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\U{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
	}
	
	/**
	 * 产生式：xnumber。变长码元。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate xnumber()
	{
		if(lookahead()=='{')
		{
			next();
			if(HEX.match(lookahead()))
			{
				int codePoint=toNumber(lookahead());
				next();
				while(true)
				{
					if(HEX.match(lookahead()))
					{
						codePoint=codePoint<<4+toNumber(lookahead());
					}
					else if(lookahead()=='}')
					{
						if(ALL.match(codePoint))
						{
							return CharPredicate.create(codePoint);
						}
						else
						{
							throw new IllegalArgumentException("分析转义序列[\\x{X}]时，在下标%d发现Unicode码元不存在[%04X]。".formatted(cursor,codePoint));
						}
					}
					else
					{
						break;
					}
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\x{X}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
	}
	
	/**
	 * 产生式：control。将可见字符转义为前32个控制字符。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate control()
	{
		if(lookahead()!=-1)
		{
			int a=lookahead();
			next();
			return CharPredicate.create(a&0x1F);
		}
		throw new IllegalArgumentException("分析转义序列[\\\\cX]时，在下标%d发现不期望的结尾。");
	}
	
	/**
	 * 产生式：clazz。字符类，在其中部分符号的含义将被略去。
	 * 
	 * @return 对应谓词。
	 */
	private CharPredicate clazz()
	{
		boolean isNot=false;
		LinkedList<CharPredicate> and=new LinkedList<>();
		LinkedList<CharPredicate> or=new LinkedList<>();
		if(lookahead()=='^')
		{
			next();
			isNot=true;
		}
		CharPredicate p;
		while(true)
		{
			switch(lookahead())
			{
				case EOS:
					throw new IllegalArgumentException("分析字符类时，在下标%d发现不期望的情况[%s]。".formatted(cursor,toString(lookahead())));
				case '\\':
					next();
					or.add(escape());
					break;
				case '&':
					//逻辑与运算。
					next();
					if(or.isEmpty())
					{
						or.add(CharPredicate.create('&'));
					}
					else
					{
						p=or.removeFirst();
						for(CharPredicate pred:or)
						{
							p=CharPredicate.or(p,pred);
						}
						or.clear();
						and.add(p);
					}
					break;
				case '[':
					//嵌套字符类。
					next();
					or.add(clazz());
					break;
				case '-':
					//范围符。
					next();
					if(or.isEmpty()||or.getLast().isRange())
					{
						or.add(CharPredicate.create('-'));
					}
					else
					{
						switch(lookahead())
						{
							case '[':
							case '&':
							case ']':
								or.add(CharPredicate.create('-'));
								break;
							case '\\':
								p=escape();
								if(p.isRange())
								{
									or.add(CharPredicate.create('-'));
									or.add(p);
								}
								else
								{
									or.add(CharPredicate.create(or.removeLast(),p));
								}
								break;
							default:
								or.add(CharPredicate.create(or.removeLast(),CharPredicate.create(lookahead())));
								next();
								break;
						}
					}
					break;
				case ']':
					next();
					if(or.isEmpty())
					{
						throw new IllegalArgumentException("分析字符类时，在下标%d发现没有指定范围。".formatted(cursor,toString(lookahead())));
					}
					else
					{
						p=or.removeFirst();
						for(CharPredicate pred:or)
						{
							p=CharPredicate.or(p,pred);
						}
						and.add(p);
						p=and.removeFirst();
						for(CharPredicate pred:and)
						{
							p=CharPredicate.and(p,pred);
						}
						if(isNot)
						{
							p=CharPredicate.not(p);
						}
						return p;
					}
				default:
					or.add(CharPredicate.create(lookahead()));
					next();
					break;
			}
		}
	}
	
	/**
	 * 对正则表达式分析，获得有向图。
	 * 
	 * @return 正则表达式对应的有向图。
	 */
	public Digraph parse()
	{
		return start();
	}
	
	/**
	 * 将字符转换为数字。
	 * 
	 * @param value 字符。
	 * @return 对应的数字。
	 */
	static int toNumber(int value)
	{
		if(NUMBER.match(value))
		{
			return value-'0';
		}
		else if(value>='a'&&value<='f')
		{
			return value-'a'+10;
		}
		else
		{
			return value-'A'+10;
		}
	}
	
	/**
	 * 将字符变成可显示的字符串。
	 * 
	 * @param value 字符。
	 * @return 字符串。
	 */
	static String toString(int value)
	{
		if(value==EOS)
		{
			return "EOS";
		}
		else if(UGRAPH.match(value))
		{
			return Character.toString(value);
		}
		else
		{
			return "\\x{%04X}".formatted(value);
		}
	}
}
