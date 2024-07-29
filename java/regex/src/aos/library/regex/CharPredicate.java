package aos.library.regex;

import java.io.Serializable;
import java.util.function.IntPredicate;

/**
 * 字符谓词类。
 * 
 * <p>可以通过三种方式构造谓词：
 * <ol>
 * <li>单码元构造</li>
 * <li>码元范围构造</li>
 * <li>逻辑运算组合构造</li>
 * </ol>
 * 设计模式参考外观模式。隐藏三种构造对内部的细节。
 * 
 * <p>在2024-06-16时生成。
 * 
 * @author Tony Chen Smith
 */
abstract class CharPredicate implements Serializable
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=8550723343181471879L;

	/**
	 * 结构空谓词。
	 */
	static final CharPredicate EPSILON;
	
	/**
	 * 全字符谓词，从最小码元到最大码元。
	 */
	static final CharPredicate ALL;
	
	/**
	 * 通配符谓词，去掉一个换行符。
	 */
	static final CharPredicate DOT;
	
	/**
	 * 十六进制数字字符。
	 */
	static final CharPredicate HEX;
	
	/**
	 * 十进制数字字符。
	 */
	static final CharPredicate NUMBER;
	
	/**
	 * 水平空白符。
	 */
	static final CharPredicate HSPACE;
	
	/**
	 * 竖直空白符。
	 */
	static final CharPredicate VSPACE;
	
	/**
	 * 空白符。
	 */
	static final CharPredicate SPACE;
	
	/**
	 * 单词字符。
	 */
	static final CharPredicate WORD;
	
	/**
	 * 小写字母。
	 */
	static final CharPredicate LOWER;
	
	/**
	 * 大写字母。
	 */
	static final CharPredicate UPPER;
	
	/**
	 * ASCII码。
	 */
	static final CharPredicate ASCII;
	
	/**
	 * 字母。
	 */
	static final CharPredicate ALPHA;
	
	/**
	 * 字母数字。
	 */
	static final CharPredicate ALNUM;
	
	/**
	 * 标点。
	 */
	static final CharPredicate PUNCT;
	
	/**
	 * 可见字符。
	 */
	static final CharPredicate GRAPH;
	
	/**
	 * 可打印字符。
	 */
	static final CharPredicate PRINT;
	
	/**
	 * 空白和制表符。
	 */
	static final CharPredicate BLANK;
	
	/**
	 * 控制字符。
	 */
	static final CharPredicate CNTRL;
	
	/**
	 * Unicode单词字符。
	 */
	static final CharPredicate UWORD;
	
	/**
	 * Unicode小写。
	 */
	static final CharPredicate LOWERCASE;
	
	/**
	 * Unicode大写。
	 */
	static final CharPredicate UPPERCASE;
	
	/**
	 * Unicode字母表。
	 */
	static final CharPredicate ALPHABETIC;
	
	/**
	 * Unicode数字。
	 */
	static final CharPredicate DIGIT;
	
	/**
	 * Unicode字母数字。
	 */
	static final CharPredicate UALNUM;
	
	/**
	 * Unicode标点。
	 */
	static final CharPredicate PUNCTUATION;
	
	/**
	 * Unicode可见字符。
	 */
	static final CharPredicate UGRAPH;
	
	/**
	 * Unicode可打印字符。
	 */
	static final CharPredicate UPRINT;
	
	/**
	 * Unicode空白制表。
	 */
	static final CharPredicate UBLANK;
	
	/**
	 * Unicode控制字符。
	 */
	static final CharPredicate CC;
	
	/**
	 * Unicode16进制字符。
	 */
	static final CharPredicate UXDIGIT;
	
	/**
	 * Unicode表意文字。
	 */
	static final CharPredicate IDEOGRAPHIC;
	
	/**
	 * Unicode字母。
	 */
	static final CharPredicate LETTER;
	
	/**
	 * Unicode标题。
	 */
	static final CharPredicate TITLECASE;
	
	/**
	 * Unicode空白。
	 */
	static final CharPredicate WHITE_SPACE;
	
	/**
	 * Unicode16进制数。
	 */
	static final CharPredicate HEX_DIGIT;
	
	/**
	 * Unicode连接控制。
	 */
	static final CharPredicate JOIN_CONTROL;
	
	/**
	 * Unicode非字符码元。
	 */
	static final CharPredicate NONCHARACTER_CODE_POINT;
	
	/**
	 * Unicode已分配字符。
	 */
	static final CharPredicate ASSIGNED;
	
	/**
	 * 初始化常量。
	 */
	static
	{
		EPSILON=new Single(0);
		LOWER=new Range('a','z');
		UPPER=new Range('A','Z');
		ALPHA=or(LOWER,UPPER);
		ASCII=new Range(0,0x7F);
		ALL=new Range(Character.MIN_CODE_POINT,Character.MAX_CODE_POINT);
		DOT=CharPredicate.and(ALL,not(create('\n')));
		NUMBER=new Range('0','9');
		CharPredicate phex=CharPredicate.or(new Range('a','f'),new Range('A','F'));
		HEX=CharPredicate.or(NUMBER,phex);
		HSPACE=or(or(or(or(new Single(' '),new Single('\t')),or(new Single('\u00A0'),new Single('\u1680'))),or(or(new Single('\u180E'),new Range('\u2000','\u200A')),or(new Single('\u202F'),new Single('\u205F')))),new Single('\u3000'));
		VSPACE=or(or(or(new Single('\n'),new Single('\u000B')),or(new Single('\u000C'),new Single('\r'))),or(or(new Single('\u0085'),new Single('\u2028')),new Single('\u2029')));
		SPACE=or(or(or(new Single(' '),new Single('\t')),or(new Single('\n'),new Single('\u000B'))),or(new Single('\u000C'),new Single('\r')));
		WORD=or(ALPHA,or(new Single('_'),NUMBER));
		ALNUM=or(ALPHA,NUMBER);
		CNTRL=or(new Range(0,0x1F),new Single(0x7F));
		PUNCT=or(or(new Range(0x20,0x2F),new Range(0x3A,0x40)),or(new Range(0x5B,0x60),new Range(0x7B,0x7E)));
		GRAPH=or(ALNUM,PUNCT);
		PRINT=or(GRAPH,new Single(0x20));
		BLANK=or(new Single(' '),new Single('\t'));
		
		//下面为Unicode区。
		LOWERCASE=new Caller(Character::isLowerCase,"\\U{IsLowercase}");
		UPPERCASE=new Caller(Character::isUpperCase,"\\U{IsUppercase}");
		ALPHABETIC=new Caller(Character::isAlphabetic,"\\U{IsAlphabetic}");
		DIGIT=new Caller(Character::isDigit,"\\U{IsDigit}");
		UALNUM=or(ALPHABETIC,DIGIT);
		PUNCTUATION=new Caller(codePoint->{
			switch(Character.getType(codePoint))
			{
				case Character.CONNECTOR_PUNCTUATION,Character.DASH_PUNCTUATION,Character.START_PUNCTUATION,
					Character.END_PUNCTUATION,Character.OTHER_PUNCTUATION,Character.INITIAL_QUOTE_PUNCTUATION,Character.FINAL_QUOTE_PUNCTUATION->{return true;}
				default->{return false;}
			}
		},"\\U{IsPunctuation}");
		UGRAPH=new Caller(codePoint->{
			switch(Character.getType(codePoint))
			{
				case Character.SPACE_SEPARATOR,Character.LINE_SEPARATOR,Character.PARAGRAPH_SEPARATOR,
					Character.CONTROL,Character.SURROGATE,Character.UNASSIGNED->{return false;}
				default->{return true;}
			}
		},"[^\\U{Graph]");
		IDEOGRAPHIC=new Caller(Character::isIdeographic,"\\U{IsIdeographic}");
		LETTER=new Caller(Character::isLetter,"\\U{IsLetter}");
		TITLECASE=new Caller(Character::isTitleCase,"\\U{IsTitleCase");
		CC=new Caller(codePoint->Character.getType(codePoint)==Character.CONTROL,"\\U{gc=Cc}");
		WHITE_SPACE=new Caller(codePoint->{
			switch(Character.getType(codePoint))
			{
				case Character.SPACE_SEPARATOR,Character.LINE_SEPARATOR,Character.PARAGRAPH_SEPARATOR->{return true;}
			}
			if((codePoint>=0x9&&codePoint<=0xD)||codePoint==0x85)
			{
				return true;
			}
			return false;
		},"\\U{IsWhite_Space}");
		CharPredicate up2=or(or(DIGIT,or(new Range(0x30,0x39),new Range(0x41,0x46))),or(or(new Range(0x61,0x66),new Range(0xFF10,0xFF19)),or(new Range(0xFF21,0xFF26),new Range(0xFF41,0xFF46))));
		HEX_DIGIT=new Caller(codePoint->up2.match(codePoint),"\\U{IsHex_Digit}");
		JOIN_CONTROL=new Range(0x200C,0x200D);
		NONCHARACTER_CODE_POINT=new Caller(codePoint->{
			return (codePoint&0xFFFE)==0xFFFE||(codePoint<=0xFDEF&&codePoint>=0xFDD0);
		},"\\U{IsNoncharacter_Code_Point}");
		ASSIGNED=new Caller(codePoint->Character.getType(codePoint)!=Character.UNASSIGNED,"\\U{Assigned}");
		UPRINT=new Caller(codePoint->{
			return false;
		},"\\U{Print}");
		CharPredicate up=and(WHITE_SPACE,not(or(or(Parser.getPredicate("Zl"),Parser.getPredicate("Zp")),or(new Range(0xA,0xD),new Single(0x85)))));
		UBLANK=new Caller(codePoint->up.match(codePoint),"\\U{Blank}");
		CharPredicate up1=or(Parser.getPredicate("Nd"),HEX_DIGIT);
		UXDIGIT=new Caller(codePoint->up1.match(codePoint),"\\U{XDigit}");
		CharPredicate up3=or(or(or(ALPHABETIC,Parser.getPredicate("Mn")),or(Parser.getPredicate("Me"),Parser.getPredicate("Mc"))),or(or(DIGIT,Parser.getPredicate("Pc")),JOIN_CONTROL));
		UWORD=new Caller(codePoint->up3.match(codePoint),"\\w");
	}
	
	/**
	 * 匹配码元。
	 * 
	 * @param codePoint 码元。
	 * @return 码元符合谓词匹配规则时返回真，其余情况返回假。
	 */
	abstract boolean match(int codePoint);
	
	/**
	 * 谓词是否为一个范围。
	 * 
	 * @return 如果结果为一个范围，返回真。
	 */
	abstract boolean isRange();
	
	/**
	 * 创建单字符谓词。
	 * 
	 * @param matched 被匹配码元。
	 * @return 构造出的谓词。
	 */
	static CharPredicate create(int matched)
	{
		return create(matched,true);
	}
	
	/**
	 * 创建单字符谓词。
	 * 
	 * @param matched 被匹配码元。
	 * @param input 是否为正则表达式输入码元。
	 * @return 构造出的谓词。
	 */
	static CharPredicate create(int matched,boolean input)
	{
		return matched==0&&!input?EPSILON:new Single(matched);
	}
	
	/**
	 * 创建范围字符谓词。
	 * 
	 * @param lower 范围下限。
	 * @param upper 范围上限。
	 * @return 构造出的谓词。
	 */
	static CharPredicate create(int lower,int upper)
	{
		return new Range(lower,upper);
	}
	
	/**
	 * 创建范围字符谓词。
	 * 
	 * @param lower 范围下限谓词。
	 * @param upper 范围上限谓词。
	 * @return 构造出的谓词。
	 */
	static CharPredicate create(CharPredicate lower,CharPredicate upper)
	{
		if(lower instanceof Single low&&upper instanceof Single up)
		{
			return new Range(low.matched,up.matched);
		}
		else
		{
			throw new ClassCastException("构造范围时输入的谓词类型不正确。");
		}
	}
	
	/**
	 * 创造方法引用实现的范围字符谓词。
	 * 
	 * @param predicate 方法引用。
	 * @param text 对应的字符类表示。
	 * @return 对应谓词。
	 */
	static CharPredicate create(CallerPredicate predicate,String text)
	{
		return new Caller(predicate,text);
	}
	
	/**
	 * 创造对应字符类型的范围字符谓词。
	 * 
	 * @param mask 字符类型掩码。
	 * @param text 对应字符串表示。
	 * @return
	 */
	static CharPredicate create(int mask,String text)
	{
		return new Caller(codePoint->(mask&(1<<Character.getType(codePoint)))!=0,"\\U{%s}".formatted(text));
	}
	
	/**
	 * 对两个谓词进行与运算。
	 * 
	 * @param left 左值。
	 * @param right 右值。
	 * @return 运算后的谓词。
	 */
	static CharPredicate and(CharPredicate left,CharPredicate right)
	{
		return new And(left,right);
	}
	
	/**
	 * 对两个谓词进行或运算。
	 * 
	 * @param left 左值。
	 * @param right 右值。
	 * @return 运算后的谓词。
	 */
	static CharPredicate or(CharPredicate left,CharPredicate right)
	{
		return new Or(left,right);
	}
	
	/**
	 * 对谓词进行非运算。
	 * 
	 * @param right 右值。
	 * @return 运算后的谓词。
	 */
	static CharPredicate not(CharPredicate right)
	{
		return new Not(right);
	}
	
	/**
	 * 单字符谓词类。
	 * 
	 * <p>在2024-06-16时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	private static class Single extends CharPredicate
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=-6054468537224131496L;
		
		/**
		 * 被匹配码元。
		 */
		private final int matched;
		
		/**
		 * 使用单一码元构造谓词。
		 * 
		 * @param matched 被匹配码元。
		 */
		private Single(int matched)
		{
			this.matched=matched;
		}
		
		@Override
		boolean match(int codePoint)
		{
			return matched==codePoint;
		}
		
		@Override
		boolean isRange()
		{
			return false;
		}
		
		@Override
		public String toString()
		{
			return Parser.toString(matched);
		}
	}
	
	/**
	 * 范围字符谓词类。
	 * 
	 * <p>在2024-06-16时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	private static class Range extends CharPredicate
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=-8850683211376552654L;

		/**
		 * 范围下限。
		 */
		private final int lower;
		
		/**
		 * 范围上限。
		 */
		private final int upper;
		
		/**
		 * 使用上下限字符码元构造谓词。
		 * 
		 * @param lower 范围下限。
		 * @param upper 范围上限。
		 */
		private Range(int lower,int upper)
		{
			this.lower=lower;
			this.upper=upper;
		}
		
		@Override
		boolean match(int codePoint)
		{
			return lower<=codePoint&&upper>=codePoint;
		}
		
		@Override
		boolean isRange()
		{
			//有且仅有a-b形式会构建该类。默认为一个范围。
			return true;
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append('[');
			result.append(Parser.toString(lower));
			result.append('-');
			result.append(Parser.toString(upper));
			return result.append(']').toString();
		}
	}
	
	/**
	 * 可序列化的函数式接口。
	 * 
	 * <p>在2024-07-28时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	static interface CallerPredicate extends Serializable,IntPredicate
	{
		
	}
	
	/**
	 * 方法引用谓词类。
	 * 
	 * <p>在2024-07-28时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	private static class Caller extends CharPredicate
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=-2015631955863210638L;
		
		/**
		 * 方法引用谓词。
		 */
		private final CallerPredicate predicate;
		
		/**
		 * 字符类表示。
		 */
		private final String text;
		
		/**
		 * 使用方法引用构造该谓词。
		 * 
		 * @param predicate 方法引用。
		 * @param text 对应的字符类表示。
		 */
		private Caller(CallerPredicate predicate,String text)
		{
			this.predicate=predicate;
			this.text=text;
		}
		
		@Override
		boolean match(int codePoint)
		{
			return predicate.test(codePoint);
		}

		@Override
		boolean isRange()
		{
			//方法调用视为范围。
			return true;
		}
		
		@Override
		public String toString()
		{
			return text;
		}
	}
	
	/**
	 * 与运算字符谓词类。
	 * 
	 * <p>在2024-06-16时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	private static class And extends CharPredicate
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=6658173815371247278L;

		/**
		 * 左值。
		 */
		private final CharPredicate left;
		
		/**
		 * 右值。
		 */
		private final CharPredicate right;
		
		/**
		 * 构造与运算谓词。
		 * 
		 * @param left 左值。
		 * @param right 右值。
		 */
		private And(CharPredicate left,CharPredicate right)
		{
			this.left=left;
			this.right=right;
		}
		
		@Override
		boolean match(int codePoint)
		{
			return left.match(codePoint)&&right.match(codePoint);
		}
		
		@Override
		boolean isRange()
		{
			//逻辑运算均视为范围。本质是逻辑运算的该方法不重要。
			return true;
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append('[').append(Parser.getParam(left.toString(),1)).append("&").append(Parser.getParam(right.toString(),1)).append(']');
			return result.toString();
		}
	}
	
	/**
	 * 或运算字符谓词类。
	 * 
	 * <p>在2024-06-16时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	private static class Or extends CharPredicate
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=7547941211030867183L;

		/**
		 * 左值。
		 */
		private final CharPredicate left;
		
		/**
		 * 右值。
		 */
		private final CharPredicate right;
		
		/**
		 * 构造或运算谓词。
		 * 
		 * @param left 左值。
		 * @param right 右值。
		 */
		private Or(CharPredicate left,CharPredicate right)
		{
			this.left=left;
			this.right=right;
		}
		
		@Override
		boolean match(int codePoint)
		{
			return left.match(codePoint)||right.match(codePoint);
		}
		
		@Override
		boolean isRange()
		{
			return true;
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append('[').append(Parser.getParam(left.toString(),2)).append(Parser.getParam(right.toString(),2)).append(']');
			return result.toString();
		}
	}
	
	/**
	 * 非运算字符谓词类。
	 * 
	 * <p>在2024-06-16时生成。
	 * 
	 * @author Tony Chen Smith
	 */
	private static class Not extends CharPredicate
	{
		/**
		 * 序列化号。
		 */
		private static final long serialVersionUID=-5870870430255508834L;
		
		/**
		 * 右值。
		 */
		private final CharPredicate right;
		
		/**
		 * 构造非运算谓词。
		 * 
		 * @param right 右值。
		 */
		private Not(CharPredicate right)
		{
			this.right=right;
		}
		
		@Override
		boolean match(int codePoint)
		{
			return !right.match(codePoint);
		}
		
		@Override
		boolean isRange()
		{
			return true;
		}
		
		@Override
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			String param=Parser.getParam(right.toString(),0);
			result.append('[');
			if(param.startsWith("^"))
			{
				result.append(param.substring(1));
			}
			else
			{
				result.append('^').append(param);
			}
			result.append(']');
			return result.toString();
		}
	}
}
