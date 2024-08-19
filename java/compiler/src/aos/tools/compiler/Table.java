package aos.tools.compiler;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.TreeSet;

/**
 * 表。由三个类型参数确定的二维数据结构。V代表列，H代表行，C代表内容。
 * 
 * <p>特别说明，该表不依靠行标进行排序。对行标不做硬性排序要求。
 * 
 * <p>在2024-08-14时生成。
 *
 * @author Tony Chen Smith
 * @param <V> 行标类型。
 * @param <H> 列标类型。
 * @param <C> 值类型。
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
	 * 行号集。
	 */
	private final Set<V> rows;
	
	/**
	 * 表载体。
	 */
	private final Map<TableKey<V,H>,C> table;
	
	/**
	 * 构造表。
	 */
	Table()
	{
		this(false,false);
	}
	
	/**
	 * 构造可选排序表。
	 * 
	 * @param isSortedV 竖直可排。
	 * @param isSortedH 水平可排。
	 */
	Table(boolean isSortedV,boolean isSortedH)
	{
		table=new HashMap<>();
		if(isSortedV)
		{
			rows=new TreeSet<>();
		}
		else
		{
			rows=new LinkedHashSet<>();
		}
		if(isSortedH)
		{
			columns=new TreeSet<>();
		}
		else
		{
			columns=new LinkedHashSet<>();
		}
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
		return table.getOrDefault(new TableKey<V,H>(row,column),defaultValue);
	}
	
	/**
	 * 获取列。调用该方法时应该假设在所有列中该值只存在其中一个列里。
	 * 
	 * @param value 值。
	 * 
	 * @return 对应遇到的第一个列标。
	 */
	H getColumn(C value)
	{
		for(Map.Entry<TableKey<V,H>,C> entry:table.entrySet())
		{
			if(Objects.equals(entry.getValue(),value))
			{
				return entry.getKey().column;
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
	 * @return 对应集合。不可改。
	 */
	Collection<C> getColumnValues(H column)
	{
		LinkedList<C> result=new LinkedList<>();
		for(Map.Entry<TableKey<V,H>,C> entry:table.entrySet())
		{
			if(Objects.equals(entry.getKey().column,column))
			{
				result.add(entry.getValue());
			}
		}
		return Collections.unmodifiableCollection(result);
	}
	
	/**
	 * 获取一列所有内容。
	 * 
	 * @param row 行。
	 * 
	 * @return 对应集合。不可改。
	 */
	Collection<C> getRowValues(V row)
	{
		LinkedList<C> result=new LinkedList<>();
		for(Map.Entry<TableKey<V,H>,C> entry:table.entrySet())
		{
			if(Objects.equals(entry.getKey().row,row))
			{
				result.add(entry.getValue());
			}
		}
		return Collections.unmodifiableCollection(result);
	}
	
	/**
	 * 获取行。调用该方法时应该假设在所有行中该值只存在其中一个行里。
	 * 
	 * @param value 值。
	 * 
	 * @return 对应行标。
	 */
	V getRow(C value)
	{
		for(Map.Entry<TableKey<V,H>,C> entry:table.entrySet())
		{
			if(Objects.equals(entry.getValue(),value))
			{
				return entry.getKey().row;
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
		return Collections.unmodifiableSet(rows);
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
		TableKey<V,H> key=new TableKey<>(row,column);
		table.put(key,value);
		rows.add(row);
		columns.add(column);
	}
	
	/**
	 * 获得当前行数。
	 * 
	 * @return 行数。
	 */
	int rowSize()
	{
		return rows.size();
	}
	
	/**
	 * 获取当前列数。
	 * 
	 * @return 列数。
	 */
	int columnSize()
	{
		return columns.size();
	}
	
	/**
	 * 将表内容转换成字符串行。
	 * 
	 * @return 行数组。
	 */
	String[] toLines()
	{
		List<String> srows=new ArrayList<>(rowSize());
		List<V> orows=new ArrayList<>(rowSize());
		rows.forEach(row->{srows.add(row.toString());orows.add(row);});
		List<String> scols=new ArrayList<>(columnSize());
		List<H> ocols=new ArrayList<>(columnSize());
		columns.forEach(col->{scols.add(col.toString());ocols.add(col);});
		int[] widths=new int[scols.size()+1];
		widths[0]=srows.parallelStream().mapToInt(String::length).max().getAsInt();
		for(int index=0;index<scols.size();index++)
		{
			widths[index+1]=Math.max(getColumnValues(ocols.get(index)).parallelStream().mapToInt(v->v==null?0:v.toString().length()).max().getAsInt(),scols.get(index).length());
		}
		List<String> list=new LinkedList<>();
		StringBuilder line=new StringBuilder();
		//行分析。
		for(int index=0;index<widths.length;index++)
		{
			if(index==0)
			{
				for(int count=0;count<widths[0];count++)
				{
					line.append(' ');
				}
			}
			else
			{
				int white=widths[index]-scols.get(index-1).length();
				int prefix=white/2;
				white=white-prefix;
				for(int count=0;count<prefix;count++)
				{
					line.append(' ');
				}
				line.append(scols.get(index-1));
				for(int count=0;count<white;count++)
				{
					line.append(' ');
				}
			}
			if(index!=widths.length-1)
			{
				//分隔符。
				line.append(' ');
			}
		}
		list.add(line.toString());
		for(int itr=0;itr<srows.size();itr++)
		{
			line.delete(0,line.length());
			//每行内容分析。
			for(int index=0;index<widths.length;index++)
			{
				if(index==0)
				{
					int white=widths[index]-srows.get(itr).length();
					int prefix=white/2;
					white=white-prefix;
					for(int count=0;count<prefix;count++)
					{
						line.append(' ');
					}
					line.append(srows.get(itr));
					for(int count=0;count<white;count++)
					{
						line.append(' ');
					}
				}
				else
				{
					String text=Objects.toString(get(orows.get(itr),ocols.get(index-1)),"-");
					int white=widths[index]-text.length();
					int prefix=white/2;
					white=white-prefix;
					for(int count=0;count<prefix;count++)
					{
						line.append(' ');
					}
					line.append(text);
					for(int count=0;count<white;count++)
					{
						line.append(' ');
					}
				}
				if(index!=widths.length-1)
				{
					//分隔符。
					line.append(' ');
				}
			}
			list.add(line.toString());
		}
		return list.toArray(String[]::new);
	}
	
	@Override
	public String toString()
	{
		StringBuilder result=new StringBuilder();
		String[] lines=toLines();
		for(int index=0;index<lines.length;index++)
		{
			result.append(lines[index]);
			if(index!=lines.length-1)
			{
				result.append('\n');
			}
		}
		return result.toString();
	}
	
	/**
	 * 表键。
	 * 
	 * <p>在2024-08-19时生成。
	 *
	 * @author Tony Chen Smith
	 * @param <K1> 行标类型。
	 * @param <K2> 列标类型。
	 */
	private record TableKey<K1,K2>(K1 row,K2 column) implements Serializable {}
}
