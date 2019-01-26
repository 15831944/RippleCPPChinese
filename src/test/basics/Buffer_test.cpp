
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
//————————————————————————————————————————————————————————————————————————————————————————————————————————————————
/*
    此文件是Rippled的一部分：https://github0.com/ripple/rippled
    版权所有（c）2012-2016 Ripple Labs Inc.

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

#include <ripple/basics/Buffer.h>
#include <ripple/beast/unit_test.h>
#include <cstdint>
#include <type_traits>

namespace ripple {
namespace test {

struct Buffer_test : beast::unit_test::suite
{
    bool sane (Buffer const& b) const
    {
        if (b.size() == 0)
            return b.data() == nullptr;

        return b.data() != nullptr;
    }

    void run() override
    {
        std::uint8_t const data[] =
        {
            0xa8, 0xa1, 0x38, 0x45, 0x23, 0xec, 0xe4, 0x23,
            0x71, 0x6d, 0x2a, 0x18, 0xb4, 0x70, 0xcb, 0xf5,
            0xac, 0x2d, 0x89, 0x4d, 0x19, 0x9c, 0xf0, 0x2c,
            0x15, 0xd1, 0xf9, 0x9b, 0x66, 0xd2, 0x30, 0xd3
        };

        Buffer b0;
        BEAST_EXPECT (sane (b0));
        BEAST_EXPECT (b0.empty());

        Buffer b1 { 0 };
        BEAST_EXPECT (sane (b1));
        BEAST_EXPECT (b1.empty());
        std::memcpy(b1.alloc(16), data, 16);
        BEAST_EXPECT (sane (b1));
        BEAST_EXPECT (!b1.empty());
        BEAST_EXPECT (b1.size() == 16);

        Buffer b2 { b1.size() };
        BEAST_EXPECT (sane (b2));
        BEAST_EXPECT (!b2.empty());
        BEAST_EXPECT (b2.size() == b1.size());
        std::memcpy(b2.data(), data + 16, 16);

        Buffer b3 { data, sizeof(data) };
        BEAST_EXPECT (sane (b3));
        BEAST_EXPECT (!b3.empty());
        BEAST_EXPECT (b3.size() == sizeof(data));
        BEAST_EXPECT (std::memcmp (b3.data(), data, b3.size()) == 0);

//检查相等和不相等比较
        BEAST_EXPECT (b0 == b0);
        BEAST_EXPECT (b0 != b1);
        BEAST_EXPECT (b1 == b1);
        BEAST_EXPECT (b1 != b2);
        BEAST_EXPECT (b2 != b3);

//检查复制构造函数和复制分配：
        {
            testcase ("Copy Construction / Assignment");

            Buffer x{ b0 };
            BEAST_EXPECT (x == b0);
            BEAST_EXPECT (sane (x));
            Buffer y{ b1 };
            BEAST_EXPECT (y == b1);
            BEAST_EXPECT (sane (y));
            x = b2;
            BEAST_EXPECT (x == b2);
            BEAST_EXPECT (sane (x));
            x = y;
            BEAST_EXPECT (x == y);
            BEAST_EXPECT (sane (x));
            y = b3;
            BEAST_EXPECT (y == b3);
            BEAST_EXPECT (sane (y));
            x = b0;
            BEAST_EXPECT (x == b0);
            BEAST_EXPECT (sane (x));
#if defined(__clang__) && !defined(__APPLE__) && (__clang_major__ >= 7)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

            x = x;
            BEAST_EXPECT (x == b0);
            BEAST_EXPECT (sane (x));
            y = y;
            BEAST_EXPECT (y == b3);
            BEAST_EXPECT (sane (y));
#if defined(__clang__) && !defined(__APPLE__) && (__clang_major__ >= 7)
#pragma clang diagnostic pop
#endif
        }

//检查移动构造函数和移动分配：
        {
            testcase ("Move Construction / Assignment");
 
            static_assert(std::is_nothrow_move_constructible<Buffer>::value, "");
            static_assert(std::is_nothrow_move_assignable<Buffer>::value, "");

{ //从空buf移动构造
                Buffer x;
                Buffer y { std::move(x) };
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.empty());
                BEAST_EXPECT (sane(y));
                BEAST_EXPECT (y.empty());
                BEAST_EXPECT (x == y);
            }

{ //从非空buf移动构造
                Buffer x { b1 };
                Buffer y { std::move(x) };
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.empty());
                BEAST_EXPECT (sane(y));
                BEAST_EXPECT (y == b1);
            }

{ //将assign empty buf移到empty buf
                Buffer x;
                Buffer y;

                x = std::move(y);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.empty());
                BEAST_EXPECT (sane(y));
                BEAST_EXPECT (y.empty());
            }

{ //将分配非空buf移动到空buf
                Buffer x;
                Buffer y { b1 };

                x = std::move(y);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x == b1);
                BEAST_EXPECT (sane(y));
                BEAST_EXPECT (y.empty());
            }

{ //将分配空buf移动到非空buf
                Buffer x { b1 };
                Buffer y;

                x = std::move(y);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.empty());
                BEAST_EXPECT (sane(y));
                BEAST_EXPECT (y.empty());
            }

{ //将分配非空buf移动到非空buf
                Buffer x { b1 };
                Buffer y { b2 };
                Buffer z { b3 };

                x = std::move(y);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (!x.empty());
                BEAST_EXPECT (sane(y));
                BEAST_EXPECT (y.empty());

                x = std::move(z);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (!x.empty());
                BEAST_EXPECT (sane(z));
                BEAST_EXPECT (z.empty());
            }
        }

        {
            testcase ("Slice Conversion / Construction / Assignment");

            Buffer w { static_cast<Slice>(b0) };
            BEAST_EXPECT(sane(w));
            BEAST_EXPECT(w == b0);

            Buffer x { static_cast<Slice>(b1) };
            BEAST_EXPECT(sane(x));
            BEAST_EXPECT(x == b1);

            Buffer y { static_cast<Slice>(b2) };
            BEAST_EXPECT(sane(y));
            BEAST_EXPECT(y == b2);

            Buffer z { static_cast<Slice>(b3) };
            BEAST_EXPECT(sane(z));
            BEAST_EXPECT(z == b3);

//将空切片分配给空缓冲区
            w = static_cast<Slice>(b0);
            BEAST_EXPECT(sane(w));
            BEAST_EXPECT(w == b0);

//将非空切片分配给空缓冲区
            w = static_cast<Slice>(b1);
            BEAST_EXPECT(sane(w));
            BEAST_EXPECT(w == b1);

//将非空切片分配给非空缓冲区
            x = static_cast<Slice>(b2);
            BEAST_EXPECT(sane(x));
            BEAST_EXPECT(x == b2);

//将非空切片分配给非空缓冲区
            y = static_cast<Slice>(z);
            BEAST_EXPECT(sane(y));
            BEAST_EXPECT(y == z);

//将空切片分配给非空缓冲区：
            z = static_cast<Slice>(b0);
            BEAST_EXPECT(sane(z));
            BEAST_EXPECT (z == b0);
        }

        {
            testcase ("Allocation, Deallocation and Clearing");

            auto test = [this](Buffer const& b, std::size_t i)
            {
                Buffer x{b};

//尝试分配一些字节数，可能
//零（意思是清晰）和健全性检查
                x(i);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.size() == i);
                BEAST_EXPECT ((x.data() == nullptr) == (i == 0));

//尝试分配更多的数据（总是非零）
                x(i + 1);
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.size() == i + 1);
                BEAST_EXPECT (x.data() != nullptr);

//试图澄清：
                x.clear();
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.size() == 0);
                BEAST_EXPECT (x.data() == nullptr);

//尝试再次清除：
                x.clear();
                BEAST_EXPECT (sane(x));
                BEAST_EXPECT (x.size() == 0);
                BEAST_EXPECT (x.data() == nullptr);
            };

            for (std::size_t i = 0; i < 16; ++i)
            {
                test (b0, i);
                test (b1, i);
            }
        }
    }
};

BEAST_DEFINE_TESTSUITE(Buffer, ripple_basics, ripple);

}  //命名空间测试
}  //命名空间波纹
