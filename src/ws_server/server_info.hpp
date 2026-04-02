//
// Copyright (c) 2025 arisdf
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <string>
#include <memory>

namespace nedehe
{
	class server_info:
		virtual public std::enable_shared_from_this<nedehe::server_info>
	{
	public:
		const std::string host;
		const std::uint16_t port;
		// wr: write then read, rw: read then write, sim: simultaneously
		std::string mode;
	public:
		server_info(
			const std::string & host__,
			const std::uint16_t & port__,
			const std::string & mode__
		):
			host{host__},
			port{port__},
			mode{mode__}
		{
		}
	};
}	// namespace nedehe

