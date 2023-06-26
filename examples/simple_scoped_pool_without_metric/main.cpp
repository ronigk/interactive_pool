/* .....................................................................
 * simple_pool_without_metric
 * Show the usage of the interactive_pool as a simple pool without
 * taking sistem metrics
 * LICENSE: MIT
 * deloped by Roni Gonzalez - <roni.gonzalez@interconetica.com>
 * on june, 2023
 * ..................................................................... */

#include <iostream>
#include "./../../include/interactive_pool.h"
#include <thread>
#include <chrono>
#include <algorithm>



using namespace std;

const int threads = 15;				// Number of threads to be launched
const int interval = 5;				// Sleep interval, in milliseconds, for each thread execution
const int operations  = 50;		// Number of successfully operations to be done by each thread
const int work_duration_ms = 20;	// used to simulate a work task duration
const int connect_duration_ms = 10;	// used to simulate a db connection duration
const int pool_size = 10;			// size of the resources pool

/// class MyConnection  
/// Simulates a class manage a fake database or redis
const char* _host = "192.168.42.165";		// fake url of database
const int   _port = 6379;					// fake port of database
typedef struct 
{
	
	void Connect(const char* ip, int port) {
		
		cout << "Connecting to database ... " << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(connect_duration_ms));
		cout << "Successfully connected to database ... " << endl;
		 bConnected = true;
	}
		
	bool isConnected() {
		return bConnected;
	}
	
	bool ping() {
		
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		return true;
	}
	
	void execute( const std::string& command )
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(work_duration_ms));
	}
	
	bool bConnected = false;
}  MyConnection ;



// Manages the connection to database
class MyConnectors
{
public:
	MyConnectors() {

		_driver = new MyConnection;
	}
	
    bool is_connected () {
		
		if( !_driver->isConnected() )
		{
			// connect
			_driver->Connect(_host, _port);
		}
		else
		{
			// test connection
			_driver->ping();
		}
		
		return _driver->isConnected();
	}
	
	void execute( const std::string& command )
	{
		_driver->execute(command);
	}
	
	virtual ~MyConnectors() 
	{
		delete _driver;
	}
private:
	MyConnection* _driver;
	
};






/// Worker thread that consumes the pool's resources 
void worker(interactive_pool< MyConnectors >* pool)
{
	for (int i = 0; i < operations; i++)
	{
		try
		{
			// max waits 1 second to get an instance
			interactive_pool<MyConnectors>::item c = pool->get_item(1000);

			// test connection
			if( c->is_connected() )
			{
				c->execute("keys *");
				cout << "Thread " << std::this_thread::get_id() << " finished to execute command" << endl;
			}

			// reelase item
			pool->set_item(c);
		}
		catch (std::exception& e)
		{
			cout << "Thread " << std::this_thread::get_id() << " Exception " << string(e.what()) << endl;
			i--; // this attempt will no count
		}

		// sleep a little between works
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	}
}



int main()
{

	// pools of MyConnectors class 
	interactive_pool< MyConnectors > pool(pool_size);
	
	// container for all threads 
	std::vector<std::thread> t(threads);

	// create threads passing the pool instance
	std::for_each(t.begin(), t.end(), [&pool](auto& th) {th = std::thread(&worker, &pool);});

	// wait for fnish all threads
	std::for_each(t.begin(), t.end(), [](auto& th) {th.join(); });

	try
	{
		// befor end do this check to verify that threads are correctly implemented and are releasin all pool instances
		pool.check_before_destruct();
	}
	catch (std::exception&  e)
	{
		cout << "Exception " << string(e.what());
	}

	cout << "End of execution" << endl;
	
	return 0;
}
