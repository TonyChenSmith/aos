/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

import java.util.HashMap;
import java.util.Map;

/**
 * 层级表。
 * 
 * <p>用于有作用域需求的语音存储标识符用。
 * 
 * <p>其调用方式如同映射，存储结构分层级，然后查询值时先查新层级，新层级未找到时查询老层级。
 * 
 * <p>键值暂时固定为字符串类型。
 * 
 * <p>在2024-08-13时生成。
 *
 * @author Tony Chen Smith
 */
final class LayerTable
{
	/**
	 * 当前表所存映射。
	 */
	private final Map<String,String> map;
	
	/**
	 * 上一层级的表。
	 */
	private final LayerTable parent;
	
	/**
	 * 构造层级表。
	 * 
	 * @param parent 上一层级表。
	 */
	private LayerTable(LayerTable parent)
	{
		this.parent=parent;
		map=new HashMap<>();
	}
	
	@Override
	public String toString()
	{
		StringBuilder result;
		if(parent!=null)
		{
			result=new StringBuilder(parent.toString()).append('\n');
		}
		else
		{
			result=new StringBuilder();
		}
		return result.append("LayerTable@").append(Integer.toHexString(hashCode())).append(':').append(map).toString();
	}
	
	/**
	 * 返回当前层级的不可改映射。
	 * 
	 * @return 当前不可改映射。
	 */
	Map<String,String> current()
	{
		return Map.copyOf(map);
	}
	
	/**
	 * 删除当前层级。
	 * 
	 * @return 上一层级表。
	 */
	LayerTable deleteLayer()
	{
		return parent;
	}
	
	/**
	 * 获得属性值。
	 * 
	 * @param key 键。
	 * 
	 * @return 对应的值，或空值。
	 */
	String get(String key)
	{
		final String result=map.get(key);
		if(result==null)
		{
			return parent==null?null:parent.get(key);
		}
		return result;
	}
	
	/**
	 * 构造新层级。
	 * 
	 * @return 新层级表。
	 */
	LayerTable newLayer()
	{
		return new LayerTable(this);
	}
	
	/**
	 * 放入一对键值对。
	 * 
	 * @param key   键。
	 * @param value 值。
	 */
	void put(String key,String value)
	{
		map.put(key,value);
	}
	
	/**
	 * 产生一个根层级表。
	 * 
	 * @return 根层级表。
	 */
	static LayerTable root()
	{
		return new LayerTable(null);
	}
}
