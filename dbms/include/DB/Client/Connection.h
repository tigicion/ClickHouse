#include <Poco/Net/StreamSocket.h>

#include <DB/Core/Block.h>
#include <DB/Core/Progress.h>
#include <DB/Core/Protocol.h>
#include <DB/Core/QueryProcessingStage.h>

#include <DB/IO/ReadBufferFromPocoSocket.h>
#include <DB/IO/WriteBufferFromPocoSocket.h>

#include <DB/DataTypes/DataTypeFactory.h>

#include <DB/DataStreams/IBlockInputStream.h>
#include <DB/DataStreams/IBlockOutputStream.h>


namespace DB
{

using Poco::SharedPtr;


/** Соединение с сервером БД для использования в клиенте.
  * Как использовать - см. Core/Protocol.h
  * (Реализацию на стороне сервера - см. Server/TCPHandler.h)
  */
class Connection
{
public:
	Connection(const String & host_, UInt16 port_,
		DataTypeFactory & data_type_factory_,
		Protocol::Compression::Enum compression_ = Protocol::Compression::Enable)
		: host(host_), port(port_), socket(), in(socket), out(socket),
		query_id(0), compression(compression_), data_type_factory(data_type_factory_)
	{
		connect();
	}

	virtual ~Connection() {};


	/// Пакет, который может быть получен от сервера.
	struct Packet
	{
		UInt64 type;

		Block block;
		SharedPtr<Exception> exception;
		Progress progress;

		Packet() : type(Protocol::Server::Hello) {}
	};


	void sendQuery(const String & query, UInt64 query_id_ = 0, UInt64 stage = QueryProcessingStage::Complete);
	void sendCancel();
	void sendData(Block & block);

	/// Проверить, если ли данные, которые можно прочитать.
	bool poll(size_t timeout_microseconds = 0);

	/// Получить пакет от сервера.
	Packet receivePacket();
	
private:
	String host;
	UInt16 port;
	
	Poco::Net::StreamSocket socket;
	ReadBufferFromPocoSocket in;
	WriteBufferFromPocoSocket out;

	UInt64 query_id;
	UInt64 compression;		/// Сжимать ли данные при взаимодействии с сервером.

	DataTypeFactory & data_type_factory;

	/// Откуда читать результат выполнения запроса.
	SharedPtr<ReadBuffer> chunked_in;
	SharedPtr<ReadBuffer> maybe_compressed_in;
	BlockInputStreamPtr block_in;

	/// Куда писать данные INSERT-а.
	SharedPtr<WriteBuffer> chunked_out;
	SharedPtr<WriteBuffer> maybe_compressed_out;
	BlockOutputStreamPtr block_out;

	void connect();
	void sendHello();
	void receiveHello();
	void forceConnected();
	bool ping();

	Block receiveData();
	SharedPtr<Exception> receiveException();
	Progress receiveProgress();
};

}
