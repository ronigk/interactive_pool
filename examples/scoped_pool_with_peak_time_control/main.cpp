/* .....................................................................
 * scoped_pool_with_peak_time_control
 * Show the usage of the interactive_pool using metrics and a plugin 
 * that trigger an alert when the connection time exceeds a configured
 * threshold value
 * LICENSE: MIT
 * developed by Roni Gonzalez - <roni.gonzalez@interconetica.com>
 * on june, 2023
 * ..................................................................... */
#include <iostream>
#include "./../../include/interactive_pool.h"
#include <thread>
#include <chrono>
#include <algorithm>
#include <string>
#include <vector>

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
		// cout <<"Thread " << std::this_thread::get_id() << " Finished to write" << endl;
	}
	
	~Foo() = default;
};


// Call back function instead lambda for this example
void peak_alarm_function( const std::string& id, uint32_t trigger_level, uint32_t peak_alarm )
{
	cout << "Has triggered peak time to access pool " << peak_alarm << " ms." << " Reported by pool: " << id << endl;
}


// worker thread 
// worker_with_scope_average_detector
void worker_with_scope_peak_detector(interactive_pool< Foo > * pool)
{
	// interactive_average_detector plugin
	// allow to call a function or a lambda to trigger an alert when the connection time exceeds a configured threshold value
	// In this case we are using a string to identify the pool instead thre thread 
	// interactive_peak_detector< T = identification type >  Ex.: interactive_average_detector<std::string>
	// Id -> Identification
	// Trigger level , threshold value in ms
	// Function to call with parameters
	//	-> id, same as constructor
	//  -> trigger_level , same tha constructor
	//	-> peak_alarm : value above the threshold
	interactive_peak_detector<std::string> _peak ( string("Connection Pool 1"), 1300, &peak_alarm_function );
	
	for (int i = 0; i < operations; i++)
	{
		try
		{
			interactive_pool_time elapsedTime;
			interactive_pool_scoped_connection<Foo> c( pool, numeric_limits<uint32_t>::max(), &elapsedTime, &_peak );
			// do task
			c->Write();

		}
		catch (std::exception& e)
		{
			cout << "Thread " << std::this_thread::get_id() << " Exception " << string(e.what()) << endl;
			i--;
		}

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
	 std::for_each(t.begin(), t.end(), [&pool](auto& th) {th = std::thread(&worker_with_scope_peak_detector, &pool); });
	
	
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
