package aos.tools.compiler;

import java.util.LinkedList;
import java.util.Map;

import aos.tools.compiler.LRTable.Action;

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
	 * 文件路径。
	 */
	private String file;
	
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
		this.file="";
	}
	
	/**
	 * 重置语法器。需要更新词法器。
	 * 
	 * @param lexer 词法器。
	 * @param file 文件路径。
	 */
	void reset(Lexer lexer,String file)
	{
		this.lexer=lexer;
		this.file=file;
		context.reset();
		context.states.push(0);
	}
	
	/**
	 * 语法分析获得语法树。假设已经重置完了。
	 * 
	 * @return 语法树的根语法符号。
	 */
	SyntaxSymbol parse()
	{
		while(true)
		{
			if(choose(lexer.scan(context)))
			{
				break;
			}
		}
		return context.symbols.peek();
	}
	
	/**
	 * 选择行为。
	 * 
	 * @param token 词法单元。
	 * @return 返回真时结束选择。
	 */
	private boolean choose(Token token)
	{
		Action action=context.table.get(context.states.peek(),token.name());
		switch(action.type())
		{
			case ACCEPT:
				//接受。
				if(context.symbols.size()==1&&token.name().equals("$eof"))
				{
					return true;
				}
				throw new RuntimeException("不可归约。");
			case ERROR:
				//报错。
				context.onError((String)action.param(),file);
			case GOTO:
				//不可达。
				throw new IllegalArgumentException("不可达GOTO行为。");
			case REDUCE:
				//归约。
				return reduce(action,token);
			case SHIFT:
				//移入。
				return shift(action,token);
			default:
				//不可达。
				throw new IllegalArgumentException("未知类型。");
		}
	}
	
	/**
	 * 移入操作。
	 * 
	 * @param action 行为。
	 * @param token 词法单元。
	 * @return 返回真时结束行为。
	 */
	private boolean shift(Action action,Token token)
	{
		int current=context.states.peek();
		context.table.getState(current).process(token,context);
		context.states.push((Integer)action.param());
		context.symbols.push(token);
		return false;
	}
	
	/**
	 * 归约操作。
	 * 
	 * @param action 行为。
	 * @param token 词法单元。
	 * @return 返回真时结束行为。
	 */
	private boolean reduce(Action action,Token token)
	{
		int current=context.states.peek();
		LinkedList<SyntaxSymbol> buffer=new LinkedList<>();
		context.table.getState(current).process(token,context);
		int count=0;
		buffer.clear();
		for(int state:context.states)
		{
			if(context.table.getState(state).contains((String)action.param()))
			{
				count++;
			}
		}
		for(int count1=0;count1<count;count1++)
		{
			context.states.pop();
			buffer.push(context.symbols.pop());
		}
		Noterminal symbol=new Noterminal((String)action.param(),buffer);
		for(Map.Entry<String,String> entry:context.getVariables(symbol.name()).entrySet())
		{
			symbol.set(entry.getKey(),entry.getValue());
		}
		current=context.states.peek();
		//推入归约符号。
		context.table.getState(current).process(symbol,context);
		//正常构造情况下该行为必为GOTO。
		action=context.table.get(current,symbol.name());
		current=(Integer)action.param();
		context.states.push(current);
		context.symbols.push(symbol);
		return choose(token);
	}
}
