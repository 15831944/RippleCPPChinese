
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
    此文件是Beast的一部分：https://github.com/vinniefalco/beast
    版权所有2014，vinnie falco<vinnie.falco@gmail.com>

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

#ifndef BEAST_HASH_XXHASHER_H_INCLUDED
#define BEAST_HASH_XXHASHER_H_INCLUDED

#include <ripple/beast/hash/endian.h>
#include <ripple/beast/hash/impl/xxhash.h>
#include <type_traits>
#include <cstddef>

namespace beast {

class xxhasher
{
private:
//需要64位标准：：大小
    static_assert(sizeof(std::size_t)==8, "");

    detail::XXH64_state_t state_;

public:
    using result_type = std::size_t;

    static beast::endian const endian = beast::endian::native;

    xxhasher() noexcept
    {
        detail::XXH64_reset (&state_, 1);
    }

    template <class Seed,
        std::enable_if_t<
            std::is_unsigned<Seed>::value>* = nullptr>
    explicit
    xxhasher (Seed seed)
    {
        detail::XXH64_reset (&state_, seed);
    }

    template <class Seed,
        std::enable_if_t<
            std::is_unsigned<Seed>::value>* = nullptr>
    xxhasher (Seed seed, Seed)
    {
        detail::XXH64_reset (&state_, seed);
    }

    void
    operator()(void const* key, std::size_t len) noexcept
    {
        detail::XXH64_update (&state_, key, len);
    }

    explicit
    operator std::size_t() noexcept
    {
        return detail::XXH64_digest(&state_);
    }
};

} //野兽

#endif
