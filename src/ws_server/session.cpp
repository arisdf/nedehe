//
// Copyright (c) 2025 arisdf
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "session.hpp"
#include <iostream>
#include <random>
#include <iostream>
#include "string.hpp"

nedehe::session::session(
	boost::asio::ip::tcp::socket && socket__,
	std::shared_ptr<nedehe::server_info> server_info__
):
	__ws_stream{
		std::move(socket__)
	},
	__server_info{server_info__}
{
}

boost::asio::awaitable<void> nedehe::session::start()
{
	std::cout << "c++ Websocket Sesion ...\n";
	co_await this->handshake();
	co_await this->chat();
	co_return;
}

boost::asio::awaitable<void> nedehe::session::handshake()
{
	__ws_stream.next_layer().expires_after(std::chrono::seconds(30));
	auto [ec] = co_await __ws_stream.async_accept(
		boost::asio::as_tuple
	);
	if (ec)
		throw std::system_error{ec, "Websocket session handshake from client failed."};
	std::cout << "Websocket session hanshake from client OK." << std::endl;
}

boost::asio::awaitable<void> nedehe::session::read()
{
	boost::beast::flat_buffer buffer;
	__ws_stream.next_layer().expires_after(std::chrono::seconds(10));
	auto [ec, bytes] = co_await __ws_stream.async_read(
		buffer,
		boost::asio::as_tuple
	);
	if (bytes < 1 || (ec && ec != boost::asio::error::eof))
	{
		co_return;
	}
	std::cout << "Received:\n"
		<< boost::beast::make_printable(buffer.data()) << std::endl;
	co_return;
}

boost::asio::awaitable<void> nedehe::session::write()
{
	static std::random_device rd;
	std::mt19937 rng{rd()};
	__ws_stream.next_layer().expires_after(std::chrono::seconds(10));
	auto [ec, bytes] = co_await __ws_stream.async_write(
		boost::asio::buffer("random number: "s + std::to_string(rng())),
		boost::asio::as_tuple
	);
}

boost::asio::awaitable<void> nedehe::session::run_awaitable_cmd_list(
	std::function<boost::asio::awaitable<void>(void)> awaitable_cmd_list,
	const std::string & label
)
{
	boost::asio::co_spawn(
		co_await boost::asio::this_coro::executor,
		std::bind(awaitable_cmd_list),
		[label=label] (std::exception_ptr eptr)
		{
			if (eptr)
			{
				try
				{
					std::rethrow_exception(eptr);
				}
				catch (const std::exception & e)
				{
					std::cout << "==== " << label << " ==== " << e.what() << std::endl;
				}
			}
		}
	);
	co_return;
}

boost::asio::awaitable<void> nedehe::session::chat()
{
	if (__server_info->mode == "rw")
	{
		co_await this->run_awaitable_cmd_list(
			[self=this->shared_from_this()] -> boost::asio::awaitable<void>
			{
				std::cout << "Mode: " << self->__server_info->mode << std::endl;
				while (true)
				{
					co_await self->read();
					co_await self->write();
				}
			},
			"mode: rw"
		);
	}
	else if (__server_info->mode == "wr")
	{
		co_await this->run_awaitable_cmd_list(
			[self=this->shared_from_this()] -> boost::asio::awaitable<void>
			{
				std::cout << "Mode: " << self->__server_info->mode << std::endl;
				while (true)
				{
					co_await self->write();
					co_await self->read();
				}
			},
			"mode: wr"
		);
	}
	else if (__server_info->mode == "sim")
	{
		co_await this->run_awaitable_cmd_list(
			[self=this->shared_from_this()] -> boost::asio::awaitable<void>
			{
				std::cout << "Mode: " << self->__server_info->mode << std::endl;
				while (true)
				{
					co_await self->read();
				}
			},
			"mode: sim,(read)"
		);
		co_await this->run_awaitable_cmd_list(
			[self=this->shared_from_this()] -> boost::asio::awaitable<void>
			{
				std::cout << "Mode: " << self->__server_info->mode << std::endl;
				while (true)
				{
					co_await self->write();
				}
			},
			"mode: sim,(write)"
		);
	}
	co_return;
}

