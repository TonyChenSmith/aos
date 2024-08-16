package aos.tools.compiler;

/**
 * 词法器回调。在词法器分析出单词但还没被禁止输出前调用。
 * 
 * <p>在2024-08-15时生成。
 *
 * @author Tony Chen Smith
 */
interface LexerCallback
{
	/**
	 * 处理单词。
	 * 
	 * @param token   单词。
	 * @param context 上下文。
	 */
	abstract void process(Token token,Context context);
}
