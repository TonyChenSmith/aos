package aos.library.regex;

import static aos.library.regex.CharPredicate.ALL;
import static aos.library.regex.CharPredicate.ALNUM;
import static aos.library.regex.CharPredicate.ALPHA;
import static aos.library.regex.CharPredicate.ALPHABETIC;
import static aos.library.regex.CharPredicate.ASCII;
import static aos.library.regex.CharPredicate.ASSIGNED;
import static aos.library.regex.CharPredicate.BLANK;
import static aos.library.regex.CharPredicate.CC;
import static aos.library.regex.CharPredicate.CNTRL;
import static aos.library.regex.CharPredicate.DIGIT;
import static aos.library.regex.CharPredicate.DOT;
import static aos.library.regex.CharPredicate.EPSILON;
import static aos.library.regex.CharPredicate.GRAPH;
import static aos.library.regex.CharPredicate.HEX;
import static aos.library.regex.CharPredicate.HEX_DIGIT;
import static aos.library.regex.CharPredicate.HSPACE;
import static aos.library.regex.CharPredicate.IDEOGRAPHIC;
import static aos.library.regex.CharPredicate.JOIN_CONTROL;
import static aos.library.regex.CharPredicate.LETTER;
import static aos.library.regex.CharPredicate.LOWER;
import static aos.library.regex.CharPredicate.LOWERCASE;
import static aos.library.regex.CharPredicate.NONCHARACTER_CODE_POINT;
import static aos.library.regex.CharPredicate.NUMBER;
import static aos.library.regex.CharPredicate.PRINT;
import static aos.library.regex.CharPredicate.PUNCT;
import static aos.library.regex.CharPredicate.PUNCTUATION;
import static aos.library.regex.CharPredicate.SPACE;
import static aos.library.regex.CharPredicate.TITLECASE;
import static aos.library.regex.CharPredicate.UALNUM;
import static aos.library.regex.CharPredicate.UBLANK;
import static aos.library.regex.CharPredicate.UGRAPH;
import static aos.library.regex.CharPredicate.UPPER;
import static aos.library.regex.CharPredicate.UPPERCASE;
import static aos.library.regex.CharPredicate.UPRINT;
import static aos.library.regex.CharPredicate.UWORD;
import static aos.library.regex.CharPredicate.UXDIGIT;
import static aos.library.regex.CharPredicate.VSPACE;
import static aos.library.regex.CharPredicate.WHITE_SPACE;
import static aos.library.regex.CharPredicate.WORD;

import java.util.LinkedList;

import aos.library.regex.NFA.Digraph;

/**
 * 正则表达式的语法分析器类。
 * 
 * <p>
 * 用于编译正则表达式到完整有向图。
 * 
 * <p>
 * 在2024-07-23时生成。
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
		CharPredicate p;
		while(true)
		{
			switch(lookahead()){
				case '.':
					// 通配符。
					next();
					concats.add(closure(Digraph.transform(DOT)));
					break;
				case '(':
					// 分组。
					next();
					concats.add(closure(group(false)));
					break;
				case '\\':
					// 转义序列。
					next();
					concats.add(closure(Digraph.transform(escape())));
					break;
				case '[':
					// 字符类。
					next();
					concats.add(closure(Digraph.transform(clazz())));
					break;
				case '|':
					next();
					if(concats.isEmpty())
					{
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现分支缺乏左值。".formatted(
							cursor));
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
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现多余括号。".formatted(
							cursor));
					}
					else if(concats.isEmpty())
					{
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现缺乏字符或分支缺乏右值。".formatted(
							cursor));
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
				case '~':
					// 直到下一字符或字符类。
					next();
					switch(lookahead()){
						case '(':
						case ')':
						case '|':
						case EOS:
							// 无事发生，作为正常字符加入。
							d=Digraph.transform(CharPredicate.create('~'));
							concats.add(d);
							break;
						case '[':
							// 字符类。
							next();
							p=clazz();
							d=Digraph.transform(CharPredicate.not(p)).closure();
							d=d.concat(closure(Digraph.transform(p)));
							concats.add(d);
							break;
						case '.':
							// 通配符。
							next();
							p=DOT;
							d=Digraph.transform(CharPredicate.not(p)).closure();
							d=d.concat(closure(Digraph.transform(p)));
							concats.add(d);
							break;
						case '\\':
							// 转义序列。
							next();
							p=escape();
							d=Digraph.transform(CharPredicate.not(p)).closure();
							d=d.concat(closure(Digraph.transform(p)));
							concats.add(d);
							break;
						default:
							// 一般字符。
							p=CharPredicate.create(lookahead());
							next();
							d=Digraph.transform(CharPredicate.not(p)).closure();
							d=d.concat(closure(Digraph.transform(p)));
							concats.add(d);
							break;
					}
					break;
				case EOS:
					if(isStart)
					{
						if(concats.isEmpty())
						{
							throw new IllegalArgumentException("分析表达式时，在下标%d处发现缺乏字符或分支缺乏右值。"
								.formatted(cursor));
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
						throw new IllegalArgumentException("分析表达式时，在下标%d处发现不期待的结尾。".formatted(
							cursor));
					}
				default:
					// 一般字符。
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
		switch(lookahead()){
			case '*':
				// 0到无穷。
				next();
				return digraph.closure();
			case '+':
				// 1到无穷。
				next();
				return digraph.concat(digraph.closure());
			case '?':
				// 0到1。
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
					throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(
						cursor,toNumber(lookahead())));
				}
			}
		}
		else
		{
			throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(cursor,
				toNumber(lookahead())));
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
						throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(
							cursor,toNumber(lookahead())));
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
				throw new IllegalArgumentException("分析量词范围时，在下标%d处遇到了不期望的情况[%s]。".formatted(cursor,
					toNumber(lookahead())));
			}
		}
		if(l==r)
		{
			// 重复n次。{n}。
			int a=l;
			if(a==0)
			{
				// 重复0次，即该图不存在。
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
			// n到无穷。{n,}。
			int a=l;
			if(a==0)
			{
				// 0次，即0边界闭包。
				return digraph.closure();
			}
			else
			{
				// 连接多次。
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
				throw new IllegalArgumentException("分析量词范围时，到下标%d为止，发现量词范围的上限%d小于下限%d。".formatted(
					cursor,b,a));
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
		switch(lookahead()){
			case 'u':
				// uXXXX。四位Unicode。
				next();
				return uxxxx();
			case 'n':
				// 换行。
				next();
				return CharPredicate.create('\n');
			case 't':
				// 制表。
				next();
				return CharPredicate.create('\t');
			case 'r':
				// 回车。
				next();
				return CharPredicate.create('\r');
			case 'a':
				// 警铃。
				next();
				return CharPredicate.create('\u0007');
			case 'f':
				// 竖直制表。
				next();
				return CharPredicate.create('\u000C');
			case 'e':
				// 退出。
				next();
				return CharPredicate.create('\u001B');
			case EOS:
				// 错误结尾。
				throw new IllegalArgumentException("分析转义序列时，在下标%d发现不期望的结束。".formatted(cursor));
			case 'N':
				// 字符名。
				next();
				return name();
			case 'x':
				// 变长码元。
				next();
				return xnumber();
			case 'c':
				next();
				return control();
			case 'd':
				// 十进制数字。
				next();
				return DIGIT;
			case 'D':
				// 非十进制数字。
				next();
				return CharPredicate.not(DIGIT);
			case 'h':
				// 水平空白。
				next();
				return HSPACE;
			case 'H':
				// 非水平空白。
				next();
				return CharPredicate.not(HSPACE);
			case 'v':
				// 竖直空白。
				next();
				return VSPACE;
			case 'V':
				// 非竖直空白。
				next();
				return CharPredicate.not(VSPACE);
			case 's':
				// 一般空白。
				next();
				return WHITE_SPACE;
			case 'S':
				// 非一般空白。
				next();
				return CharPredicate.not(WHITE_SPACE);
			case 'w':
				// 单词。
				next();
				return UWORD;
			case 'W':
				// 非单词。
				next();
				return CharPredicate.not(UWORD);
			case 'p':
				// POSIX字符类。
				next();
				return posix();
			case 'U':
				// Unicode字符类。
				next();
				return unicode();
			default:
				// 默认转义一切字符。
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
			// 第一字符通过。
			codePoint=toNumber(lookahead());
			next();
			if(HEX.match(lookahead()))
			{
				// 第二字符通过。
				codePoint=codePoint<<4+toNumber(lookahead());
				next();
				if(HEX.match(lookahead()))
				{
					// 第三字符通过。
					codePoint=codePoint<<4+toNumber(lookahead());
					next();
					if(HEX.match(lookahead()))
					{
						// 第四字符通过。
						codePoint=codePoint<<4+toNumber(lookahead());
						next();
						return CharPredicate.create(codePoint);
					}
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\uXXXX]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,
			toString(lookahead())));
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
				switch(lookahead()){
					case EOS:
						throw new IllegalArgumentException("分析转义序列[\\N{name}]时，在下标%d发现不期望的情况[%s]。"
							.formatted(cursor,toString(lookahead())));
					case '}':
						next();
						try
						{
							codePoint=Character.codePointOf(n.toString());
							return CharPredicate.create(codePoint);
						}
						catch(IllegalArgumentException e)
						{
							throw new IllegalArgumentException(
								"分析转义序列[\\N{name}]，在下标%d时发现不存在的字符名[%s]。".formatted(cursor,n
									.toString()));
						}
					default:
						n.appendCodePoint(lookahead());
						next();
						break;
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\N{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,
			toString(lookahead())));
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
				switch(lookahead()){
					case EOS:
						throw new IllegalArgumentException("分析转义序列[\\p{name}]时，在下标%d发现不期望的情况[%s]。"
							.formatted(cursor,toString(lookahead())));
					case '}':
						next();
						switch(n.toString()){
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
							case "d":
								return NUMBER;
							case "D":
								return CharPredicate.not(NUMBER);
							case "s":
								return SPACE;
							case "S":
								return CharPredicate.not(SPACE);
							case "w":
								return WORD;
							case "W":
								return CharPredicate.not(WORD);
							default:
								throw new IllegalArgumentException(
									"分析转义序列[\\p{name}]，在下标%d时发现不存在的字符类名[%s]。".formatted(cursor,n
										.toString()));
						}
					default:
						n.appendCodePoint(lookahead());
						next();
						break;
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\p{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,
			toString(lookahead())));
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
			String text;
			Character.UnicodeBlock block;
			Character.UnicodeScript script;
			CharPredicate p;
			next();
			while(true)
			{
				switch(lookahead()){
					case EOS:
						throw new IllegalArgumentException("分析转义序列[\\U{name}]时，在下标%d发现不期望的情况[%s]。"
							.formatted(cursor,toString(lookahead())));
					case '}':
						next();
						text=n.toString();
						switch(text){
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
								return UPRINT;
							case "Blank":
								return UBLANK;
							case "Cntrl":
								return CC;
							case "XDigit":
								return UXDIGIT;
							case "Space":
								return WHITE_SPACE;
							default:
								p=getPredicate(text);
								if(p!=null)
								{
									return p;
								}
								if(text.startsWith("Is"))
								{
									text=text.substring(2);
									// 不同于Java。这里区分大小写。
									switch(text){
										case "Alphabetic":
											return ALPHABETIC;
										case "Ideographic":
											return IDEOGRAPHIC;
										case "Letter":
											return LETTER;
										case "Lowercase":
											return LOWERCASE;
										case "Uppercase":
											return UPPERCASE;
										case "Titlecase":
											return TITLECASE;
										case "Punctuation":
											return PUNCTUATION;
										case "Control":
											return CC;
										case "White_Space":
										case "WhiteSpace":
											return WHITE_SPACE;
										case "Digit":
											return DIGIT;
										case "Hex_Digit":
										case "HexDigit":
											return HEX_DIGIT;
										case "Join_Control":
										case "JoinControl":
											return JOIN_CONTROL;
										case "Noncharacter_Code_Point":
										case "NoncharacterCodePoint":
											return NONCHARACTER_CODE_POINT;
										case "Assigned":
											return ASSIGNED;
										default:
											p=getPredicate(text);
											if(p!=null)
											{
												return p;
											}
											// 默认为书写系统。
											try
											{
												script=Character.UnicodeScript.forName(text);
												return CharPredicate.create(
													codePoint->Character.UnicodeScript.of(
														codePoint)==script,"\\U{Is%s}".formatted(
															text));
											}
											catch(Exception e)
											{
												throw new IllegalArgumentException(
													"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode书写系统[%s]。"
														.formatted(cursor,text));
											}
									}
								}
								else if(text.startsWith("In"))
								{
									// 代码区段。
									text=text.substring(2);
									try
									{
										block=Character.UnicodeBlock.forName(text);
										return CharPredicate.create(
											codePoint->Character.UnicodeBlock.of(codePoint)==block,
											"\\U{In%s}".formatted(text));
									}
									catch(Exception e)
									{
										throw new IllegalArgumentException(
											"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode代码区段[%s]。"
												.formatted(cursor,text));
									}
								}
								else if(text.startsWith("script="))
								{
									// 书写系统。
									text=text.substring(7);
									try
									{
										script=Character.UnicodeScript.forName(text);
										return CharPredicate.create(
											codePoint->Character.UnicodeScript.of(
												codePoint)==script,"\\U{Is%s}".formatted(text));
									}
									catch(Exception e)
									{
										throw new IllegalArgumentException(
											"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode书写系统[%s]。"
												.formatted(cursor,text));
									}
								}
								else if(text.startsWith("sc="))
								{
									// 书写系统。
									text=text.substring(3);
									try
									{
										script=Character.UnicodeScript.forName(text);
										return CharPredicate.create(
											codePoint->Character.UnicodeScript.of(
												codePoint)==script,"\\U{Is%s}".formatted(text));
									}
									catch(Exception e)
									{
										throw new IllegalArgumentException(
											"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode书写系统[%s]。"
												.formatted(cursor,text));
									}
								}
								else if(text.startsWith("block="))
								{
									// 代码区段。
									text=text.substring(6);
									try
									{
										block=Character.UnicodeBlock.forName(text);
										return CharPredicate.create(
											codePoint->Character.UnicodeBlock.of(codePoint)==block,
											"\\U{In%s}".formatted(text));
									}
									catch(Exception e)
									{
										throw new IllegalArgumentException(
											"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode代码区段[%s]。"
												.formatted(cursor,text));
									}
								}
								else if(text.startsWith("blk="))
								{
									// 代码区段。
									text=text.substring(4);
									try
									{
										block=Character.UnicodeBlock.forName(text);
										return CharPredicate.create(
											codePoint->Character.UnicodeBlock.of(codePoint)==block,
											"\\U{In%s}".formatted(text));
									}
									catch(Exception e)
									{
										throw new IllegalArgumentException(
											"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode代码区段[%s]。"
												.formatted(cursor,text));
									}
								}
								else if(text.startsWith("general_category="))
								{
									// 类别。
									text=text.substring(17);
									p=getPredicate(text);
									if(p!=null)
									{
										return p;
									}
									throw new IllegalArgumentException(
										"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode类别[%s]。".formatted(
											cursor,text));
								}
								else if(text.startsWith("gc="))
								{
									// 属性。
									text=text.substring(3);
									p=getPredicate(text);
									if(p!=null)
									{
										return p;
									}
									throw new IllegalArgumentException(
										"分析转义序列[\\U{name}]，在下标%d时发现不存在的Unicode属性[%s]。".formatted(
											cursor,text));
								}
								else
								{
									throw new IllegalArgumentException(
										"分析转义序列[\\U{name}]，在下标%d时发现不可理解参数[%s]。".formatted(cursor,
											text));
								}
						}
					default:
						n.appendCodePoint(lookahead());
						next();
						break;
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\U{name}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,
			toString(lookahead())));
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
							throw new IllegalArgumentException(
								"分析转义序列[\\x{X}]时，在下标%d发现Unicode码元不存在[%04X]。".formatted(cursor,
									codePoint));
						}
					}
					else
					{
						break;
					}
				}
			}
		}
		throw new IllegalArgumentException("分析转义序列[\\x{X}]时，在下标%d发现不期望的情况[%s]。".formatted(cursor,
			toString(lookahead())));
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
			switch(lookahead()){
				case EOS:
					throw new IllegalArgumentException("分析字符类时，在下标%d发现不期望的情况[%s]。".formatted(cursor,
						toString(lookahead())));
				case '\\':
					next();
					or.add(escape());
					break;
				case '&':
					// 逻辑与运算。
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
					// 嵌套字符类。
					next();
					or.add(clazz());
					break;
				case '-':
					// 范围符。
					next();
					if(or.isEmpty()||or.getLast().isRange())
					{
						or.add(CharPredicate.create('-'));
					}
					else
					{
						switch(lookahead()){
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
								or.add(CharPredicate.create(or.removeLast(),CharPredicate.create(
									lookahead())));
								next();
								break;
						}
					}
					break;
				case ']':
					next();
					if(or.isEmpty())
					{
						throw new IllegalArgumentException("分析字符类时，在下标%d发现没有指定范围。".formatted(cursor,
							toString(lookahead())));
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
				case '.':
					// 通配符。
					next();
					or.add(DOT);
					break;
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
		else if(value=='['||value==']'||value=='^'||value=='-'||value=='&')
		{
			return "\\u%04X".formatted(value);
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
	
	/**
	 * 在谓词转字符串过程中获取下属谓词参数。
	 * 
	 * @param text 下属谓词字符串。
	 * @param type 当前谓词类型。0非1与2或。
	 * @return
	 */
	static String getParam(String text,int type)
	{
		switch(type){
			case 0:
				if(text.startsWith("["))
				{
					return text.substring(1,text.length()-1);
				}
				else
				{
					return text;
				}
			case 1:
				if(text.startsWith("[^"))
				{
					return text;
				}
				else if(text.startsWith("["))
				{
					return text.substring(1,text.length()-1);
				}
				else
				{
					return text;
				}
			case 2:
				if(text.startsWith("[^")||text.contains("&"))
				{
					return text;
				}
				else if(text.startsWith("["))
				{
					return text.substring(1,text.length()-1);
				}
				else
				{
					return text;
				}
			default:
				// 不可达。
				return null;
		}
	}
	
	/**
	 * 获得Unicode类别对应谓词。
	 * 
	 * @param category 类别。
	 * @return 对应谓词。
	 */
	static CharPredicate getPredicate(String category)
	{
		switch(category){
			case "Cn":
				return CharPredicate.create(1<<Character.UNASSIGNED,"Cn");
			case "Lu":
				return CharPredicate.create(1<<Character.UPPERCASE_LETTER,"Lu");
			case "Ll":
				return CharPredicate.create(1<<Character.LOWERCASE_LETTER,"Ll");
			case "Lt":
				return CharPredicate.create(1<<Character.TITLECASE_LETTER,"Lt");
			case "Lm":
				return CharPredicate.create(1<<Character.MODIFIER_LETTER,"Lm");
			case "Lo":
				return CharPredicate.create(1<<Character.OTHER_LETTER,"Lo");
			case "Mn":
				return CharPredicate.create(1<<Character.NON_SPACING_MARK,"Mn");
			case "Me":
				return CharPredicate.create(1<<Character.ENCLOSING_MARK,"Me");
			case "Mc":
				return CharPredicate.create(1<<Character.COMBINING_SPACING_MARK,"Mc");
			case "Nd":
				return CharPredicate.create(1<<Character.DECIMAL_DIGIT_NUMBER,"Nd");
			case "Nl":
				return CharPredicate.create(1<<Character.LETTER_NUMBER,"Nl");
			case "No":
				return CharPredicate.create(1<<Character.OTHER_NUMBER,"No");
			case "Zs":
				return CharPredicate.create(1<<Character.SPACE_SEPARATOR,"Zs");
			case "Zl":
				return CharPredicate.create(1<<Character.LINE_SEPARATOR,"Zl");
			case "Zp":
				return CharPredicate.create(1<<Character.PARAGRAPH_SEPARATOR,"Zp");
			case "Cc":
				return CharPredicate.create(1<<Character.CONTROL,"Cc");
			case "Cf":
				return CharPredicate.create(1<<Character.FORMAT,"Cf");
			case "Co":
				return CharPredicate.create(1<<Character.PRIVATE_USE,"Co");
			case "Cs":
				return CharPredicate.create(1<<Character.SURROGATE,"Cs");
			case "Pd":
				return CharPredicate.create(1<<Character.DASH_PUNCTUATION,"Pd");
			case "Ps":
				return CharPredicate.create(1<<Character.START_PUNCTUATION,"Ps");
			case "Pe":
				return CharPredicate.create(1<<Character.END_PUNCTUATION,"Pe");
			case "Pc":
				return CharPredicate.create(1<<Character.CONNECTOR_PUNCTUATION,"Pc");
			case "Po":
				return CharPredicate.create(1<<Character.OTHER_PUNCTUATION,"Po");
			case "Sm":
				return CharPredicate.create(1<<Character.MATH_SYMBOL,"Sm");
			case "Sc":
				return CharPredicate.create(1<<Character.CURRENCY_SYMBOL,"Sc");
			case "Sk":
				return CharPredicate.create(1<<Character.MODIFIER_SYMBOL,"Sk");
			case "So":
				return CharPredicate.create(1<<Character.OTHER_SYMBOL,"So");
			case "Pi":
				return CharPredicate.create(1<<Character.INITIAL_QUOTE_PUNCTUATION,"Pi");
			case "Pf":
				return CharPredicate.create(1<<Character.FINAL_QUOTE_PUNCTUATION,"Pf");
			case "L":
				return CharPredicate.create((1<<Character.UPPERCASE_LETTER)
					|(1<<Character.LOWERCASE_LETTER)|(1<<Character.TITLECASE_LETTER)
					|(1<<Character.MODIFIER_LETTER)|(1<<Character.OTHER_LETTER),"L");
			case "M":
				return CharPredicate.create((1<<Character.NON_SPACING_MARK)
					|(1<<Character.ENCLOSING_MARK)|(1<<Character.COMBINING_SPACING_MARK),"M");
			case "N":
				return CharPredicate.create((1<<Character.DECIMAL_DIGIT_NUMBER)
					|(1<<Character.LETTER_NUMBER)|(1<<Character.OTHER_NUMBER),"N");
			case "Z":
				return CharPredicate.create((1<<Character.SPACE_SEPARATOR)
					|(1<<Character.LINE_SEPARATOR)|(1<<Character.PARAGRAPH_SEPARATOR),"Z");
			case "C":
				return CharPredicate.create((1<<Character.CONTROL)|(1<<Character.FORMAT)
					|(1<<Character.PRIVATE_USE)|(1<<Character.SURROGATE)|(1<<Character.UNASSIGNED),
					"C");
			case "P":
				return CharPredicate.create((1<<Character.DASH_PUNCTUATION)
					|(1<<Character.START_PUNCTUATION)|(1<<Character.END_PUNCTUATION)
					|(1<<Character.CONNECTOR_PUNCTUATION)|(1<<Character.OTHER_PUNCTUATION)
					|(1<<Character.INITIAL_QUOTE_PUNCTUATION)
					|(1<<Character.FINAL_QUOTE_PUNCTUATION),"P");
			case "S":
				return CharPredicate.create((1<<Character.MATH_SYMBOL)
					|(1<<Character.CURRENCY_SYMBOL)|(1<<Character.MODIFIER_SYMBOL)
					|(1<<Character.OTHER_SYMBOL),"S");
			default:
				return null;
		}
	}
}
