package aos.library.regex;

import java.io.Serializable;

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
	 * 初始化常量。
	 */
	static
	{
		EPSILON=new Single(0);
		ALL=new Range(Character.MIN_CODE_POINT,Character.MAX_CODE_POINT);
		DOT=CharPredicate.and(ALL,not(create('\n')));
		NUMBER=new Range('0','9');
		CharPredicate phex=CharPredicate.or(new Range('a','f'),new Range('A','F'));
		HEX=CharPredicate.or(NUMBER,phex);
		HSPACE=or(or(or(or(new Single(' '),new Single('\t')),or(new Single('\u00A0'),new Single('\u1680'))),or(or(new Single('\u180E'),new Range('\u2000','\u200A')),or(new Single('\u202F'),new Single('\u205F')))),new Single('\u3000'));
		VSPACE=or(or(or(new Single('\n'),new Single('\u000B')),or(new Single('\u000C'),new Single('\r'))),or(or(new Single('\u0085'),new Single('\u2028')),new Single('\u2029')));
		SPACE=or(or(or(new Single(' '),new Single('\t')),or(new Single('\n'),new Single('\u000B'))),or(new Single('\u000C'),new Single('\r')));
		WORD=or(or(new Range('a','z'),new Range('A','Z')),or(new Single('_'),NUMBER));
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
			return Character.isAlphabetic(matched)?Character.toString(matched):"U+%04X".formatted(matched);
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
			if(Character.isAlphabetic(lower))
			{
				result.appendCodePoint(lower);
			}
			else
			{
				result.append("U+%04X".formatted(lower));
			}
			result.append('-');
			if(Character.isAlphabetic(upper))
			{
				result.appendCodePoint(upper);
			}
			else
			{
				result.append("U+%04X".formatted(upper));
			}
			return result.toString();
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
			result.append('(').append(left).append(")&(").append(right).append(')');
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
			result.append('(').append(left).append(")|(").append(right).append(')');
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
			result.append("!(").append(right).append(')');
			return result.toString();
		}
	}
}
