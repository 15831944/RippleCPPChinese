
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
    此文件是Rippled的一部分：https://github.com/ripple/rippled
    版权所有（c）2012，2013 Ripple Labs Inc.

    使用、复制、修改和/或分发本软件的权限
    特此授予免费或不收费的目的，前提是
    版权声明和本许可声明出现在所有副本中。

    本软件按“原样”提供，作者不作任何保证。
    关于本软件，包括
    适销性和适用性。在任何情况下，作者都不对
    任何特殊、直接、间接或后果性损害或任何损害
    因使用、数据或利润损失而导致的任何情况，无论是在
    合同行为、疏忽或其他侵权行为
    或与本软件的使用或性能有关。
**/

//==============================================================

#include <ripple/basics/contract.h>
#include <ripple/json/json_reader.h>
#include <algorithm>
#include <string>
#include <cctype>

namespace Json
{
//类阅读器的实现
////

constexpr unsigned Reader::nest_limit;

static
std::string
codePointToUTF8 (unsigned int cp)
{
    std::string result;

//基于http://en.wikipedia.org/wiki/utf-8中的描述

    if (cp <= 0x7f)
    {
        result.resize (1);
        result[0] = static_cast<char> (cp);
    }
    else if (cp <= 0x7FF)
    {
        result.resize (2);
        result[1] = static_cast<char> (0x80 | (0x3f & cp));
        result[0] = static_cast<char> (0xC0 | (0x1f & (cp >> 6)));
    }
    else if (cp <= 0xFFFF)
    {
        result.resize (3);
        result[2] = static_cast<char> (0x80 | (0x3f & cp));
        result[1] = 0x80 | static_cast<char> ((0x3f & (cp >> 6)));
        result[0] = 0xE0 | static_cast<char> ((0xf & (cp >> 12)));
    }
    else if (cp <= 0x10FFFF)
    {
        result.resize (4);
        result[3] = static_cast<char> (0x80 | (0x3f & cp));
        result[2] = static_cast<char> (0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char> (0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char> (0xF0 | (0x7 & (cp >> 18)));
    }

    return result;
}


//类读者
//////////////////////////////////////////////

bool
Reader::parse ( std::string const& document,
                Value& root)
{
    document_ = document;
    const char* begin = document_.c_str ();
    const char* end = begin + document_.length ();
    return parse ( begin, end, root );
}


bool
Reader::parse ( std::istream& sin,
                Value& root)
{
//std:：istream_iterator<char>begin（sin）；
//std:：istream_iterator<char>end；
//如果parse（）是
//模板函数。

//因为std：：string是引用计数的，所以至少不会
//创建一个额外的副本。
    std::string doc;
    std::getline (sin, doc, (char)EOF);
    return parse ( doc, root );
}

bool
Reader::parse ( const char* beginDoc, const char* endDoc,
                Value& root)
{
    begin_ = beginDoc;
    end_ = endDoc;
    current_ = begin_;
    lastValueEnd_ = 0;
    lastValue_ = 0;
    errors_.clear ();

    while ( !nodes_.empty () )
        nodes_.pop ();

    nodes_.push ( &root );
    bool successful = readValue(0);
    Token token;
    skipCommentTokens ( token );

    if ( !root.isNull() && !root.isArray() && !root.isObject() )
    {
//将错误位置设置为文档开头，最好是在文档中找到的第一个标记
        token.type_ = tokenError;
        token.start_ = beginDoc;
        token.end_ = endDoc;
        addError ( "A valid JSON document must be either an array or an object value.",
                   token );
        return false;
    }

    return successful;
}

bool
Reader::readValue(unsigned depth)
{
    Token token;
    skipCommentTokens ( token );
    if (depth > nest_limit)
        return addError("Syntax error: maximum nesting depth exceeded", token);
    bool successful = true;

    switch ( token.type_ )
    {
    case tokenObjectBegin:
        successful = readObject(token, depth);
        break;

    case tokenArrayBegin:
        successful = readArray(token, depth);
        break;

    case tokenInteger:
        successful = decodeNumber ( token );
        break;

    case tokenDouble:
        successful = decodeDouble ( token );
        break;

    case tokenString:
        successful = decodeString ( token );
        break;

    case tokenTrue:
        currentValue () = true;
        break;

    case tokenFalse:
        currentValue () = false;
        break;

    case tokenNull:
        currentValue () = Value ();
        break;

    default:
        return addError ( "Syntax error: value, object or array expected.", token );
    }

    return successful;
}


void
Reader::skipCommentTokens ( Token& token )
{
    do
    {
        readToken ( token );
    }
    while ( token.type_ == tokenComment );
}


bool
Reader::expectToken ( TokenType type, Token& token, const char* message )
{
    readToken ( token );

    if ( token.type_ != type )
        return addError ( message, token );

    return true;
}


bool
Reader::readToken ( Token& token )
{
    skipSpaces ();
    token.start_ = current_;
    Char c = getNextChar ();
    bool ok = true;

    switch ( c )
    {
    case '{':
        token.type_ = tokenObjectBegin;
        break;

    case '}':
        token.type_ = tokenObjectEnd;
        break;

    case '[':
        token.type_ = tokenArrayBegin;
        break;

    case ']':
        token.type_ = tokenArrayEnd;
        break;

    case '"':
        token.type_ = tokenString;
        ok = readString ();
        break;

    case '/':
        token.type_ = tokenComment;
        ok = readComment ();
        break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
        token.type_ = readNumber ();
        break;

    case 't':
        token.type_ = tokenTrue;
        ok = match ( "rue", 3 );
        break;

    case 'f':
        token.type_ = tokenFalse;
        ok = match ( "alse", 4 );
        break;

    case 'n':
        token.type_ = tokenNull;
        ok = match ( "ull", 3 );
        break;

    case ',':
        token.type_ = tokenArraySeparator;
        break;

    case ':':
        token.type_ = tokenMemberSeparator;
        break;

    case 0:
        token.type_ = tokenEndOfStream;
        break;

    default:
        ok = false;
        break;
    }

    if ( !ok )
        token.type_ = tokenError;

    token.end_ = current_;
    return true;
}


void
Reader::skipSpaces ()
{
    while ( current_ != end_ )
    {
        Char c = *current_;

        if ( c == ' '  ||  c == '\t'  ||  c == '\r'  ||  c == '\n' )
            ++current_;
        else
            break;
    }
}


bool
Reader::match ( Location pattern,
                int patternLength )
{
    if ( end_ - current_ < patternLength )
        return false;

    int index = patternLength;

    while ( index-- )
        if ( current_[index] != pattern[index] )
            return false;

    current_ += patternLength;
    return true;
}


bool
Reader::readComment ()
{
    Char c = getNextChar ();

    if ( c == '*' )
        return readCStyleComment ();

    if ( c == '/' )
        return readCppStyleComment ();

    return false;
}

bool
Reader::readCStyleComment ()
{
    while ( current_ != end_ )
    {
        Char c = getNextChar ();

        if ( c == '*'  &&  *current_ == '/' )
            break;
    }

    return getNextChar () == '/';
}


bool
Reader::readCppStyleComment ()
{
    while ( current_ != end_ )
    {
        Char c = getNextChar ();

        if (  c == '\r'  ||  c == '\n' )
            break;
    }

    return true;
}

Reader::TokenType
Reader::readNumber ()
{
    static char const extended_tokens[] = { '.', 'e', 'E', '+', '-' };

    TokenType type = tokenInteger;

    if ( current_ != end_ )
    {
        if (*current_ == '-')
            ++current_;

        while ( current_ != end_ )
        {
            if (!std::isdigit (static_cast<unsigned char>(*current_)))
            {
                auto ret = std::find (std::begin (extended_tokens),
                    std::end (extended_tokens), *current_);

                if (ret == std::end (extended_tokens))
                    break;

                type = tokenDouble;
            }

            ++current_;
        }
    }

    return type;
}

bool
Reader::readString ()
{
    Char c = 0;

    while ( current_ != end_ )
    {
        c = getNextChar ();

        if ( c == '\\' )
            getNextChar ();
        else if ( c == '"' )
            break;
    }

    return c == '"';
}


bool
Reader::readObject(Token& tokenStart, unsigned depth)
{
    Token tokenName;
    std::string name;
    currentValue () = Value ( objectValue );

    while ( readToken ( tokenName ) )
    {
        bool initialTokenOk = true;

        while ( tokenName.type_ == tokenComment  &&  initialTokenOk )
            initialTokenOk = readToken ( tokenName );

        if  ( !initialTokenOk )
            break;

if ( tokenName.type_ == tokenObjectEnd  &&  name.empty () ) //空对象
            return true;

        if ( tokenName.type_ != tokenString )
            break;

        name = "";

        if ( !decodeString ( tokenName, name ) )
            return recoverFromError ( tokenObjectEnd );

        Token colon;

        if ( !readToken ( colon ) ||  colon.type_ != tokenMemberSeparator )
        {
            return addErrorAndRecover ( "Missing ':' after object member name",
                                        colon,
                                        tokenObjectEnd );
        }

//拒绝重复的名称
        if (currentValue ().isMember (name))
            return addError ( "Key '" + name + "' appears twice.", tokenName );

        Value& value = currentValue ()[ name ];
        nodes_.push ( &value );
        bool ok = readValue(depth+1);
        nodes_.pop ();

if ( !ok ) //已设置错误
            return recoverFromError ( tokenObjectEnd );

        Token comma;

        if ( !readToken ( comma )
                ||  ( comma.type_ != tokenObjectEnd  &&
                      comma.type_ != tokenArraySeparator &&
                      comma.type_ != tokenComment ) )
        {
            return addErrorAndRecover ( "Missing ',' or '}' in object declaration",
                                        comma,
                                        tokenObjectEnd );
        }

        bool finalizeTokenOk = true;

        while ( comma.type_ == tokenComment &&
                finalizeTokenOk )
            finalizeTokenOk = readToken ( comma );

        if ( comma.type_ == tokenObjectEnd )
            return true;
    }

    return addErrorAndRecover ( "Missing '}' or object member name",
                                tokenName,
                                tokenObjectEnd );
}


bool
Reader::readArray(Token& tokenStart, unsigned depth)
{
    currentValue () = Value ( arrayValue );
    skipSpaces ();

if ( *current_ == ']' ) //空数组
    {
        Token endArray;
        readToken ( endArray );
        return true;
    }

    int index = 0;

    while ( true )
    {
        Value& value = currentValue ()[ index++ ];
        nodes_.push ( &value );
        bool ok = readValue(depth+1);
        nodes_.pop ();

if ( !ok ) //已设置错误
            return recoverFromError ( tokenArrayEnd );

        Token token;
//接受数组中最后一项后的注释。
        ok = readToken ( token );

        while ( token.type_ == tokenComment  &&  ok )
        {
            ok = readToken ( token );
        }

        bool badTokenType = ( token.type_ != tokenArraySeparator &&
                              token.type_ != tokenArrayEnd );

        if ( !ok  ||  badTokenType )
        {
            return addErrorAndRecover ( "Missing ',' or ']' in array declaration",
                                        token,
                                        tokenArrayEnd );
        }

        if ( token.type_ == tokenArrayEnd )
            break;
    }

    return true;
}

bool
Reader::decodeNumber ( Token& token )
{
    Location current = token.start_;
    bool isNegative = *current == '-';

    if ( isNegative )
        ++current;

    if (current == token.end_)
    {
        return addError ( "'" + std::string ( token.start_, token.end_ ) +
            "' is not a valid number.", token );
    }

//现有的JSON整数是32位的，因此在这里使用64位值可以避免
//下面的转换代码溢出。
    std::int64_t value = 0;

    static_assert(sizeof(value) > sizeof(Value::maxUInt),
        "The JSON integer overflow logic will need to be reworked.");

    while (current < token.end_ && (value <= Value::maxUInt))
    {
        Char c = *current++;

        if ( c < '0'  ||  c > '9' )
        {
            return addError ( "'" + std::string ( token.start_, token.end_ ) +
                "' is not a number.", token );
        }

        value = (value * 10) + (c - '0');
    }

//还有更多的令牌->输入大于最大可能的返回值
    if (current != token.end_)
    {
        return addError ( "'" + std::string ( token.start_, token.end_ ) +
            "' exceeds the allowable range.", token );
    }

    if ( isNegative )
    {
        value = -value;

        if (value < Value::minInt || value > Value::maxInt)
        {
            return addError ( "'" + std::string ( token.start_, token.end_ ) +
                "' exceeds the allowable range.", token );
        }

        currentValue () = static_cast<Value::Int>( value );
    }
    else
    {
        if (value > Value::maxUInt)
        {
            return addError ( "'" + std::string ( token.start_, token.end_ ) +
                "' exceeds the allowable range.", token );
        }

//如果它可以表示为有符号整数，则将其构造为一个整数。
        if ( value <= Value::maxInt )
            currentValue () = static_cast<Value::Int>( value );
        else
            currentValue () = static_cast<Value::UInt>( value );
    }

    return true;
}

bool
Reader::decodeDouble( Token &token )
{
    double value = 0;
    const int bufferSize = 32;
    int count;
    int length = int(token.end_ - token.start_);
//健全性检查以避免缓冲区溢出漏洞。
    if (length < 0) {
        return addError( "Unable to parse token length", token );
    }
//避免对给定的格式控制字符串使用字符串常量
//SScanf，因为这会导致OS X上难以调试的崩溃。有关更多信息，请参阅此处。
//信息：
//
//http://developer.apple.com/library/mac/documentation/developertools/gcc-4.0.1/gcc/uncompatibilities.html
    char format[] = "%lf";
    if ( length <= bufferSize )
    {
        Char buffer[bufferSize+1];
        memcpy( buffer, token.start_, length );
        buffer[length] = 0;
        count = sscanf( buffer, format, &value );
    }
    else
    {
        std::string buffer( token.start_, token.end_ );
        count = sscanf( buffer.c_str(), format, &value );
    }
    if ( count != 1 )
        return addError( "'" + std::string( token.start_, token.end_ ) + "' is not a number.", token );
    currentValue() = value;
    return true;
}



bool
Reader::decodeString ( Token& token )
{
    std::string decoded;

    if ( !decodeString ( token, decoded ) )
        return false;

    currentValue () = decoded;
    return true;
}


bool
Reader::decodeString ( Token& token, std::string& decoded )
{
    decoded.reserve ( token.end_ - token.start_ - 2 );
Location current = token.start_ + 1; //跳过“
Location end = token.end_ - 1;      //不包括'“'

    while ( current != end )
    {
        Char c = *current++;

        if ( c == '"' )
            break;
        else if ( c == '\\' )
        {
            if ( current == end )
                return addError ( "Empty escape sequence in string", token, current );

            Char escape = *current++;

            switch ( escape )
            {
            case '"':
                decoded += '"';
                break;

            case '/':
                decoded += '/';
                break;

            case '\\':
                decoded += '\\';
                break;

            case 'b':
                decoded += '\b';
                break;

            case 'f':
                decoded += '\f';
                break;

            case 'n':
                decoded += '\n';
                break;

            case 'r':
                decoded += '\r';
                break;

            case 't':
                decoded += '\t';
                break;

            case 'u':
            {
                unsigned int unicode;

                if ( !decodeUnicodeCodePoint ( token, current, end, unicode ) )
                    return false;

                decoded += codePointToUTF8 (unicode);
            }
            break;

            default:
                return addError ( "Bad escape sequence in string", token, current );
            }
        }
        else
        {
            decoded += c;
        }
    }

    return true;
}

bool
Reader::decodeUnicodeCodePoint ( Token& token,
                                 Location& current,
                                 Location end,
                                 unsigned int& unicode )
{

    if ( !decodeUnicodeEscapeSequence ( token, current, end, unicode ) )
        return false;

    if (unicode >= 0xD800 && unicode <= 0xDBFF)
    {
//代理对
        if (end - current < 6)
            return addError ( "additional six characters expected to parse unicode surrogate pair.", token, current );

        unsigned int surrogatePair;

        if (* (current++) == '\\' && * (current++) == 'u')
        {
            if (decodeUnicodeEscapeSequence ( token, current, end, surrogatePair ))
            {
                unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
            }
            else
                return false;
        }
        else
            return addError ( "expecting another \\u token to begin the second half of a unicode surrogate pair", token, current );
    }

    return true;
}

bool
Reader::decodeUnicodeEscapeSequence ( Token& token,
                                      Location& current,
                                      Location end,
                                      unsigned int& unicode )
{
    if ( end - current < 4 )
        return addError ( "Bad unicode escape sequence in string: four digits expected.", token, current );

    unicode = 0;

    for ( int index = 0; index < 4; ++index )
    {
        Char c = *current++;
        unicode *= 16;

        if ( c >= '0'  &&  c <= '9' )
            unicode += c - '0';
        else if ( c >= 'a'  &&  c <= 'f' )
            unicode += c - 'a' + 10;
        else if ( c >= 'A'  &&  c <= 'F' )
            unicode += c - 'A' + 10;
        else
            return addError ( "Bad unicode escape sequence in string: hexadecimal digit expected.", token, current );
    }

    return true;
}


bool
Reader::addError ( std::string const& message,
                   Token& token,
                   Location extra )
{
    ErrorInfo info;
    info.token_ = token;
    info.message_ = message;
    info.extra_ = extra;
    errors_.push_back ( info );
    return false;
}


bool
Reader::recoverFromError ( TokenType skipUntilToken )
{
    int errorCount = int (errors_.size ());
    Token skip;

    while ( true )
    {
        if ( !readToken (skip) )
errors_.resize ( errorCount ); //放弃由恢复引起的错误

        if ( skip.type_ == skipUntilToken  ||  skip.type_ == tokenEndOfStream )
            break;
    }

    errors_.resize ( errorCount );
    return false;
}


bool
Reader::addErrorAndRecover ( std::string const& message,
                             Token& token,
                             TokenType skipUntilToken )
{
    addError ( message, token );
    return recoverFromError ( skipUntilToken );
}


Value&
Reader::currentValue ()
{
    return * (nodes_.top ());
}


Reader::Char
Reader::getNextChar ()
{
    if ( current_ == end_ )
        return 0;

    return *current_++;
}


void
Reader::getLocationLineAndColumn ( Location location,
                                   int& line,
                                   int& column ) const
{
    Location current = begin_;
    Location lastLineStart = current;
    line = 0;

    while ( current < location  &&  current != end_ )
    {
        Char c = *current++;

        if ( c == '\r' )
        {
            if ( *current == '\n' )
                ++current;

            lastLineStart = current;
            ++line;
        }
        else if ( c == '\n' )
        {
            lastLineStart = current;
            ++line;
        }
    }

//列和行从1开始
    column = int (location - lastLineStart) + 1;
    ++line;
}


std::string
Reader::getLocationLineAndColumn ( Location location ) const
{
    int line, column;
    getLocationLineAndColumn ( location, line, column );
    char buffer[18 + 16 + 16 + 1];
    sprintf ( buffer, "Line %d, Column %d", line, column );
    return buffer;
}


std::string
Reader::getFormatedErrorMessages () const
{
    std::string formattedMessage;

    for ( Errors::const_iterator itError = errors_.begin ();
            itError != errors_.end ();
            ++itError )
    {
        const ErrorInfo& error = *itError;
        formattedMessage += "* " + getLocationLineAndColumn ( error.token_.start_ ) + "\n";
        formattedMessage += "  " + error.message_ + "\n";

        if ( error.extra_ )
            formattedMessage += "See " + getLocationLineAndColumn ( error.extra_ ) + " for detail.\n";
    }

    return formattedMessage;
}


std::istream& operator>> ( std::istream& sin, Value& root )
{
    Json::Reader reader;
    bool ok = reader.parse (sin, root);

//JSON断言（OK）；
    if (! ok)
        ripple::Throw<std::runtime_error> (reader.getFormatedErrorMessages ());

    return sin;
}


} //命名空间JSON
