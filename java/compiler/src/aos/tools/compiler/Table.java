package aos.tools.compiler;

import java.io.Serializable;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

/**
 * 表。由三个类型参数确定的二维数据结构。V代表列，H代表行，C代表内容。
 * 
 * <p>在2024-08-14时生成。
 *
 * @author Tony Chen Smith
 */
final class Table<V,H,C> implements Serializable
{
	/**
	 * 序列化号。
	 */
	private static final long serialVersionUID=-505275319090267359L;
	
	/**
	 * 列标集。
	 */
	private final Set<H> columns;
	
	/**
	 * 表载体。
	 */
	private final Map<V,Map<H,C>> table;
	
	/**
	 * 构造表。
	 */
	Table()
	{
		table=new TreeMap<>();
		columns=new TreeSet<>();
	}
	
	/**
	 * 获得当前存在列数。
	 * 
	 * @return 列数。
	 */
	int columnSize()
	{
		return columns.size();
	}
	
	/**
	 * 获得值。
	 * 
	 * @param row    行。
	 * @param column 列。
	 * 
	 * @return 对应值，或空值。
	 */
	C get(V row,H column)
	{
		return get(row,column,null);
	}
	
	/**
	 * 获得值。
	 * 
	 * @param row          行。
	 * @param column       列。
	 * @param defaultValue 默认值。
	 * 
	 * @return 对应值，或默认值。
	 */
	C get(V row,H column,C defaultValue)
	{
		if(table.containsKey(row))
		{
			return table.get(row).getOrDefault(column,defaultValue);
		}
		return null;
	}
	
	/**
	 * 获取列。
	 * 
	 * @param value 值。
	 * 
	 * @return 对应列标。
	 */
	H getColumn(C value)
	{
		for(final V crow:table.keySet())
		{
			final Map<H,C> map=table.get(crow);
			if(map.containsValue(value))
			{
				for(final Map.Entry<H,C> entry:map.entrySet())
				{
					if(entry.getValue().equals(value))
					{
						return entry.getKey();
					}
				}
			}
		}
		return null;
	}
	
	/**
	 * 获得列集。
	 * 
	 * @return 当前列集。不可改。
	 */
	Set<H> getColumns()
	{
		return Collections.unmodifiableSet(columns);
	}
	
	/**
	 * 获取一列所有内容。
	 * 
	 * @param column 列。
	 * 
	 * @return 对应集合，不存在的不会添加。
	 */
	Set<C> getColumnValues(H column)
	{
		if(columns.contains(column))
		{
			final Set<C> result=new TreeSet<>();
			for(final V crow:table.keySet())
			{
				final C value=table.get(crow).get(column);
				if(value!=null)
				{
					result.add(value);
				}
			}
			return result;
		}
		return Collections.emptySet();
	}
	
	/**
	 * 获取行。
	 * 
	 * @param value 值。
	 * 
	 * @return 对应行标。
	 */
	V getRow(C value)
	{
		for(final V crow:table.keySet())
		{
			final Map<H,C> map=table.get(crow);
			if(map.containsValue(value))
			{
				return crow;
			}
		}
		return null;
	}
	
	/**
	 * 获得行集。
	 * 
	 * @return 当前行集。不可改。
	 */
	Set<V> getRows()
	{
		return Collections.unmodifiableSet(table.keySet());
	}
	
	/**
	 * 放置值。
	 * 
	 * @param row    行。
	 * @param column 列。
	 * @param value  值。
	 */
	void put(V row,H column,C value)
	{
		Map<H,C> rows;
		if(table.containsKey(row))
		{
			rows=table.get(row);
		}
		else
		{
			rows=new TreeMap<>();
			table.put(row,rows);
		}
		if(!columns.contains(column))
		{
			columns.add(column);
		}
		rows.put(column,value);
	}
	
	/**
	 * 获得当前行数。
	 * 
	 * @return 行数。
	 */
	int rowSize()
	{
		return table.size();
	}
}
