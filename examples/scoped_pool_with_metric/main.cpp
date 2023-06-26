#include <iostream>
#include "./../../include/interactive_pool.h"
#include <thread>
#include <chrono>
#include <algorithm>


using namespace std;

const int threads = 15;				// Working threads , that consumes thepool resources
const int interval = 5;				// Interval on each thread iteration
const int operations  = 20;		// Count of writes of each thread before to finish
const int work_duration_ms = 100;	// fake value in ms simulating a task duration
const int pool_size = 2;			// Size of pool ( amount of resources )

// class used in pool simulating some resource
class Foo {
public:
	Foo() {}
    void Write () {
		
		std::this_thread::sleep_for(std::chrono::milliseconds(work_duration_ms));
		cout <<"Thread " << std::this_thread::get_id() << " Finished to write" << endl;
	}
	
	~Foo() = default;
};


// worker thread 
void worker_with_scope(interactive_pool<Foo>* pool)
{
	for (int i = 0; i < operations; i++) 
	{
		try
		{
			interactive_pool_time elapsedTime; // get elapsed time necessary to connect
			interactive_pool_scoped_connection<Foo> c( pool, 2000, &elapsedTime );

			cout << "Thread " << std::this_thread::get_id() << " got item in " << elapsedTime.elapsed_time.count() << " ms" << endl;
			c->Write();
		}
		catch (std::exception& e)
		{
			cout << "Thread " << std::this_thread::get_id() << " Exception " << string(e.what()) << endl;
			i--; // must complete all tasks 
		}
		// rest a little
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	}
}



int main()
{
	// pool creation
	interactive_pool< Foo > pool(pool_size);
	// container for all threads
	std::vector<std::thread> t(threads);

	// using scoped call
	 std::for_each(t.begin(), t.end(), [&pool](auto& th) {th = std::thread(&worker_with_scope, &pool); });
	
	
	// wait for the end of all tasks
	std::for_each(t.begin(), t.end(), [](auto& th) {th.join(); });

	try
	{
		// check if all itemns are released
		pool.check_before_destruct();
	}
	catch (std::exception&  e)
	{
		cout << "Exception " << string(e.what());
	}

	cout << "End of example " << endl;
	return 0;
}
