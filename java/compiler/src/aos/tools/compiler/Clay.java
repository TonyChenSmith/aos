/*
 * Copyright (c) 2024-2025 Tony Chen Smith. All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
package aos.tools.compiler;

import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import aos.library.regex.Pattern;

/**
 * 确定代号“clay”为该编译器框架。
 * 
 * <p>该类为主类。用于调用编译器构造工具。
 * 
 * <p>在2024-08-22时生成。
 *
 * @author Tony Chen Smith
 */
public final class Clay
{
	/**
	 * 制胚过程中的上下文。
	 */
	private static final Context FORM_CONTEXT=null;
	
	/**
	 * 烧窑。将“*.body”陶胚代码编译为“.pottery”陶器文件。
	 * 
	 * @param arg 参数。
	 */
	private static void fire(Argument args)
	{
		
	}
	
	/**
	 * 制胚。将“*.clay”泥料代码编译为“.body”陶胚代码。
	 * 
	 * @param arg 参数。
	 */
	private static void form(Argument args)
	{
		
	}
	
	/**
	 * 主程序入口。
	 * 
	 * @param args 参数数组。
	 */
	public static void main(String[] args)
	{
		Argument arg=new Argument(args);
		switch(arg.get("operation"))
		{
			case "form":
				form(arg);
				break;
			case "fire":
				fire(arg);
				break;
		}
	}
	
	/**
	 * 初始化语言。
	 */
	static
	{
		//第一个语言，clay语言。
		Map<String,Pattern> patterns=new HashMap<>();
		Set<String> banTokens=new HashSet<>();
		Map<String,LexerCallback> callbacks=new HashMap<>();
		Map<Pattern,String> tokenMap=new HashMap<>();
		patterns.put("$whitespace",Pattern.compile("[ \t]"));
		patterns.put("$linefeed",Pattern.compile("\n|\r\n"));
		patterns.put("$comment",Pattern.compile("%comment~%end"));
		patterns.put("$token",Pattern.compile("%token"));
		patterns.put("$production",Pattern.compile("%prod|%production"));
		patterns.put("$end",Pattern.compile("%end"));
		patterns.put("$id",Pattern.compile("[_\\p{Alpha}]\\p{w}*"));
		patterns.put("$regex",Pattern.compile("\"[^ \t\n\r]+\""));
		patterns.put("$assign",Pattern.compile(":"));
		patterns.put("$closure",Pattern.compile("*"));
		patterns.put("$append",Pattern.compile("+"));
		patterns.put("$optional",Pattern.compile("?"));
		for(Map.Entry<String,Pattern> entry:patterns.entrySet())
		{
			tokenMap.put(entry.getValue(),entry.getKey());
		}
		Map<String,Integer> priorityMap=new HashMap<>();
		priorityMap.put("$assign",0);
		priorityMap.put("$closure",1);
		priorityMap.put("$append",2);
		priorityMap.put("$assign",3);
		priorityMap.put("$comment",4);
		priorityMap.put("$linefeed",5);
		priorityMap.put("$whitespace",6);
		priorityMap.put("$token",7);
		priorityMap.put("$production",8);
		priorityMap.put("$end",9);
		priorityMap.put("$id",10);
		priorityMap.put("$regex",11);
		Collections.addAll(banTokens,"$whitespace","$comment");
		callbacks.put("$linefeed",LexerCallback.NEW_LINE);
		callbacks.put("$regex",(t,c)->{
			String lexem=t.get("lexem");
			t.set("regex",lexem.substring(1,lexem.length()-1));
		});
		Map<String,Production> productions=new HashMap<>();
		productions.put("%start",Production.Builder.create("%S").build("%start"));
		productions.put("%token",Production.Builder.create("$id").concat(Production.Builder.create("$regex")).concat(Production.Builder.create("$linefeed")).build("%start"));
	}
	
	/**
	 * 输入参数分析。
	 * 
	 * <p>在2024-08-22时生成。
	 *
	 * @author Tony Chen Smith
	 */
	private static final class Argument
	{
		/**
		 * 参数映射。
		 */
		final Map<String,String> arguments;
		
		/**
		 * 初始化参数信息。
		 * 
		 * @param args 参数数组。
		 */
		Argument(String[] args)
		{
			arguments=new HashMap<>();
			for(int index=0;index<args.length;index++)
			{
				Path p;
				switch(args[index])
				{
					case "-o":
						if(index==args.length-1)
						{
							throw new IllegalArgumentException("缺乏输出文件名称。");
						}
						index=index+1;
						arguments.put("output",args[index]);
						break;
					case "-w":
					case "-workspace":
						if(index==args.length-1)
						{
							throw new IllegalArgumentException("缺乏工作目录。");
						}
						index=index+1;
						p=Paths.get(args[index]);
						if(!Files.isDirectory(p))
						{
							throw new IllegalArgumentException("路径不是一个目录。");
						}
						arguments.put("workspace",p.toAbsolutePath().normalize().toString());
						break;
					default:
						//检查是否为输入文件。
						p=Paths.get(arguments.getOrDefault("workspace","."),args[index]);
						if(!Files.exists(p))
						{
							throw new IllegalArgumentException("文件不存在。");
						}
						if(Files.isDirectory(p))
						{
							throw new IllegalArgumentException("文件是一个目录。");
						}
						else if(!Files.isReadable(p))
						{
							throw new IllegalArgumentException("文件不可读。");
						}
						else if(args[index].endsWith(".clay"))
						{
							arguments.put("operation","form");
							arguments.put("input",args[index]);
						}
						else if(args[index].endsWith(".body"))
						{
							arguments.put("operation","fire");
							arguments.put("input",args[index]);
						}
						else
						{
							throw new IllegalArgumentException("不认识的文件后缀。");
						}
						break;
				}
			}
			String name=arguments.get("input");
			if(name==null)
			{
				throw new IllegalArgumentException("不存在输入文件。");
			}
			putDefaultArgs();
		}
		
		/**
		 * 补足默认参数。
		 */
		void putDefaultArgs()
		{
			String name=arguments.get("input");
			name=name.substring(0,name.lastIndexOf('.'));
			arguments.putIfAbsent("workspace",Paths.get(".").toAbsolutePath().normalize().toString());
			switch(arguments.get("operation"))
			{
				case "form":
					arguments.putIfAbsent("output",name.concat(".body"));
					break;
				case "fire":
					arguments.putIfAbsent("output",name.concat(".pottery"));
					break;
			}
		}
		
		/**
		 * 获取参数。
		 * 
		 * @param key 键。
		 * @return 参数。
		 */
		String get(String key)
		{
			return arguments.get(key);
		}
	}
}
