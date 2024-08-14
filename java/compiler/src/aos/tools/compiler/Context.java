package aos.tools.compiler;

import java.util.Set;
import java.util.TreeSet;

/**
 * 编译器前端上下文。
 * 
 * <p>在2024-08-13时生成。
 *
 * @author Tony Chen Smith
 */
final class Context
{
	/**
	 * 词法器模式优先级表。
	 */
	final Table<Integer,String,Object> patterns;
	
	/**
	 * 全局禁用单词名s。
	 */
	final Set<String> banTokens;
	
	/**
	 * 构造上下文。
	 */
	Context()
	{
		//column=pattern,name
		patterns=new Table<>();
		banTokens=new TreeSet<>();
	}
	
	
}
