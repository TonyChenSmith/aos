/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

/**
 * <p>在2024-08-23时生成。
 *
 * @author Tony Chen Smith
 */
public class ClaySyntaxError extends Error
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=-3900284905175273116L;
	
	/**
	 * 错误行信息。
	 */
	private final String lines;
	
	/**
	 * 构造异常。
	 * 
	 * @param message 错误信息。
	 */
	public ClaySyntaxError(String message)
	{
		lines=message;
	}
	
	@Override
	public String toString()
	{
		return lines;
	}
}
