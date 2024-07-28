/**
 * 正则库主包。
 * 
 * <p>非特殊情况，主要类基本在该包定义。
 * 
 * <p>该包负责提供一种<s>NFA引擎</s>DFA引擎的正则分析器。
 * 
 * <p>已经实现功能与介绍如下面篇幅所述。
 * 
 * <h2>一、基础运算</h2>
 * 
 * 正则库实现了三大基础运算。
 * <ol>
 * <li>连接（concat），形如“<code>ab</code>”。其为默认运算，没有显示运算符。</li>
 * <li>并联（union），形如“<code>a|b</code>”。其为二元运算符“<code>|</code>”。</li>
 * <li>闭包（closure），形如“<code>a*</code>”。其为后置位的一元运算符“<code>*</code>”。</li>
 * </ol>
 * 后续功能皆从这三种运算推导出来。
 * 
 * <h2>二、正则字符常量</h2>
 * 
 * 正则库支持以下字符常量。
 * <ol>
 * <li><code>x</code> - 字符x。</li>
 * <li><code>\b</code> - 字符b。可转义一切未预定转义功能的字符，“b”为字符位置。</li>
 * <li><code>\x{X}</code> - 码元X对应字符。X为变长16进制数字，支持大小写混合。</li>
 * <li><code>&#92;uXXXX</code> - 码元X对应字符。X为4位16进制数字，支持大小写混合。</li>
 * </ol>
 * 
 * <p>在2024-06-16时生成。
 * 
 * @author Tony Chen Smith
 */
package aos.library.regex;