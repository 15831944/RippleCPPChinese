
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
    版权所有（c）2012-2015 Ripple Labs Inc.

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
#include <ripple/core/Config.h>
#include <ripple/core/ConfigSections.h>
#include <ripple/server/Port.h>
#include <test/jtx/TestSuite.h>
#include <test/unit_test/FileDirGuard.h>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>

namespace ripple {
namespace detail {
std::string configContents (std::string const& dbPath,
    std::string const& validatorsFile)
{
    static boost::format configContentsTemplate (R"rippleConfig(
[server]
port_rpc
port_peer
port_wss_admin

[port_rpc]
port = 5005
ip = 127.0.0.1
admin = 127.0.0.1, ::1
protocol = https

[port_peer]
port = 51235
ip = 0.0.0.0
protocol = peer

[port_wss_admin]
port = 6006
ip = 127.0.0.1
admin = 127.0.0.1
protocol = wss

#[port_ws_public]
#port = 5005
#ip = 127.0.0.1
#protocol = wss

#-------------------------------------------------------------------------------

[node_size]
medium

# This is primary persistent datastore for rippled.  This includes transaction
# metadata, account states, and ledger headers.  Helpful information can be
# found here: https://Ripple.com/wiki/nodebackend公司
# delete old ledgers while maintaining at least 2000. Do not require an
# external administrative command to initiate deletion.
[node_db]
type=memory
path=/Users/dummy/ripple/config/db/rocksdb
open_files=2000
filter_bits=12
cache_mb=256
file_size_mb=8
file_size_mult=2

%1%

%2%

# This needs to be an absolute directory reference, not a relative one.
# Modify this value as required.
[debug_logfile]
/Users/dummy/ripple/config/log/debug.log

[sntp_servers]
time.windows.com
time.apple.com
time.nist.gov
pool.ntp.org

# Where to find some other servers speaking the Ripple protocol.
#
[ips]
r.ripple.com 51235

# Turn down default logging to save disk space in the long run.
# Valid values here are trace, debug, info, warning, error, and fatal
[rpc_startup]
{ "command": "log_level", "severity": "warning" }

# Defaults to 1 ("yes") so that certificates will be validated. To allow the use
# of self-signed certificates for development or internal use, set to 0 ("no").
[ssl_verify]
0

[sqdb]
backend=sqlite
)rippleConfig");

    std::string dbPathSection =
        dbPath.empty () ? "" : "[database_path]\n" + dbPath;
    std::string valFileSection =
        validatorsFile.empty () ? "" : "[validators_file]\n" + validatorsFile;
    return boost::str (
        configContentsTemplate % dbPathSection % valFileSection);
}

/*
   写一个波纹配置文件，完成后删除。
 **/

class RippledCfgGuard : public ripple::test::detail::FileDirGuard
{
private:
    path dataDir_;

    bool rmDataDir_{false};

    Config config_;

public:
    RippledCfgGuard (beast::unit_test::suite& test,
        path subDir, path const& dbPath,
            path const& validatorsFile,
                bool useCounter = true)
        : FileDirGuard(test, std::move (subDir),
            path (Config::configFileName),
            configContents (dbPath.string (), validatorsFile.string ()),
            useCounter)
        , dataDir_ (dbPath)
    {
        if (dbPath.empty ())
            dataDir_ = subdir() / path (Config::databaseDirName);

        rmDataDir_ = !exists (dataDir_);
        /*图“设置”（文件“.string（），/*bquiet*/true，
            /B-沉默*/ false, /* bStandalone */ false);

    }

    Config const& config () const
    {
        return config_;
    }

    std::string configFile() const
    {
        return file().string();
    }

    bool dataDirExists () const
    {
        return boost::filesystem::is_directory (dataDir_);
    }

    bool configFileExists () const
    {
        return fileExists();
    }

    ~RippledCfgGuard ()
    {
        try
        {
            using namespace boost::filesystem;
            if (rmDataDir_)
                rmDir (dataDir_);
            else
                test_.log << "Skipping rm dir: "
                          << dataDir_.string () << std::endl;
        }
        catch (std::exception& e)
        {
//如果我们扔到这里，就让它死吧。
            test_.log << "Error in ~RippledCfgGuard: "
                      << e.what () << std::endl;
        };
    }
};

std::string valFileContents ()
{
    std::string configContents (R"rippleConfig(
[validators]
n949f75evCHwgyP4fPVgaHqNHxUVN15PsJEZ3B3HnXPcPjcZAoy7
n9MD5h24qrQqiyBC8aeqqCWvpiBiYQ3jxSr91uiDvmrkyHRdYLUj
n9L81uNCaPgtUJfaHh89gmdvXKAmSt5Gdsw2g1iPWaPkAHW5Nm4C
n9KiYM9CgngLvtRCQHZwgC2gjpdaZcCcbt3VboxiNFcKuwFVujzS
n9LdgEtkmGB9E2h3K4Vp7iGUaKuq23Zr32ehxiU8FWY7xoxbWTSA

[validator_keys]
nHUhG1PgAG8H8myUENypM35JgfqXAKNQvRVVAFDRzJrny5eZN8d5
nHBu9PTL9dn2GuZtdW4U2WzBwffyX9qsQCd9CNU4Z5YG3PQfViM8
nHUPDdcdb2Y5DZAJne4c2iabFuAP3F34xZUgYQT2NH7qfkdapgnz

[validator_list_sites]
recommendedripplevalidators.com
moreripplevalidators.net

[validator_list_keys]
03E74EE14CB525AFBB9F1B7D86CD58ECC4B91452294B42AB4E78F260BD905C091D
030775A669685BD6ABCEBD80385921C7851783D991A8055FD21D2F3966C96F1B56
)rippleConfig");
    return configContents;
}

/*
   写一个validators.txt文件，完成后删除。
 **/

class ValidatorsTxtGuard : public test::detail::FileDirGuard
{
public:
    ValidatorsTxtGuard (beast::unit_test::suite& test,
        path subDir, path const& validatorsFileName,
            bool useCounter = true)
        : FileDirGuard (test, std::move (subDir),
            path (
                validatorsFileName.empty () ? Config::validatorsFileName :
                validatorsFileName),
            valFileContents (),
            useCounter)
    {
    }

    bool validatorsFileExists () const
    {
        return fileExists();
    }

    std::string validatorsFile () const
    {
        return absolute(file()).string();
    }

    ~ValidatorsTxtGuard ()
    {
    }
};
}  //细节

class Config_test final : public TestSuite
{
private:
    using path = boost::filesystem::path;

public:
    void testLegacy ()
    {
        testcase ("legacy");

        Config c;

        std::string toLoad(R"rippleConfig(
[server]
port_rpc
port_peer
port_wss_admin

[ssl_verify]
0
)rippleConfig");

        c.loadFromString (toLoad);

        BEAST_EXPECT(c.legacy ("ssl_verify") == "0");
expectException ([&c] {c.legacy ("server");});  //不是单行

//设置旧值
        BEAST_EXPECT(c.legacy ("not_in_file") == "");
        c.legacy ("not_in_file", "new_value");
        BEAST_EXPECT(c.legacy ("not_in_file") == "new_value");
    }
    void testDbPath ()
    {
        testcase ("database_path");

        using namespace boost::filesystem;
        {
            boost::format cc ("[database_path]\n%1%\n");

            auto const cwd = current_path ();
            path const dataDirRel ("test_data_dir");
            path const dataDirAbs (cwd / dataDirRel);
            {
//虚拟测试-我们能得到我们放进去的东西吗？
                Config c;
                c.loadFromString (boost::str (cc % dataDirAbs.string ()));
                BEAST_EXPECT(c.legacy ("database_path") == dataDirAbs.string ());
            }
            {
//rel路径应转换为abs路径
                Config c;
                c.loadFromString (boost::str (cc % dataDirRel.string ()));
                BEAST_EXPECT(c.legacy ("database_path") == dataDirAbs.string ());
            }
            {
//没有数据库部分。
//N.B.配置：：安装程序将为数据库路径提供默认值，
//负载不会。
                Config c;
                c.loadFromString ("");
                BEAST_EXPECT(c.legacy ("database_path") == "");
            }
        }
        {
//从文件绝对路径读取
            auto const cwd = current_path ();
            ripple::test::detail::DirGuard const g0(*this, "test_db");
            path const dataDirRel ("test_data_dir");
            path const dataDirAbs(cwd / g0.subdir () / dataDirRel);
            detail::RippledCfgGuard const g (*this, g0.subdir(),
                dataDirAbs, "", false);
            auto const& c (g.config ());
            BEAST_EXPECT(g.dataDirExists ());
            BEAST_EXPECT(g.configFileExists ());
            BEAST_EXPECT(c.legacy ("database_path") == dataDirAbs.string ());
        }
        {
//从文件相对路径读取
            std::string const dbPath ("my_db");
            detail::RippledCfgGuard const g (*this, "test_db", dbPath, "");
            auto const& c (g.config ());
            std::string const nativeDbPath = absolute (path (dbPath)).string ();
            BEAST_EXPECT(g.dataDirExists ());
            BEAST_EXPECT(g.configFileExists ());
            BEAST_EXPECT(c.legacy ("database_path") == nativeDbPath);
        }
        {
//从文件读取没有路径
            detail::RippledCfgGuard const g (*this, "test_db", "", "");
            auto const& c (g.config ());
            std::string const nativeDbPath =
                absolute (g.subdir () /
                          path (Config::databaseDirName))
                    .string ();
            BEAST_EXPECT(g.dataDirExists ());
            BEAST_EXPECT(g.configFileExists ());
            BEAST_EXPECT(c.legacy ("database_path") == nativeDbPath);
        }
    }

    void testValidatorKeys ()
    {
        testcase ("validator keys");

        std::string const validationSeed = "spA4sh1qTvwq92X715tYyGQKmAKfa";

        auto const token =
            "eyJ2YWxpZGF0aW9uX3ByaXZhdGVfa2V5IjoiOWVkNDVmODY2MjQxY2MxOGEyNzQ3Yj"
            "U0Mzg3YzA2MjU5MDc5NzJmNGU3MTkwMjMxZmFhOTM3NDU3ZmE5ZGFmNiIsIm1hbmlm"
            "ZXN0IjoiSkFBQUFBRnhJZTFGdHdtaW12R3RIMmlDY01KcUM5Z1ZGS2lsR2Z3MS92Q3"
            "hIWFhMcGxjMkduTWhBa0UxYWdxWHhCd0R3RGJJRDZPTVNZdU0wRkRBbHBBZ05rOFNL"
            "Rm43TU8yZmRrY3dSUUloQU9uZ3U5c0FLcVhZb3VKK2wyVjBXK3NBT2tWQitaUlM2UF"
            "NobEpBZlVzWGZBaUJzVkpHZXNhYWRPSmMvYUFab2tTMXZ5bUdtVnJsSFBLV1gzWXl3"
            "dTZpbjhIQVNRS1B1Z0JENjdrTWFSRkd2bXBBVEhsR0tKZHZERmxXUFl5NUFxRGVkRn"
            "Y1VEphMncwaTIxZXEzTVl5d0xWSlpuRk9yN0Mwa3cyQWlUelNDakl6ZGl0UTg9In0=";

        {
            Config c;
            static boost::format configTemplate (R"rippleConfig(
[validation_seed]
%1%

[validator_token]
%2%
)rippleConfig");
            std::string error;
            auto const expectedError =
                "Cannot have both [validation_seed] "
                "and [validator_token] config sections";
            try {
                c.loadFromString (boost::str (
                    configTemplate % validationSeed % token));
            } catch (std::runtime_error& e) {
                error = e.what();
            }
            BEAST_EXPECT(error == expectedError);
        }
    }

    void testValidatorsFile ()
    {
        testcase ("validators_file");

        using namespace boost::filesystem;
        {
//缺少指定的验证器文件时应引发加载
            boost::format cc ("[validators_file]\n%1%\n");
            std::string error;
            std::string const missingPath = "/no/way/this/path/exists";
            auto const expectedError =
                "The file specified in [validators_file] does not exist: " +
                missingPath;
            try {
                Config c;
                c.loadFromString (boost::str (cc % missingPath));
            } catch (std::runtime_error& e) {
                error = e.what();
            }
            BEAST_EXPECT(error == expectedError);
        }
        {
//加载应引发无效的[validators\u文件]
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.cfg");
            path const invalidFile = current_path () / vtg.subdir ();
            boost::format cc ("[validators_file]\n%1%\n");
            std::string error;
            auto const expectedError =
                "Invalid file specified in [validators_file]: " +
                invalidFile.string ();
            try {
                Config c;
                c.loadFromString (boost::str (cc % invalidFile.string ()));
            } catch (std::runtime_error& e) {
                error = e.what();
            }
            BEAST_EXPECT(error == expectedError);
        }
        {
//将验证器从配置加载到单个节中
            Config c;
            std::string toLoad(R"rippleConfig(
[validators]
n949f75evCHwgyP4fPVgaHqNHxUVN15PsJEZ3B3HnXPcPjcZAoy7
n9MD5h24qrQqiyBC8aeqqCWvpiBiYQ3jxSr91uiDvmrkyHRdYLUj
n9L81uNCaPgtUJfaHh89gmdvXKAmSt5Gdsw2g1iPWaPkAHW5Nm4C

[validator_keys]
nHUhG1PgAG8H8myUENypM35JgfqXAKNQvRVVAFDRzJrny5eZN8d5
nHBu9PTL9dn2GuZtdW4U2WzBwffyX9qsQCd9CNU4Z5YG3PQfViM8
)rippleConfig");
            c.loadFromString (toLoad);
            BEAST_EXPECT(c.legacy ("validators_file").empty ());
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 5);
        }
        {
//从配置加载验证程序列表站点和密钥
            Config c;
            std::string toLoad(R"rippleConfig(
[validator_list_sites]
ripplevalidators.com
trustthesevalidators.gov

[validator_list_keys]
021A99A537FDEBC34E4FCA03B39BEADD04299BB19E85097EC92B15A3518801E566
)rippleConfig");
            c.loadFromString (toLoad);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 2);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ()[0] ==
                    "ripplevalidators.com");
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ()[1] ==
                    "trustthesevalidators.gov");
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 1);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ()[0] ==
                    "021A99A537FDEBC34E4FCA03B39BEADD04299BB19E85097EC92B15A3518801E566");
        }
        {
//如果配置了[validator_list_sites]但
//[validator_list_keys]不是
            Config c;
            std::string toLoad(R"rippleConfig(
[validator_list_sites]
ripplevalidators.com
trustthesevalidators.gov
)rippleConfig");
            std::string error;
            auto const expectedError =
                "[validator_list_keys] config section is missing";
            try {
                c.loadFromString (toLoad);
            } catch (std::runtime_error& e) {
                error = e.what();
            }
            BEAST_EXPECT(error == expectedError);
        }
        {
//从指定的[validators_file]绝对路径加载
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.cfg");
            BEAST_EXPECT(vtg.validatorsFileExists ());
            Config c;
            boost::format cc ("[validators_file]\n%1%\n");
            c.loadFromString (boost::str (cc % vtg.validatorsFile ()));
            BEAST_EXPECT(c.legacy ("validators_file") == vtg.validatorsFile ());
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 8);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 2);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 2);
        }
        {
//从指定的[validators_file]文件名加载
//在配置目录中
            std::string const valFileName = "validators.txt";
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", valFileName);
            detail::RippledCfgGuard const rcg (
                *this, vtg.subdir (), "", valFileName, false);
            BEAST_EXPECT(vtg.validatorsFileExists ());
            BEAST_EXPECT(rcg.configFileExists ());
            auto const& c (rcg.config ());
            BEAST_EXPECT(c.legacy ("validators_file") == valFileName);
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 8);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 2);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 2);
        }
        {
//从指定的[validators_file]相对路径加载
//到配置目录
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.txt");
            auto const valFilePath = ".." / vtg.subdir() / "validators.txt";
            detail::RippledCfgGuard const rcg (
                *this, vtg.subdir (), "", valFilePath, false);
            BEAST_EXPECT(vtg.validatorsFileExists ());
            BEAST_EXPECT(rcg.configFileExists ());
            auto const& c (rcg.config ());
            BEAST_EXPECT(c.legacy ("validators_file") == valFilePath);
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 8);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 2);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 2);
        }
        {
//从默认位置的验证程序文件加载
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.txt");
            detail::RippledCfgGuard const rcg (*this, vtg.subdir (),
                "", "", false);
            BEAST_EXPECT(vtg.validatorsFileExists ());
            BEAST_EXPECT(rcg.configFileExists ());
            auto const& c (rcg.config ());
            BEAST_EXPECT(c.legacy ("validators_file").empty ());
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 8);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 2);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 2);
        }
        {
//改为从指定的[validators_file]加载
//默认位置的
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.cfg");
            BEAST_EXPECT(vtg.validatorsFileExists ());
            detail::ValidatorsTxtGuard const vtgDefault (
                *this, vtg.subdir (), "validators.txt", false);
            BEAST_EXPECT(vtgDefault.validatorsFileExists ());
            detail::RippledCfgGuard const rcg (
                *this, vtg.subdir (), "", vtg.validatorsFile (), false);
            BEAST_EXPECT(rcg.configFileExists ());
            auto const& c (rcg.config ());
            BEAST_EXPECT(c.legacy ("validators_file") == vtg.validatorsFile ());
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 8);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 2);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 2);
        }

        {
//从配置和验证文件加载验证程序
            boost::format cc (R"rippleConfig(
[validators_file]
%1%

[validators]
n949f75evCHwgyP4fPVgaHqNHxUVN15PsJEZ3B3HnXPcPjcZAoy7
n9MD5h24qrQqiyBC8aeqqCWvpiBiYQ3jxSr91uiDvmrkyHRdYLUj
n9L81uNCaPgtUJfaHh89gmdvXKAmSt5Gdsw2g1iPWaPkAHW5Nm4C
n9KiYM9CgngLvtRCQHZwgC2gjpdaZcCcbt3VboxiNFcKuwFVujzS
n9LdgEtkmGB9E2h3K4Vp7iGUaKuq23Zr32ehxiU8FWY7xoxbWTSA

[validator_keys]
nHB1X37qrniVugfQcuBTAjswphC1drx7QjFFojJPZwKHHnt8kU7v
nHUkAWDR4cB8AgPg7VXMX6et8xRTQb2KJfgv1aBEXozwrawRKgMB

[validator_list_sites]
ripplevalidators.com
trustthesevalidators.gov

[validator_list_keys]
021A99A537FDEBC34E4FCA03B39BEADD04299BB19E85097EC92B15A3518801E566
)rippleConfig");
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.cfg");
            BEAST_EXPECT(vtg.validatorsFileExists ());
            Config c;
            c.loadFromString (boost::str (cc % vtg.validatorsFile ()));
            BEAST_EXPECT(c.legacy ("validators_file") == vtg.validatorsFile ());
            BEAST_EXPECT(c.section (SECTION_VALIDATORS).values ().size () == 15);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_SITES).values ().size () == 4);
            BEAST_EXPECT(
                c.section (SECTION_VALIDATOR_LIST_KEYS).values ().size () == 3);
        }
        {
//如果[validator]、[validator_keys]和
//波纹CFG中缺少[validator_list_keys]和
//验证文件
            Config c;
            boost::format cc ("[validators_file]\n%1%\n");
            std::string error;
            detail::ValidatorsTxtGuard const vtg (
                *this, "test_cfg", "validators.cfg");
            BEAST_EXPECT(vtg.validatorsFileExists ());
            auto const expectedError =
                "The file specified in [validators_file] does not contain a "
                "[validators], [validator_keys] or [validator_list_keys] section: " +
                vtg.validatorsFile ();
            std::ofstream o (vtg.validatorsFile ());
            try {
                Config c;
                c.loadFromString (boost::str (cc % vtg.validatorsFile ()));
            } catch (std::runtime_error& e) {
                error = e.what();
            }
            BEAST_EXPECT(error == expectedError);
        }
    }

    void testSetup(bool explicitPath)
    {
        detail::RippledCfgGuard const cfg(*this, "testSetup",
            explicitPath ? "test_db" : "", "");
        /*RippledCfgGuard有一个加载在
            构造，但config:：setup不可重入，因此我们
            每个测试用例都需要一个新的配置，所以忽略它。
        **/

        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/false，
                /B-沉默*/ false, /* bStandalone */ false);

            BEAST_EXPECT(!config.quiet());
            BEAST_EXPECT(!config.silent());
            BEAST_EXPECT(!config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 256);
            BEAST_EXPECT(!config.legacy("database_path").empty());
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/true，
                /B-沉默*/ false, /* bStandalone */ false);

            BEAST_EXPECT(config.quiet());
            BEAST_EXPECT(!config.silent());
            BEAST_EXPECT(!config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 256);
            BEAST_EXPECT(!config.legacy("database_path").empty());
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/false，
                /B-沉默*/ true, /* bStandalone */ false);

            BEAST_EXPECT(config.quiet());
            BEAST_EXPECT(config.silent());
            BEAST_EXPECT(!config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 256);
            BEAST_EXPECT(!config.legacy("database_path").empty());
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/true，
                /B-沉默*/ true, /* bStandalone */ false);

            BEAST_EXPECT(config.quiet());
            BEAST_EXPECT(config.silent());
            BEAST_EXPECT(!config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 256);
            BEAST_EXPECT(!config.legacy("database_path").empty());
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/false，
                /B-沉默*/ false, /* bStandalone */ true);

            BEAST_EXPECT(!config.quiet());
            BEAST_EXPECT(!config.silent());
            BEAST_EXPECT(config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 0);
            BEAST_EXPECT(config.legacy("database_path").empty() == !explicitPath);
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/true，
                /B-沉默*/ false, /* bStandalone */ true);

            BEAST_EXPECT(config.quiet());
            BEAST_EXPECT(!config.silent());
            BEAST_EXPECT(config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 0);
            BEAST_EXPECT(config.legacy("database_path").empty() == !explicitPath);
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/false，
                /B-沉默*/ true, /* bStandalone */ true);

            BEAST_EXPECT(config.quiet());
            BEAST_EXPECT(config.silent());
            BEAST_EXPECT(config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 0);
            BEAST_EXPECT(config.legacy("database_path").empty() == !explicitPath);
        }
        {
            Config config;
            /*图设置（cfg.configfile（），/*bquiet*/true，
                /B-沉默*/ true, /* bStandalone */ true);

            BEAST_EXPECT(config.quiet());
            BEAST_EXPECT(config.silent());
            BEAST_EXPECT(config.standalone());
            BEAST_EXPECT(config.LEDGER_HISTORY == 0);
            BEAST_EXPECT(config.legacy("database_path").empty() == !explicitPath);
        }
    }

    void testPort ()
    {
        detail::RippledCfgGuard const cfg(*this, "testPort", "", "");
        auto const& conf = cfg.config();
        if (!BEAST_EXPECT(conf.exists("port_rpc")))
            return;
        if (!BEAST_EXPECT(conf.exists("port_wss_admin")))
            return;
        ParsedPort rpc;
        if (!unexcept([&]() { parse_Port (rpc, conf["port_rpc"], log); }))
            return;
        BEAST_EXPECT(rpc.admin_ip && (rpc.admin_ip .get().size() == 2));
        ParsedPort wss;
        if (!unexcept([&]() { parse_Port (wss, conf["port_wss_admin"], log); }))
            return;
        BEAST_EXPECT(wss.admin_ip && (wss.admin_ip .get().size() == 1));
    }

    void run () override
    {
        testLegacy ();
        testDbPath ();
        testValidatorKeys ();
        testValidatorsFile ();
        testSetup (false);
        testSetup (true);
        testPort ();
    }
};

BEAST_DEFINE_TESTSUITE (Config, core, ripple);

}  //涟漪
