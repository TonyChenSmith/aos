/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
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
	 * 无动作回调。
	 */
	static final LexerCallback NONACTION=(t,c)->{};
	
	/**
	 * 换行回调。
	 */
	static final LexerCallback NEW_LINE=(t,c)->{
		c.onNewLine();
	};
	
	/**
	 * 处理单词。
	 * 
	 * @param token   单词。
	 * @param context 上下文。
	 */
	abstract void process(Token token,Context context);
}
