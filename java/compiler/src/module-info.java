/**
 * <b>AOS Java项目</b>
 * 
 * <p><b>aos.tools.compiler</b>
 * 
 * <p>自定义编译器。用于构造编程语言的前端。目前设计将语言前端编译为LLVM IR，后续交给LLVM进行优化。
 * 
 * <p>在2024-08-08时生成。
 * 
 * @author Tony Chen Smith
 * 
 * @version 0.0.1
 */
module aos.tools.compiler
{
	requires aos.library.regex;
	
	exports aos.tools.compiler;
}