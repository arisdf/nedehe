//
// Copyright (c) 2025 arisdf.i2p
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio.hpp>
#include <string_view>
#include "server_info.hpp"

namespace nedehe
{
	class listener:
		virtual public std::enable_shared_from_this<nedehe::listener>
	{
	private:
		std::shared_ptr<nedehe::server_info> __server_info;
		boost::asio::ip::tcp::acceptor __acceptor;
	public:
		listener(
			boost::asio::any_io_executor executor__,
			std::shared_ptr<nedehe::server_info> server_info__
		);
	public:
		virtual ~listener() = default;
	public:
		boost::asio::awaitable<void> start();
		boost::asio::awaitable<boost::asio::ip::tcp::socket> accept();
	};
}	// namespace nedehe

