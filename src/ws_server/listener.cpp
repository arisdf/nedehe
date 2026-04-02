//
// Copyright (c) 2025 arisdf
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "listener.hpp"
#include "server_info.hpp"
#include "session.hpp"
#include <boost/asio.hpp>
#include <iostream>

nedehe::listener::listener(
	boost::asio::any_io_executor executor__,
	std::shared_ptr<nedehe::server_info> server_info__
):
	__server_info{server_info__},
	__acceptor{
		executor__,
		boost::asio::ip::tcp::endpoint{
			boost::asio::ip::make_address(__server_info->host),
			__server_info->port
		}
	}
{
}

boost::asio::awaitable<void> nedehe::listener::start()
{
	co_await this->accept();
	co_return;
}

boost::asio::awaitable<boost::asio::ip::tcp::socket> nedehe::listener::accept()
{
	while (true)
	{
		auto [ec, socket] = co_await __acceptor.async_accept(
			co_await boost::asio::this_coro::executor,
			boost::asio::as_tuple
		);
		if (ec)
		{
			std::cout << "A websocket client connection is lost, ignored." << std::endl;
			continue;
		}
		else
		{
			std::cout << "A websocket client connection is created." << std::endl;
		}
		boost::asio::co_spawn(
			co_await boost::asio::this_coro::executor,
			std::bind(
				&nedehe::session::start,
				std::make_shared<nedehe::session>(
					std::move(socket),
					__server_info
				)
			),
			[] (std::exception_ptr eptr)
			{
				if (eptr)
				{
					std::cout << "A websocket client session fails, ignored." << std::endl;
				}
			}
		);
	}
}

