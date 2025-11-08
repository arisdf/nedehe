//
// Copyright (c) 2025 arisdf.i2p
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "listener.hpp"
#include "args.hpp"
#include "string.hpp"
#include <iostream>

int main(int argc, char * argv[])
{
	try
	{
		boost::asio::io_context io_context;
		auto server_info = nedehe::server_info_args{argc, argv}.make_shared_server_info();
		boost::asio::co_spawn(
			io_context,
			[server_info] -> boost::asio::awaitable<void>
			{
				auto listener = std::make_shared<nedehe::listener>(
					co_await boost::asio::this_coro::executor,
					server_info
				);
				co_return co_await listener->start();
			},
			[] (std::exception_ptr eptr)
			{
				if (eptr)
				{
					std::cout << "Forwarding c++ exception,\n";
					std::rethrow_exception(eptr);
				}
			}
		);
		io_context.run();
	}
	catch (const std::system_error & e)
	{
		if ("exit"s == e.what())
		{
			std::cout << "[User Exit]\n";
			return 0;
		}
		std::cout << "=============================================\n";
		std::cout << "=============================================\n";
		std::cout << "=== c++ std::system_error, e.what(): "
			<< e.what() << " --- e.code(): " << e.code() << std::endl;
		return 1;
	}
	catch (const std::exception & e)
	{
		if ("exit"s == e.what())
		{
			std::cout << "[User Exit]\n";
			return 0;
		}
		std::cout << "=============================================\n";
		std::cout << "=============================================\n";
		std::cout << "=== c++ std::exception, e.what(): "
			<< e.what() << std::endl;
		return 2;
	}
	catch (...)
	{
		std::cout << "=============================================\n";
		std::cout << "=============================================\n";
		std::cout << "=== c++ unknown exception" << std::endl;
		return 3;
	}
}

