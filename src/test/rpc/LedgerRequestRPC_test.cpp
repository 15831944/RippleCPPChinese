
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

#include <ripple/protocol/ErrorCodes.h>
#include <ripple/protocol/JsonFields.h>
#include <test/jtx.h>
#include <ripple/beast/unit_test.h>
#include <ripple/app/ledger/LedgerMaster.h>

namespace ripple {

namespace RPC {

class LedgerRequestRPC_test : public beast::unit_test::suite
{
public:

    void testLedgerRequest()
    {
        using namespace test::jtx;

        Env env(*this);

        env.close();
        env.close();
        BEAST_EXPECT(env.current()->info().seq == 5);

        {
//任意文本转换为0。
            auto const result = env.rpc("ledger_request", "arbitrary_text");
            BEAST_EXPECT(RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::error_message] ==
                "Ledger index too small");
        }

        {
            auto const result = env.rpc("ledger_request", "-1");
            BEAST_EXPECT(RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::error_message] ==
                "Ledger index too small");
        }

        {
            auto const result = env.rpc("ledger_request", "0");
            BEAST_EXPECT(RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::error_message] ==
                    "Ledger index too small");
        }

        {
            auto const result = env.rpc("ledger_request", "1");
            BEAST_EXPECT(!RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::ledger_index] == 1 &&
                    result[jss::result].isMember(jss::ledger));
            BEAST_EXPECT(result[jss::result][jss::ledger].
                isMember(jss::ledger_hash) &&
                    result[jss::result][jss::ledger]
                        [jss::ledger_hash].isString());
        }

        {
            auto const result = env.rpc("ledger_request", "2");
            BEAST_EXPECT(!RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::ledger_index] == 2 &&
                    result[jss::result].isMember(jss::ledger));
            BEAST_EXPECT(result[jss::result][jss::ledger].
                isMember(jss::ledger_hash) &&
                    result[jss::result][jss::ledger]
                        [jss::ledger_hash].isString());
        }

        {
            auto const result = env.rpc("ledger_request", "3");
            BEAST_EXPECT(!RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::ledger_index] == 3 &&
                    result[jss::result].isMember(jss::ledger));
            BEAST_EXPECT(result[jss::result][jss::ledger].
                isMember(jss::ledger_hash) &&
                    result[jss::result][jss::ledger]
                        [jss::ledger_hash].isString());

            auto const ledgerHash = result[jss::result]
                [jss::ledger][jss::ledger_hash].asString();

            {
//故意在这里隐藏“result”以避免重用它。
                auto const result = env.rpc("ledger_request", ledgerHash);
                BEAST_EXPECT(!RPC::contains_error(result[jss::result]) &&
                    result[jss::result][jss::ledger_index] == 3 &&
                        result[jss::result].isMember(jss::ledger));
                BEAST_EXPECT(result[jss::result][jss::ledger].
                    isMember(jss::ledger_hash) &&
                        result[jss::result][jss::ledger]
                            [jss::ledger_hash] == ledgerHash);
            }
        }

        {
            std::string ledgerHash(64, 'q');

            auto const result = env.rpc("ledger_request", ledgerHash);

            BEAST_EXPECT(RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::error_message] ==
                    "Invalid field 'ledger_hash'.");
        }

        {
            std::string ledgerHash(64, '1');

            auto const result = env.rpc("ledger_request", ledgerHash);

            BEAST_EXPECT(!RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::have_header] == false);
        }

        {
            auto const result = env.rpc("ledger_request", "4");
            BEAST_EXPECT(RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::error_message] ==
                    "Ledger index too large");
        }

        {
            auto const result = env.rpc("ledger_request", "5");
            BEAST_EXPECT(RPC::contains_error(result[jss::result]) &&
                result[jss::result][jss::error_message] ==
                    "Ledger index too large");
        }

    }

    void testEvolution()
    {
        using namespace test::jtx;
Env env {*this, FeatureBitset{}}; //下面检查的哈希假设
//无修正案
        Account const gw { "gateway" };
        auto const USD = gw["USD"];
        env.fund(XRP(100000), gw);
        env.close();

        env.memoize("bob");
        env.fund(XRP(1000), "bob");
        env.close();

        env.memoize("alice");
        env.fund(XRP(1000), "alice");
        env.close();

        env.memoize("carol");
        env.fund(XRP(1000), "carol");
        env.close();

        auto result = env.rpc ( "ledger_request", "1" ) [jss::result];
        BEAST_EXPECT(result[jss::ledger][jss::ledger_index]     == "1");
        BEAST_EXPECT(result[jss::ledger][jss::total_coins]      == "100000000000000000");
        BEAST_EXPECT(result[jss::ledger][jss::closed]           == true);
        BEAST_EXPECT(result[jss::ledger][jss::ledger_hash]      == "AB868A6CFEEC779C2FF845C0AF00A642259986AF40C01976A7F842B6918936C7");
        BEAST_EXPECT(result[jss::ledger][jss::parent_hash]      == "0000000000000000000000000000000000000000000000000000000000000000");
        BEAST_EXPECT(result[jss::ledger][jss::account_hash]     == "A21ED30C04C88046FC61DB9DC19375EEDBD365FD8C17286F27127DF804E9CAA6");
        BEAST_EXPECT(result[jss::ledger][jss::transaction_hash] == "0000000000000000000000000000000000000000000000000000000000000000");

        result = env.rpc ( "ledger_request", "2" ) [jss::result];
        BEAST_EXPECT(result[jss::ledger][jss::ledger_index]     == "2");
        BEAST_EXPECT(result[jss::ledger][jss::total_coins]      == "100000000000000000");
        BEAST_EXPECT(result[jss::ledger][jss::closed]           == true);
        BEAST_EXPECT(result[jss::ledger][jss::ledger_hash]      == "8AEDBB96643962F1D40F01E25632ABB3C56C9F04B0231EE4B18248B90173D189");
        BEAST_EXPECT(result[jss::ledger][jss::parent_hash]      == "AB868A6CFEEC779C2FF845C0AF00A642259986AF40C01976A7F842B6918936C7");
        BEAST_EXPECT(result[jss::ledger][jss::account_hash]     == "183D5235C7C1FB5AE67AD2F6CC3B28F5FB86E8C4F89DB50DD85641A96470534E");
        BEAST_EXPECT(result[jss::ledger][jss::transaction_hash] == "0000000000000000000000000000000000000000000000000000000000000000");

        result = env.rpc ( "ledger_request", "3" ) [jss::result];
        BEAST_EXPECT(result[jss::ledger][jss::ledger_index]     == "3");
        BEAST_EXPECT(result[jss::ledger][jss::total_coins]      == "99999999999999980");
        BEAST_EXPECT(result[jss::ledger][jss::closed]           == true);
        BEAST_EXPECT(result[jss::ledger][jss::ledger_hash]      == "D2EE1E2A7288AAD43D6FA8AD8007FD1A95646F365EF3A1AD608A03258F11CF18");
        BEAST_EXPECT(result[jss::ledger][jss::parent_hash]      == "8AEDBB96643962F1D40F01E25632ABB3C56C9F04B0231EE4B18248B90173D189");
        BEAST_EXPECT(result[jss::ledger][jss::account_hash]     == "22565DC00D1A30F2C15871714E512976EF476281E5E87FF63D3E129C9069F4F4");
        BEAST_EXPECT(result[jss::ledger][jss::transaction_hash] == "0213EC486C058B3942FBE3DAC6839949A5C5B02B8B4244C8998EFDF04DBD8222");

        result = env.rpc ( "ledger_request", "4" ) [jss::result];
        BEAST_EXPECT(result[jss::ledger][jss::ledger_index]     == "4");
        BEAST_EXPECT(result[jss::ledger][jss::total_coins]      == "99999999999999960");
        BEAST_EXPECT(result[jss::ledger][jss::closed]           == true);
        BEAST_EXPECT(result[jss::ledger][jss::ledger_hash]      == "8F9032390CDD4C9D7A5B216AFDA3B525A3B39D7589C69D90D4C6BCA4619DD33C");
        BEAST_EXPECT(result[jss::ledger][jss::parent_hash]      == "D2EE1E2A7288AAD43D6FA8AD8007FD1A95646F365EF3A1AD608A03258F11CF18");
        BEAST_EXPECT(result[jss::ledger][jss::account_hash]     == "C3335CA14E712CB28F2A7C09BEB9A24BF30BBFA5528F156C19F6665D7A588FEA");
        BEAST_EXPECT(result[jss::ledger][jss::transaction_hash] == "3CBDB8F42E04333E1642166BFB93AC9A7E1C6C067092CD5D881D6F3AB3D67E76");

        result = env.rpc ( "ledger_request", "5" ) [jss::result];
        BEAST_EXPECT(result[jss::ledger][jss::ledger_index]     == "5");
        BEAST_EXPECT(result[jss::ledger][jss::total_coins]      == "99999999999999940");
        BEAST_EXPECT(result[jss::ledger][jss::closed]           == true);
        BEAST_EXPECT(result[jss::ledger][jss::ledger_hash]      == "3EDEB201735867A8EEECBC79A75902C05A7E3F192E4C12E02E67BFDDE5566CCE");
        BEAST_EXPECT(result[jss::ledger][jss::parent_hash]      == "8F9032390CDD4C9D7A5B216AFDA3B525A3B39D7589C69D90D4C6BCA4619DD33C");
        BEAST_EXPECT(result[jss::ledger][jss::account_hash]     == "7C77B1E9EB86410D84EE0CD50716AAA21192F19CF533194AD705798895248212");
        BEAST_EXPECT(result[jss::ledger][jss::transaction_hash] == "C3D086CD6BDB9E97AD1D513B2C049EF2840BD21D0B3E22D84EBBB89B6D2EF59D");

        result = env.rpc ( "ledger_request", "6" ) [jss::result];
        BEAST_EXPECT(result[jss::error]         == "invalidParams");
        BEAST_EXPECT(result[jss::status]        == "error");
        BEAST_EXPECT(result[jss::error_message] == "Ledger index too large");
    }

    void testBadInput()
    {
        using namespace test::jtx;
        Env env { *this };
        Account const gw { "gateway" };
        auto const USD = gw["USD"];
        env.fund(XRP(100000), gw);
        env.close();

        Json::Value jvParams;
        jvParams[jss::ledger_hash] = "AB868A6CFEEC779C2FF845C0AF00A642259986AF40C01976A7F842B6918936C7";
        jvParams[jss::ledger_index] = "1";
        auto result = env.rpc ("json", "ledger_request", jvParams.toStyledString()) [jss::result];
        BEAST_EXPECT(result[jss::error]         == "invalidParams");
        BEAST_EXPECT(result[jss::status]        == "error");
        BEAST_EXPECT(result[jss::error_message] == "Exactly one of ledger_hash and ledger_index can be set.");

//此测试的目的是强制分类帐过期/过期
//触发日期检查
        env.timeKeeper().adjustCloseTime(weeks{3});
        result = env.rpc ( "ledger_request", "1" ) [jss::result];
        BEAST_EXPECT(result[jss::error]         == "noCurrent");
        BEAST_EXPECT(result[jss::status]        == "error");
        BEAST_EXPECT(result[jss::error_message] == "Current ledger is unavailable.");

    }

    void testMoreThan256Closed()
    {
        using namespace test::jtx;
        using namespace std::chrono_literals;
        Env env {*this};
        Account const gw {"gateway"};
        env.app().getLedgerMaster().tune(0, 1h);
        auto const USD = gw["USD"];
        env.fund(XRP(100000), gw);

        int const max_limit = 256;

        for (auto i = 0; i < max_limit + 10; i++)
        {
            Account const bob {std::string("bob") + std::to_string(i)};
            env.fund(XRP(1000), bob);
            env.close();
        }

        auto result = env.rpc ( "ledger_request", "1" ) [jss::result];
        BEAST_EXPECT(result[jss::ledger][jss::ledger_index]     == "1");
        BEAST_EXPECT(result[jss::ledger][jss::total_coins]      == "100000000000000000");
        BEAST_EXPECT(result[jss::ledger][jss::closed]           == true);
        BEAST_EXPECT(result[jss::ledger][jss::ledger_hash]      == "AB868A6CFEEC779C2FF845C0AF00A642259986AF40C01976A7F842B6918936C7");
        BEAST_EXPECT(result[jss::ledger][jss::parent_hash]      == "0000000000000000000000000000000000000000000000000000000000000000");
        BEAST_EXPECT(result[jss::ledger][jss::account_hash]     == "A21ED30C04C88046FC61DB9DC19375EEDBD365FD8C17286F27127DF804E9CAA6");
        BEAST_EXPECT(result[jss::ledger][jss::transaction_hash] == "0000000000000000000000000000000000000000000000000000000000000000");
    }

    void testNonAdmin()
    {
        using namespace test::jtx;
        Env env { *this, envconfig(no_admin) };
        Account const gw { "gateway" };
        auto const USD = gw["USD"];
        env.fund(XRP(100000), gw);
        env.close();

        auto const result = env.rpc ( "ledger_request", "1" )  [jss::result];
//当前HTTP/S服务器处理程序在此处返回HTTP 403错误代码
//而不是nopermission json错误。jsonrpcclient就是吃这个
//错误并返回空结果。
        BEAST_EXPECT(result.type() == Json::nullValue);

    }

    void run () override
    {
        testLedgerRequest();
        testEvolution();
        testBadInput();
        testMoreThan256Closed();
        testNonAdmin();
    }
};

BEAST_DEFINE_TESTSUITE(LedgerRequestRPC,app,ripple);

} //RPC
} //涟漪

