/* .....................................................................
 * interactive_pool
 * Header only template for a pool of resources that can report about
 * the timing and the system load
 * LICENSE: MIT
 * deloped by Roni Gonzalez - <roni.gonzalez@interconetica.com>
 * on june, 2023
 * ..................................................................... */

#ifndef INTERACTIVE_POOL__H
#define INTERACTIVE_POOL__H
#include <iostream>
#include <memory>
#include <deque>

#include <mutex>
#include <chrono>
#include <functional>
#include <numeric>
#include <thread>

/// interactive_pool_time
// structure use for metrics 
typedef struct {

	std::chrono::time_point<std::chrono::high_resolution_clock> init;
	std::chrono::time_point<std::chrono::high_resolution_clock> finish;
	std::chrono::milliseconds elapsed_time;

} interactive_pool_time;


/// interactive_pool
/// main class , created on 2026-06-20 
/// author <roni.gonzalez@interconetica.com>
/// summary: interactive_pool try yop be a template class that manages a simple pool of resources
/// giving an option of monitoring the pool load 
template <class T> class  interactive_pool
{
public:
	// defines a pool's item
	typedef  std::unique_ptr< T > item;
	// Constructor 
	// size : number of resournces (initial buffer size)
	// ini_func : function initializer 
	// init_result : function for receive the initialize results
	interactive_pool(size_t size)
		: _initialSize(size)
	{
		_freeItems.resize(size);
		std::for_each(_freeItems.begin(), _freeItems.end(), [](item& i) {i = std::move(std::make_unique<T>()); });
	}

	//check_before_destruct()
	// can be called before destruct the pool, detect if exists a size difference between initial size and current size
	// Throws a exception if some element is not released
	// * Never calls directly in your destructor. Check the examples to see how to use it.
	void check_before_destruct()
	{
		std::lock_guard<std::mutex> l(_lock);
		size_t current = _freeItems.size();
		if (current != _initialSize)
		{
			throw std::runtime_error(std::string(std::string("interactive_pool: Different count of items. Pool was created with [") + std::to_string(_initialSize) + std::string("] but during destruction have [") + std::to_string(current) + std::string("]")));
		}
	}

	// Destructor
	// releases all items
	virtual ~interactive_pool()
	{
		std::for_each(_freeItems.begin(), _freeItems.end(), [](item& i) {i.reset(); });
		_freeItems.clear();
	}


	// get_item()
	// returns a item to caller or throw an exception if pool is empty
	// max_wait_ms		: maximun time, in milliseconds, to wait a free instance.  Once this time has elapsed, an exception will be thrown
	//					  special values: 
	//							0 -> try just once.
	//							Default = numeric_limits<uint32_t>::max()
	// time_elapsed_ms 	: Time, in milliseconds, it took to get an instance from the thread pool
	item get_item(uint32_t max_wait_ms = std::numeric_limits<uint32_t>::max(), interactive_pool_time* time_elapsed_ms = nullptr)
	{
		auto t0 = std::chrono::high_resolution_clock::now();
		auto t1 = t0;
		std::chrono::duration<double, std::milli> elapsed;

		if (time_elapsed_ms)
		{
			// get initial time point if metric is requested
			time_elapsed_ms->init = std::chrono::high_resolution_clock::now();
		}

		do
		{
			{ // lock scope
				std::lock_guard<std::mutex> l(_lock);
				
				if (!_freeItems.empty())
				{
					// got at least 1 item, reuturn it and remove from pool
					item j = std::move(_freeItems.front());
					_freeItems.pop_front();
					
					if (time_elapsed_ms)
					{
						// if metric is requested, calculate elapsed time
						time_elapsed_ms->finish = std::chrono::high_resolution_clock::now();
						time_elapsed_ms->elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(time_elapsed_ms->finish - time_elapsed_ms->init);
					}
					// return item
					return j;
				}
			} // end lock scope

			// not items available, wait till timeout
			t1 = std::chrono::high_resolution_clock::now();
			// used to check timeout
			elapsed = (t1 - t0);
			// rest a little 
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		} while (elapsed.count() < max_wait_ms);

		// no free items
		throw std::runtime_error("interactive_pool: All items are in use");
	}

	// get_available_count()
	// returns the number of free items in the pool
	size_t get_available_count()
	{
		std::lock_guard<std::mutex> l(_lock);
		return _freeItems.size();
	}

	// set_connection()
	// push the connection back to the pool
	void set_item(item& r)
	{
		std::lock_guard<std::mutex> l(_lock);
		_freeItems.push_back(std::move(r));
	}
	

private:
	size_t				 _initialSize;
	std::deque < item > _freeItems;
	std::mutex		     _lock;
};




/// base class for detectors
typedef struct  { virtual void set_elapsed_time(const uint32_t&) = 0; } base_detector;


/// interactive_average_detector
/// Metric facilities function, call a specific function when the average of 
/// last "n" calls to get_item exceed the limit set in milliseconds
template < class T > class interactive_average_detector : public base_detector
{public:

	// calback definition
	typedef std::function<void(T,  uint32_t, uint32_t)> _average_limit_call_back ;
	interactive_average_detector( T id, size_t samples , uint32_t trigger_level, _average_limit_call_back fcn )
		: _id(id)
		, _samples_count(samples), _samples(samples), _trigger_level(trigger_level) , _lcall_back(fcn)
		{}

	// timming control function
	virtual void set_elapsed_time( const uint32_t& i )
	{
		if( _samples.size() >= _samples_count )
		{
			// remove oldest value if buffer is full
			_samples.pop_front();
		} 
		
		// add the received interval to buffer
		_samples.push_back(i);
		
		if( _samples.size() == _samples_count )
		{
			// calculate the averange if the buffer is completed
			uint32_t cur = average();
			if( cur > _trigger_level )
			{
				// call designed function
				_lcall_back( _id, _trigger_level, cur );
			}
		}
	}
	
	// auxiliar method, calculates the time average of all samples
	uint32_t average() const 
	{
		return _samples.empty()? 0 :
			((std::accumulate(_samples.begin(), _samples.end(), 0)) / static_cast<uint32_t>(_samples.size())) ;
	}
	
private:
	T _id;
	size_t _samples_count;
	std::deque<uint32_t> _samples;
	uint32_t _trigger_level;
	_average_limit_call_back _lcall_back;
};



/// interactive_peak_detector
/// Calls a user function each time that the elapsed time to get a item in the pool
/// exceeds the set trigger value
template < class T > class interactive_peak_detector : public base_detector
{public:

	// callback prototipe
	typedef std::function<void(T,  uint32_t, uint32_t)> _peack_detected_call_back ;
	// constructor
	// id			: identifier, at the discretion of the user, to identify the thread or instances that have issued the peak signal. 
	// trigger_level: Maximum time in milliseconds. If this time is exceeded while waiting for the free item in the pool. 
	// 				  The function designated as callback is called. It does not affect the wait for items pool.
	// fcn			: user-defined function, called when peak detection occurs
	//				  parameters :
	//					id: Defined by user
	//					level: conffigured trigger level
	//					elapsed_time: detected peak value
	interactive_peak_detector( T id,  uint32_t trigger_level, _peack_detected_call_back fcn )
		: _id(id)
		, _trigger_level(trigger_level)
		, _lcall_back(fcn)
		{}

	// check the elapsed time comparing it with the configured trigger_level
	// calls user callback function f necessary
	virtual void set_elapsed_time( const uint32_t& i )
	{
		if( i > _trigger_level )
		{
			_lcall_back( _id, _trigger_level, i );
		}
	}
private:
	T _id;
	uint32_t _trigger_level;
	_peack_detected_call_back _lcall_back;
};




/// interactive_pool_scoped_connection
/// helper for interactive_pool, releases the instance once
/// the object is out of scope
template < class T> class interactive_pool_scoped_connection
{ public:
	// constructor
	interactive_pool_scoped_connection( 
		interactive_pool<T>* pool							// instance of interactive_pool
		, uint32_t max_wait_ms = 0							// maximun time, in milliseconds, to wait a free instance.  Once this time has elapsed, an exception will be thrown
		, interactive_pool_time* time_elapsed_ms = nullptr	// if metric is desired a interactive_pool_time instance
		, base_detector* detector = nullptr					// if want to use a detector for reporting and alarms 
	) :_p(nullptr) , _pool(pool), _detector(detector)
	{
		(_p) = _pool->get_item(max_wait_ms, time_elapsed_ms);
		if( _detector && time_elapsed_ms)
		{
			_detector->set_elapsed_time(time_elapsed_ms->elapsed_time.count());
		}
	}
	
	// direct access the content
	typename interactive_pool<T>::item& operator->() const
	{
		return (typename interactive_pool<T>::item&) _p;
	}

	// destructor, releases the item (if any) when is outgoing from scope 
	virtual ~interactive_pool_scoped_connection()
	{
		if (_p && _pool)
		{
			_pool->set_item(_p);
		}
	}

// members
private:
	typename interactive_pool<T>::item _p;
	interactive_pool<T>* _pool;
	base_detector* _detector;
};

#endif // INTERACTIVE_POOL__H