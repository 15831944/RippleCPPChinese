
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
//版权所有（c）2011至今，Facebook，Inc.保留所有权利。
//此源代码在两个gplv2下都获得了许可（在
//复制根目录中的文件）和Apache2.0许可证
//（在根目录的license.apache文件中找到）。
//
#pragma once
#if !defined(IOS_CROSS_COMPILE)
//如果我们用xcode编译，我们不会运行build_detect_版本，所以我们不会
//生成这些变量
//这个变量告诉我们关于git版本的信息
extern const char* rocksdb_build_git_sha;

//代码编译日期：
extern const char* rocksdb_build_compile_date;
#endif