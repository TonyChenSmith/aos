package aos.tools.compiler;

/**
 * 语法器。
 * 
 * <p>在2024-08-19时生成。
 *
 * @author Tony Chen Smith
 */
final class Parser
{
	/**
	 * 词法器。
	 */
	private Lexer lexer;
	
	/**
	 * 上下文。
	 */
	private final Context context;
	
	/**
	 * 构造语法器。
	 * 
	 * @param context 上下文。
	 */
	Parser(Context context)
	{
		this.context=context;
	}
	
	/**
	 * 重置语法器。需要更新词法器。
	 * 
	 * @param lexer 词法器。
	 */
	void reset(Lexer lexer)
	{
		this.lexer=lexer;
		context.reset();
		context.states.push(0);
	}
	
	/**
	 * 语法分析获得语法树。假设已经重置完了。
	 * 
	 * @return 语法树。
	 */
	SyntaxSymbol parse()
	{
		while(true)
		{
			Token token=lexer.scan(context);
		}
	}
}
