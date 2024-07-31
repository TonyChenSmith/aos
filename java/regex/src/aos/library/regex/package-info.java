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
 * 
 * 后续功能皆从这三种运算推导出来。于是有以下闭包变种。
 * <ol>
 * <li><code>+</code> - 一到多次。</li>
 * <li><code>?</code> - 零到一次。</li>
 * <li><code>{n}</code> - 重复n次。</li>
 * <li><code>{n,}</code> - 重复n到多次。</li>
 * <li><code>{n,m}</code> - 重复n到m次。</li>
 * </ol>
 * 
 * 基础功能还有分组，由“<code>(</code>”和“<code>)</code>”组成的一组将会被量词进行整体运算。其间不能为空。
 * 
 * <p>在分组内有一特殊运算符“<code>~</code>”，为前置一元运算符，其后须有一个字符及等价一个字符作为谓词。含义为直到遇到后面字符为止都匹配。
 * 
 * <h2>二、正则字符常量</h2>
 * 
 * 正则库支持以下字符常量。
 * <ol>
 * <li><code>x</code> - 字符x。</li>
 * <li><code>\b</code> - 字符b。可转义一切未预定转义功能的字符，“b”为字符位置。</li>
 * <li><code>&#92;uXXXX</code> - 码元X对应字符。X为4位16进制数字，支持大小写混合。</li>
 * <li><code>\x{X}</code> - 码元X对应字符。X为变长16进制数字，支持大小写混合。</li>
 * <li><code>\N{name}</code> - 字符名name对应字符。name为不定长字符串，内容应为Unicode字符名。</li>
 * <li><code>\cX</code> - X的低五位对应控制字符。推荐使用“<code>[@-_]</code>”这一区间内字符。</li>
 * <li><code>\a</code> - 警铃符（U+0007）。</li>
 * <li><code>\t</code> - 制表符（U+0009）。</li>
 * <li><code>\n</code> - 换行符（U+000A）。</li>
 * <li><code>\f</code> - 换页符（U+000C）。</li>
 * <li><code>\r</code> - 回车符（U+000D）。</li>
 * <li><code>\e</code> - 退出符（U+001B）。</li>
 * </ol>
 * 
 * 其中上面及后面以“<code>\</code>”开头的字符串统称为转义序列。
 * 
 * <h2>三、正则字符类</h2>
 * 
 * 字符类用于指定一个字符范围替代单个字符。正则库支持以下字符类。
 * <ol>
 * <li><code>[abc]</code> - 字符a、b或c（简单）。</li>
 * <li><code>[^abc]</code> - 除了字符a、b或c以外的字符（反向）。</li>
 * <li><code>[a-z]</code> - 字符a到z，包含边界字符（范围）。</li>
 * <li><code>[a-zA-Z]</code> - 字符a到z，或字符A到Z（并集）。</li>
 * <li><code>[a-z\u0026a-c]</code> - 字符a到z与字符a到c的交集（交集）。等价于“<code>[a-c]</code>”。</li>
 * <li><code>[a-c[d-f]\u0026c-e]</code> - 字符a到c和字符d-f的并集与字符c到e的交集，其中字符d-f为一嵌套字符类。等价于“<code>[c-e]</code>”。</li>
 * </ol>
 * 
 * 字符类的主要语法已在上面举例。在字符类中，字符范围可以使用第二节所述的任何字符常量形式和后面所描述的预定义字符类。
 * 
 * <p>在字符类中，范围符“<code>-</code>”前后须有单个字符，且两字符大小关系不能颠倒，应小字符在前，大字符在后，除了字符颠倒情况以外范围符将变成一般字符；反向符“<code>^</code>”须在字符类开头才为反向符，否则为一般字符；交集符“<code>&</code>”在字符类开头（除掉反向符后）或在交集符后为一般字符，其余情况前后须有字符、范围或字符类通过无限个或运算连接的总字符类。
 * 
 * <p>集合运算优先级：先并集，后交集，最后确定是否取反。内部的嵌套字符类会提高嵌套字符类内部集合运算的优先级。
 * 
 * <h2>四、正则预定义字符类</h2>
 * 
 * 正则库预定义了以下字符类。
 * <ol>
 * <li><code>.</code> -
 * 通配符，除了换行符。等价于“<code>[\x{0}-\x{10FFFF}\u0026[^\n]]</code>”。</li>
 * <li><code>\d</code> - 一个数字。等价于“<code>\U{IsDigit}</code>”。</li>
 * <li><code>\D</code> - 一个非数字。等价于“<code>[^\U{IsDigit}]</code>”。</li>
 * <li><code>\h</code> - 一个水平空白符。等价于“<code>[ \t\x{A0}\x{1680}\x{180E}\x{2000}-\x{200A}\x{202F}\x{205F}\x{3000}]</code>”。</li>
 * <li><code>\H</code> - 一个非水平空白符。等价于“<code>[^\h]</code>”。</li>
 * <li><code>\v</code> - 一个竖直空白符。等价于“<code>[\n\x{0B}\f\r\x{85}\x{2028}\x{2029}]</code>”。</li>
 * <li><code>\V</code> - 一个非竖直空白符。等价于“<code>[^\v]</code>”。</li>
 * <li><code>\s</code> - 一个空白符。等价于“<code>\U{IsWhite_Space}</code>”。</li>
 * <li><code>\S</code> - 一个非空白符。等价于“<code>[^\U{IsWhite_Space}]</code>”。</li>
 * <li><code>\w</code> - 一个词汇字符。等价于“<code>[\U{Alpha}\U{gc=Mn}\U{gc=Me}\U{gc=Mc}\U{Digit}\U{gc=Pc}\U{IsJoin_Control}]</code>”。</li>
 * <li><code>\W</code> - 一个非词汇字符。等价于“<code>[^\w]</code>”。</li>
 * </ol>
 * 
 * 其中使用了部分后面定义的字符类，将在相关章节进行解释。
 * 
 * <h2>五、POSIX预定义字符类</h2>
 * 
 * 正则库支持以下POSIX预定义的字符类，代码区域仅限于US-ASCII（<code>[\x{0}-\x{7F}]</code>）。
 * <ol>
 * <li><code>\p{Lower}</code> - 一个小写字母。等价于“<code>[a-z]</code>”。</li>
 * <li><code>\p{Upper}</code> - 一个大写字母。等价于“<code>[A-Z]</code>”。</li>
 * <li><code>\p{ASCII}</code> - 一个ASCII码。等价于“<code>[\x{0}-\x{7F}]</code>”。</li>
 * <li><code>\p{Alpha}</code> - 一个字母。等价于“<code>[\p{Lower}\p{Upper}]</code>”。</li>
 * <li><code>\p{Digit}</code> - 一个十进制数字。等价于“<code>[0-9]</code>”。</li>
 * <li><code>\p{Alnum}</code> - 一个字母或数字。等价于“<code>[\p{Alpha}\p{Digit}]</code>”。</li>
 * <li><code>\p{Punct}</code> - 一个标点。等价于“<code>[!"#$%&'()*+,\-\./:;<=>?@\[\\\]^_`{|}~]|[\x{20}-\x{2F}\x{3A}-\x{40}\x{5B}-\x{60}\x{7B}-\x{7E}]</code>”。</li>
 * <li><code>\p{Graph}</code> - 一个可见字符。等价于“<code>[\p{Alnum}\p{Punct}]</code>”。</li>
 * <li><code>\p{Print}</code> - 一个可打印字符。等价于“<code>[ \p{Graph}]</code>”。</li>
 * <li><code>\p{Blank}</code> - 一个空白。等价于“<code>[ \t]</code>”。</li>
 * <li><code>\p{Cntrl}</code> - 一个控制字符。等价于“<code>[\x{0}-\x{1F}\x{7F}]</code>”。</li>
 * <li><code>\p{XDigit}</code> - 一个十六进制数字。等价于“<code>[0-9a-fA-F]</code>”。</li>
 * <li><code>\p{Space}</code> - 一个空白字符。等价于“<code>\p{s}</code>”。</li>
 * <li><code>\p{d}</code> - 一个数字。对标“<code>\d</code>”。等价于“<code>[0-9]</code>”。</li>
 * <li><code>\p{D}</code> - 一个非数字。对标“<code>\D</code>”。等价于“<code>[^0-9]</code>”。</li>
 * <li><code>\p{s}</code> - 一个空白符。对标“<code>\s</code>”。等价于“<code>[ \t\n\x{B}\f\r]</code>”。</li>
 * <li><code>\p{S}</code> - 一个非空白符。对标“<code>\S</code>”。等价于“<code>[^\s]</code>”。</li>
 * <li><code>\p{w}</code> - 一个词汇字符。对标“<code>\w</code>”。等价于“<code>[a-zA-Z_0-9]</code>”。</li>
 * <li><code>\p{W}</code> - 一个非词汇字符。对标“<code>\W</code>”。等价于“<code>[^\w]</code>”。</li>
 * </ol>
 * 
 * 其中包含被覆盖作用域的一部分预定义字符类。
 * 
 * <h2>六、Unicode预定义字符类</h2>
 * 
 * 正则库支持以下Unicode预定义的字符类。其可分为五个部分。
 * 
 * <h3>（一）Unicode各种属性引用</h3>
 * 
 * 以下举例表示如何引用Unicode的类别（category)、书写系统（script）和代码区段（block）。具体名称参照运行的Java虚拟机中{@link java.lang.Character 字符类}支持的Unicode标准。特别注意，类别支持单字符大类别和双字符小类别。
 * <ol>
 * <li><code>\U{Lu}</code> - 一个大写字母（直接引用类别）。</li>
 * <li><code>\U{IsLl}</code> - 一个小写字母（“<code>Is</code>”前缀引用类别）。</li>
 * <li><code>\U{general_category=L}</code> - 一个字母（“<code>general_category=</code>”前缀引用类别）。</li>
 * <li><code>\U{gc=P}</code> - 一个标点（“<code>gc=</code>”前缀引用类别）。</li>
 * <li><code>\U{IsHan}</code> - 一个中文（“<code>Is</code>”前缀引用书写系统）。</li>
 * <li><code>\U{script=Han}</code> - 一个中文（“<code>script=</code>”前缀引用书写系统）。</li>
 * <li><code>\U{sc=Han}</code> - 一个中文（“<code>sc=</code>”前缀引用书写系统）。</li>
 * <li><code>\U{InMongolian}</code> - 一个蒙古文（“<code>In</code>”前缀引用代码区段）。</li>
 * <li><code>\U{block=Mongolian}</code> - 一个蒙古文（“<code>block=</code>”前缀引用代码区段）。</li>
 * <li><code>\U{blk=Mongolian}</code> - 一个蒙古文（“<code>blk=</code>”前缀引用代码区段）。</li>
 * </ol>
 * 
 * <h3>（二）Unicode预定义字符类</h3>
 * 
 * 以下是特别定义的字符类。均以“<code>Is</code>”开头。
 * <ol>
 * <li><code>\U{IsAlphabetic}</code> - 一个字母表中的字母。</li>
 * <li><code>\U{IsIdeographic}</code> - 一个表意字符。</li>
 * <li><code>\U{IsLetter}</code> - 一个字母。</li>
 * <li><code>\U{IsLowercase}</code> - 一个小写字母。</li>
 * <li><code>\U{IsUppercase}</code> - 一个大写字母。</li>
 * <li><code>\U{IsTitlecase}</code> - 一个标题字母。</li>
 * <li><code>\U{IsPunctuation}</code> - 一个标点。</li>
 * <li><code>\U{IsControl}</code> - 一个控制字符。</li>
 * <li><code>\U{IsWhite_Space}</code> - 一个空白。</li>
 * <li><code>\U{IsWhiteSpace}</code> - 一个空白。</li>
 * <li><code>\U{IsDigit}</code> - 一个数字。</li>
 * <li><code>\U{IsHex_Digit}</code> - 一个十六进制数字。</li>
 * <li><code>\U{IsHexDigit}</code> - 一个十六进制数字。</li>
 * <li><code>\U{IsJoin_Control}</code> - 一个连接控制字符。</li>
 * <li><code>\U{IsJoinControl}</code> - 一个连接控制字符。</li>
 * <li><code>\U{IsNoncharacter_Code_Point}</code> - 一个非字符码元。</li>
 * <li><code>\U{IsNoncharacterCodePoint}</code> - 一个非字符码元。</li>
 * <li><code>\U{IsAssigned}</code> - 一个已分配字符。</li>
 * </ol>
 * 
 * 其等价范围自行浏览{@link aos.library.regex.CharPredicate 字符谓词类}代码。
 * 
 * <h3>（三）兼容POSIX预定义字符类</h3>
 * <ol>
 * <li><code>\U{Lower}</code> - 一个小写字母。等价于“<code>\U{IsLowercase}</code>”。</li>
 * <li><code>\U{Upper}</code> - 一个大写字母。等价于“<code>\U{IsUppercase}</code>”。</li>
 * <li><code>\U{ASCII}</code> - 一个ASCII码。等价于“<code>[\x{0}-\x{7F}]</code>”。</li>
 * <li><code>\U{Alpha}</code> - 一个字母。等价于“<code>\U{IsAlphabetic}</code>”。</li>
 * <li><code>\U{Digit}</code> - 一个十进制数字。等价于“<code>\U{IsDigit}</code>”。</li>
 * <li><code>\U{Alnum}</code> - 一个字母或数字。等价于“<code>[\U{IsAlphabetic}\U{IsDigit}]</code>”。</li>
 * <li><code>\U{Punct}</code> - 一个标点。等价于“<code>\U{IsPunctuation}</code>”。</li>
 * <li><code>\U{Graph}</code> - 一个可见字符。等价于“<code>[^\U{IsWhite_Space}\U{gc=Cc}\U{gc=Cs}\U{gc=Cn}]</code>”。</li>
 * <li><code>\U{Print}</code> - 一个可打印字符。等价于“<code>[\U{Graph}\U{Blank}&&[^\U{Cntrl}]]</code>”。</li>
 * <li><code>\U{Blank}</code> - 一个空白。等价于“<code>[\U{IsWhite_Space}&&[^\U{gc=Zl}\U{gc=Zp}\x{A}-\x{D}\x{85}]]</code>”。</li>
 * <li><code>\U{Cntrl}</code> - 一个控制字符。等价于“<code>\U{gc=Cc}</code>”。</li>
 * <li><code>\U{XDigit}</code> - 一个十六进制数字。等价于“<code>[[\U{gc=Nd}\U{IsHex_Digit}]]</code>”。</li>
 * <li><code>\U{Space}</code> - 一个空白字符。等价于“<code>\U{IsWhite_Space}</code>”。</li>
 * </ol>
 * 
 * <p>以上为该正则库支持的所有语法。
 * 
 * <p>在2024-06-16时生成。
 * 
 * @author Tony Chen Smith
 */
package aos.library.regex;