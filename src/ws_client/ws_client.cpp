//
// Copyright (c) 2025 arisdf.i2p
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>	// std::cout, general c++
#include <memory>	// c++ smart-ptr
#include <string>
#include <string_view>

using std::string_literals::operator""s;
using std::string_view_literals::operator""sv;

namespace nedehe
{

	class server_info:
		virtual public std::enable_shared_from_this<nedehe::server_info>
	{
	public:
		const std::string host;
		const std::string port;
		std::string mode;
	public:
		server_info(
			const std::string & host__,
			const std::string & port__,
			const std::string & mode__
		):
			host{host__},
			port{port__},
			mode{mode__}
		{
		}
	};

	class session:
		virtual public std::enable_shared_from_this<nedehe::session>
	{
	private:
		boost::beast::websocket::stream<boost::beast::tcp_stream> __ws_stream;
		std::shared_ptr<nedehe::server_info> __server;
	public:
		session(
			boost::asio::ip::tcp::socket && socket__,
			std::shared_ptr<nedehe::server_info> server__
		):
			__ws_stream{std::move(socket__)},
			__server{server__}
		{
		}
	public:
		virtual ~session() = default;
	public:
		boost::asio::awaitable<void> start()
		{
			co_await this->handshake();
			std::cout << "awesome, A c++ websocket client session is started !" << std::endl;
			std::cout << "Seeing this message means the websocket client is OK."
				<< std::endl;
			co_await this->chat();
		}
	private:
		boost::asio::awaitable<void> handshake()
		{
			__ws_stream.next_layer().expires_after(std::chrono::seconds(30));
			auto [ec] = co_await __ws_stream.async_handshake(
				__server->host + ":" + __server->port,
				"/",
				boost::asio::as_tuple
			);
			if (ec)
				throw std::system_error{
					ec,
					"Handshake server error (should terminate program)."
				};
		}
	private:
		boost::asio::awaitable<void> chat()
		{
			if (__server->mode == "wr")
			{
				co_await this->run_awaitable_cmd_list(
					[self=this->shared_from_this()] -> boost::asio::awaitable<void>
					{
						while (true)
						{
							co_await self->write();
							co_await self->read();
						}
					},
					"mode: wr"
				);
			}
			else if (__server->mode == "rw")
			{
				co_await this->run_awaitable_cmd_list(
					[self=this->shared_from_this()] -> boost::asio::awaitable<void>
					{
						while (true)
						{
							co_await self->read();
							co_await self->write();
						}
					},
					"mode: rw"
				);
			}
			else if (__server->mode == "sim")
			{
				co_await this->run_awaitable_cmd_list(
					[self=this->shared_from_this()] -> boost::asio::awaitable<void>
					{
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
						while (true)
						{
							co_await self->write();
						}
					},
					"mode: sim,(write)"
				);
			}
			else
			{
				std::cout << "You have not set mode, do you want to set mode now?\n"
					<< "wr - write then read;\n"
					<< "rw - read then write;\n"
					<< "sim - read and write simultaneously;\n"
					<< "(not above, the program will quit now.\n"
					<< "Please choose: "
					<< std::flush;
				std::string get;
				std::getline(std::cin, get);
				if (
					get != "wr"
					&&
					get != "rw"
					&&
					get != "sim"
				)
				{
					throw std::runtime_error{
						"No mode, and user does not choose mode (program should terminate)"
					};
				}
				// else
				__server->mode = get;
				co_await this->chat();
			}
			co_return;
		}
	private:
		boost::asio::awaitable<void> run_awaitable_cmd_list(
			std::function<boost::asio::awaitable<void>(void)> awaitable_cmd_list,
			const std::string_view label
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
						catch (const std::system_error & e)
						{
							if ("exit"s == e.what())
							{
								std::cout << "[Uer Exit] (1)\n";
								std::rethrow_exception(eptr);
							}
							std::cout << "==== unexpected exception (1) ==== "
								<< label << "==== ";
							std::cout << e.what() << " - " << e.code() << std::endl;
						}
						catch (const std::exception & e)
						{
							if ("exit"s == e.what())
							{
								std::cout << "[Uer Exit] (2)\n";
								std::rethrow_exception(eptr);
							}
							std::cout << "==== unexpected exception (2) ==== "
								<< label << "==== ";
							std::cout << e.what() << std::endl;
						}
						catch (...)
						{
							std::cout << "==== unexpected exception (3) ===="
								<< label << "==== ";
							std::cout << std::endl;
						}
					}
				}
			);
		}
	private:
		// Read and write should not terminate program (we do not throw exception simply)
		boost::asio::awaitable<void> write()
		{
			std::cout << "What do you want to say? (exit to exit) send >>> " << std::flush;
			std::string get;
			std::getline(std::cin, get);
			if (get == "exit")
				throw std::runtime_error{"exit"};
			__ws_stream.next_layer().expires_after(std::chrono::seconds(10));
			auto [ec, bytes] = co_await __ws_stream.async_write(
				boost::asio::buffer(get),
				boost::asio::as_tuple
			);
			if (bytes < 1 || (ec && ec != boost::asio::error::eof))
			{
				std::cout << "Send error! Please try send again," << std::endl;
				co_return co_await this->write();
			}
			std::cout << "Sent:\n";
			std::cout.write(get.data(), bytes);
			std::cout << std::endl;
		}
	private:
		// Read and write should not terminate program (we do not throw exception simply)
		boost::asio::awaitable<void> read()
		{
			boost::beast::flat_buffer buffer;
			__ws_stream.next_layer().expires_after(std::chrono::seconds(10));
			auto [ec, bytes] = co_await __ws_stream.async_read(
				buffer,
				boost::asio::as_tuple
			);
			if (bytes < 1 || (ec && ec != boost::asio::error::eof))
			{
				while (true)
				{
					std::cout << "Receive error, do you want to try receive again? (yes/no) >>> "
						<< std::flush;
					std::string get;
					std::getline(std::cin, get);
					if (get == "yes")
					{
						co_return co_await this->read();
						//co_return;
					}
					else if (get == "no")
					{
						co_return;
					}
				}
			}
			std::cout << "Received:\n"
				<< boost::beast::make_printable(buffer.data()) << std::endl;
		}
	};

	class ws_client:
		virtual public std::enable_shared_from_this<nedehe::ws_client>
	{
	private:
		boost::asio::ip::tcp::resolver __resolver;
		std::shared_ptr<nedehe::server_info> __server;
	public:
		ws_client(
			boost::asio::any_io_executor executor__,
			std::shared_ptr<nedehe::server_info> server__
		):
			__resolver{
				executor__
			},
			__server{server__}
		{
		}
	public:
		virtual ~ws_client() = default;
	public:
		boost::asio::awaitable<void> start()
		{
			co_await this->make_session();
		}
	private:
		boost::asio::awaitable<void> make_session()
		{
			auto socket = co_await this->connect(co_await this->resolve());
			boost::asio::co_spawn(
				co_await boost::asio::this_coro::executor,
				std::bind(
					&nedehe::session::start,
					std::make_shared<nedehe::session>(
						std::move(socket),
						this->__server
					)
				),
				[] (std::exception_ptr eptr)
				{
					if (eptr)
					{
						std::cout << "Forwarding c++ exception, "
							<< std::endl;
						std::rethrow_exception(eptr);
					}
				}
			);
		}
	private:
		boost::asio::awaitable<boost::asio::ip::tcp::resolver::results_type> resolve()
		{
			auto [ec, ep_list] = co_await __resolver.async_resolve(
				__server->host,
				__server->port,
				boost::asio::as_tuple
			);
			if (ec)
				throw std::system_error{
					ec,
					"Resolve server error! (should terminate program)"
				};
			co_return ep_list;
		}
		boost::asio::awaitable<boost::asio::ip::tcp::socket> connect(auto && ep_list)
		{
			boost::asio::ip::tcp::socket socket{co_await boost::asio::this_coro::executor};
			auto [ec, ep] = co_await boost::asio::async_connect(
				socket,
				ep_list,
				boost::asio::as_tuple
			);
			if (ec)
				throw std::system_error{
					ec,
					"Connect server error! (should terminate program)"
				};
			co_return socket;
		}
	};

}	// namespace nedehe

int main(int argc, char * argv[])
{
	try
	{
		std::cout << "Hello c++ World!" << std::endl;
		if (argc != 4)
			throw std::runtime_error{
				"Usage:\n"s
				+ "ws_client <host> <port> <mode:wr,rw,sim>\n"
				+ "mode: wr - write first, then read\n"
				+ "mode: rw - read first, then write\n"
				+ "mode: sim - run two threads: read and write simultaneously\n"
				+ "Example:\n"
				+ "ws_client 127.0.0.1 9000 rw"
			};
		boost::asio::io_context io_context(1);

		if (
			"wr"s != argv[3]
			&&
			"rw"s != argv[3]
			&&
			"sim"s != argv[3]
		)
		{
			throw std::runtime_error{
				"Mode error: mode should be any of wr, rw, sim"
			};
		}

		boost::asio::co_spawn(
			io_context,
			[
				server = std::make_shared<nedehe::server_info>(
					std::string{argv[1]},
					std::string{argv[2]},
					std::string{argv[3]}
				)
			]
			-> boost::asio::awaitable<void>
			{
				auto wscli = std::make_shared<nedehe::ws_client>(
					co_await boost::asio::this_coro::executor,
					server
				);
				co_await wscli->start();
			},
			[] (std::exception_ptr eptr)
			{
				if (eptr)
				{
					std::cout << "Forwarding c++ exception ...\n";
					std::rethrow_exception(eptr);
				}
			}
		);

		io_context.run();
	}
	catch (const std::exception & e)
	{
		if ("exit"s == e.what())
		{
			std::cout << "User Exit\n";
			return 0;
		}
		std::cerr
			<< "========================================\n"
			<< "========================================\n"
			<< "==== c++ std::exception, general error, e.what():\n\n"
			<< e.what()
			<< std::endl
			<< std::endl
		;
		return 1;
	}
	catch (...)
	{
		std::cerr
			<< "========================================\n"
			<< "========================================\n"
			<< "==== c++ std::exception, general error, unknown error\n"
			<< std::endl
		;
		return 2;
	}
	return 0;
}

