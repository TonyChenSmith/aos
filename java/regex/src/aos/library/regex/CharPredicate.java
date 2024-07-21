package aos.library.regex;

/**
 * 字符谓词类。
 * <p>
 * 可以通过三种方式构造谓词：
 * <ol>
 * <li>单码元构造</li>
 * <li>码元范围构造</li>
 * <li>逻辑运算组合构造</li>
 * </ol>
 * 设计模式参考外观模式。隐藏三种构造对内部的细节。
 * <p>
 * 在2024-06-16时生成。<br>
 * @author Tony Chen Smith
 */
abstract class CharPredicate
{
	/**
	 * 空谓词。
	 */
	static final CharPredicate EPSILON=new Single(0);
	
	/**
	 * 匹配码元。
	 * 
	 * @param codePoint 码元。
	 * @return 码元符合谓词匹配规则时返回真，其余情况返回假。
	 */
	abstract boolean match(int codePoint);
	
	/**
	 * 创建单字符谓词。
	 * 
	 * @param matched 被匹配码元。
	 * @return 构造出的谓词。
	 */
	static CharPredicate create(int matched)
	{
		return matched==0?EPSILON:new Single(matched);
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
	 * <p>
	 * 在2024-06-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	private static class Single extends CharPredicate
	{
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
		public String toString()
		{
			return Character.isAlphabetic(matched)?Character.toString(matched):"U+%04X".formatted(matched);
		}
	}
	
	/**
	 * 范围字符谓词类。
	 * <p>
	 * 在2024-06-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	private static class Range extends CharPredicate
	{
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
	 * <p>
	 * 在2024-06-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	private static class And extends CharPredicate
	{
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
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append('(').append(left).append(")&(").append(right).append(')');
			return result.toString();
		}
	}
	
	/**
	 * 或运算字符谓词类。
	 * <p>
	 * 在2024-06-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	private static class Or extends CharPredicate
	{
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
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append('(').append(left).append(")|(").append(right).append(')');
			return result.toString();
		}
	}
	
	/**
	 * 非运算字符谓词类。
	 * <p>
	 * 在2024-06-16时生成。<br>
	 * @author Tony Chen Smith
	 */
	private static class Not extends CharPredicate
	{
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
		public String toString()
		{
			StringBuilder result=new StringBuilder();
			result.append("!(").append(right).append(')');
			return result.toString();
		}
	}
}
