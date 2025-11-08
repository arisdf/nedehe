//
// Copyright (c) 2025 arisdf.i2p
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <memory>
#include "server_info.hpp"
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <functional>

namespace nedehe
{

	class session:
		virtual public std::enable_shared_from_this<nedehe::session>
	{
	private:
		boost::beast::websocket::stream<boost::beast::tcp_stream> __ws_stream;
		std::shared_ptr<nedehe::server_info> __server_info;
	public:
		session(
			boost::asio::ip::tcp::socket && socket__,
			std::shared_ptr<nedehe::server_info> server_info__
		);
	public:
		virtual ~session() = default;
	public:
		boost::asio::awaitable<void> start();
	private:
		boost::asio::awaitable<void> handshake();
	private:
		boost::asio::awaitable<void> read();
		boost::asio::awaitable<void> write();
	private:
		boost::asio::awaitable<void> run_awaitable_cmd_list(
			std::function<boost::asio::awaitable<void>(void)> awaitable_cmd_list,
			const std::string & label
		);
	private:
		boost::asio::awaitable<void> chat();
	};

}	// namespace nedehe

