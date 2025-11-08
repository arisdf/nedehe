//
// Copyright (c) 2025 arisdf.i2p
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <vector>
#include <string>
#include <tuple>
#include "server_info.hpp"
#include "string.hpp"

namespace nedehe
{

	class server_info_args
	{
	private:
		std::vector<std::string> __args;
	public:
		server_info_args(
			int argc,
			char ** argv
		)
		{
			for (int i=0; i<argc; ++i)
			{
				__args.emplace_back(argv[i]);
			}
		}
	private:
		void check() const
		{
			if (__args.size() != 4)
				throw std::runtime_error{
					"Usage:\n"s
					+ "ws_server <bind ip> <bind port> <mode:rw,wr,sim>\n"
					+ "mode: rw - read then write\n"
					+ "mode: wr - write then read\n"
					+ "mode: sim - read and write simultaneously (run two separated threads)\n"
				};
			if (
				__args[3] != "rw"
				&&
				__args[3] != "wr"
				&&
				__args[3] != "sim"
			)
			{
				throw std::runtime_error{
					"mode should be any one of rw, wr, sim"
				};
			}
		}
	public:
		std::shared_ptr<nedehe::server_info> make_shared_server_info() const
		{
			this->check();
			return std::make_shared<nedehe::server_info>(
				__args[1],
				static_cast<std::uint16_t>(std::stoi(__args[2], nullptr, 10)),
				__args[3]
			);
		}
	};

}	// namespace nedehe

