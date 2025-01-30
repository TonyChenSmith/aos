/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

import java.util.Set;
import java.util.TreeSet;

/**
 * 基础整形集。
 * 
 * <p>在2024-08-14时生成。
 *
 * @author Tony Chen Smith
 */
final class IntSet
{
	/**
	 * 内部集合。
	 */
	private final Set<Integer> set;
	
	/**
	 * 构造一个整形集。
	 */
	IntSet()
	{
		set=new TreeSet<>();
	}
	
	/**
	 * 构造一个预定义的整形集。
	 * 
	 * @param 数组。
	 */
	IntSet(int[] set)
	{
		this();
		add(set);
	}
	
	@Override
	public boolean equals(Object obj)
	{
		if(obj instanceof final IntSet iset)
		{
			return set.equals(iset.set);
		}
		return false;
	}
	
	@Override
	public int hashCode()
	{
		return set.hashCode();
	}
	
	@Override
	public String toString()
	{
		return set.toString();
	}
	
	/**
	 * 将单个值添加到整形集中。
	 * 
	 * @param value 值。
	 */
	void add(int value)
	{
		set.add(value);
	}
	
	/**
	 * 将数组集添加到整形集中。
	 * 
	 * @param set 数组集。
	 */
	void add(int[] set)
	{
		for(final int value:set)
		{
			this.set.add(value);
		}
	}
	
	/**
	 * 检查集中是否包含当前元素。
	 * 
	 * @param value 值。
	 * 
	 * @return 如果有则返回真。
	 */
	boolean contain(int value)
	{
		return set.contains(value);
	}
	
	/**
	 * 获得整形集的集结果。
	 * 
	 * @return 结果集数组。
	 */
	int[] get()
	{
		return set.stream().mapToInt(Integer::intValue).sorted().toArray();
	}
}
