# interactive_pool
A multi-platform pool of resources / threads, in a single header file that allows the measurement of connection times and the use of specialized plugins
to issue alerts or trigger actions when access times to resources exceed a set threshold.

In the "include" folder is the header file with the definition of the class and all its plugins.
Examples of using the pool and its plugins are available in the "examples" folder.

The pool can be used, directly or through a scope template. The scope template, ensures to automatically release the items obtained, once it goes out of its scope

## Direct access of class

When direct access to the pool is used, you must manually release the resource, once used. Below is an example of direct access without 
obtaining the time interval to access the resource and its second example capturing the elapsed time to access the resource.

### Direct access without metric example
```	cpp

/// Worker thread that consumes the pool's resources 
void worker(interactive_pool< MyConnectors >* pool)
{
	for (int i = 0; i < operations; i++)
	{
		try
		{
			// max waits 1 second to get an instance, 
			interactive_pool<MyConnectors>::item c = pool->get_item(1000);

			// ... 
			// use the instance 
			c->doSOmeThing();
			
			// reelase item ( very important ! )
			pool->set_item(c);
		}
		catch (std::exception& e)
		{
			cout << "Thread " << std::this_thread::get_id() << " Exception " << string(e.what()) << endl;
		}

		// sleep a little between works
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	}
}
```

### Direct access getting metrics. (Example)
```	cpp

/// Worker thread that consumes the pool's resources 
void worker(interactive_pool< MyConnectors >* pool)
{
	for (int i = 0; i < operations; i++)
	{
		try
		{
			interactive_pool_time elapsedTime; // get elapsed time necessary to connect
			// max waits 1 second to get an instance, 
			interactive_pool<MyConnectors>::item c = pool->get_item(1000, &elapsedTime);
			cout << "Thread " << std::this_thread::get_id() << " got item in " << elapsedTime.elapsed_time.count() << " ms" << endl;
			// ... 
			// use the instance 
			c->doSOmeThing();
			
			// reelase item ( very important ! )
			pool->set_item(c);
		}
		catch (std::exception& e)
		{
			cout << "Thread " << std::this_thread::get_id() << " Exception " << string(e.what()) << endl;
		}

		// sleep a little between works
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
	}
}
```


## Access with an scoped handler (recomended)
The pool class also gives a scope handler make easy and more safe managing the pool 

### Scoped access getting metrics. (Example)

```	cpp
void worker_with_scope(interactive_pool<Foo>* pool)
{
	for (int i = 0; i < operations; i++) 
	{
		try
		{
			interactive_pool_time elapsedTime; // get elapsed time necessary to connect
			interactive_pool_scoped_connection<Foo> c( pool, 2000, &elapsedTime );

			cout << "Thread " << std::this_thread::get_id() << " got item in " << elapsedTime.elapsed_time.count() << " ms" << endl;
			c->doSOmeThing();
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

```

## Calling a test or initialize function when get an instance from pool
You may wish to test if the item in the pool is connect or initialized, also you may be intersted in reconnect a item if the connection was lost.
In that case you can specify a test function to be executed before return a instance of the pool's item. When the function returns "true" the item will be returned in other case will keep waiting for a "good" item.

### Scoped access getting metrics and test items. (Example)

```	cpp
void worker_with_scope(interactive_pool<Foo>* pool)
{
	for (int i = 0; i < operations; i++) 
	{
		try
		{
			interactive_pool_time elapsedTime; // get elapsed time necessary to connect
			interactive_pool_scoped_connection<Foo> c( pool, 2000, &elapsedTime ,
			[](interactive_pool<Foo>::item& i ) 
				{
					if( !i->is_connected() )
					{
						cout << "Thread " << std::this_thread::get_id() << " Foo lost connecttion will connect again"  << endl;
						return i->connect(); // returns true or false
					}
						
					cout << "Thread " << std::this_thread::get_id() << " successfully got connection"  << endl;
					return true;
				}
			);
			);

			cout << "Thread " << std::this_thread::get_id() << " got item in " << elapsedTime.elapsed_time.count() << " ms" << endl;
			c->doSOmeThing();
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

```


## Cotrolling the time to access your resources
You may wish to issue an alert or trigger an action when access times to pool resources exceed a pre-set threshold.
2 plugins are offered : 

**interactive_pool_average_detector** : Takes a configurable number of the last access time samples and calculates the average access time. 
When it exceeds a set threshold, the user calls a function.

**interactive_pool_peak_detector** : Calls a user defined function each time that the access duration exceeds the set threshold.


### Example using average detector and calling a labda funcrion
(see examples for a more detailed information)

```	cpp

// worker thread 
// worker_with_scope_average_detector
void worker_with_scope_average_detector(interactive_pool< Foo > * pool)
{
	// interactive_average_detector plugin
	// allow to call a function or a lambda to trigger an alert when the average connection time exceeds a configured threshold value
	// In this case we are using the thread id as indetification, but can be a string or othe value to identify the caller or the pool
	// It is your choice.
	// interactive_average_detector< T = identification type >  Ex.: interactive_average_detector<std::string>
	// Id -> Thread Id
	// Samples -> number of last "n" samples to calculate the average.
	// Trigger level , threshold value
	// Function to call with parameters
	//	-> id, same as constructor
	//  -> trigger_level , same tha constructor
	//	-> sample_average_alarm : average calculate when is above the threshold
	
	interactive_average_detector<std::thread::id> _average ( std::this_thread::get_id(), 5, 1300,
		[]( const std::thread::id& id, uint32_t trigger_level, uint32_t sample_average_alarm )
		{
			cout << "Average time to get an instance from the pool has exceeded the threshold (" << trigger_level << "ms) connection time: " << sample_average_alarm << "ms. " << " Informer Thread : " << id << endl;
		}
	);
	
	
	for (int i = 0; i < operations; i++)
	{
		interactive_pool<Foo>::item c;

		try
		{
			interactive_pool_time elapsedTime;
			interactive_pool_scoped_connection<Foo> c( pool, numeric_limits<uint32_t>::max(), &elapsedTime, &_average );
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

```




## Compiling examples
Detailed usage of all items are introduced in examples. To compile the examples just follow the bellow instructions :

### Windows Users
In vs folder you can find a VS Solution. All examples are defined as projects an you can compile all 

### Linux Users  
Justo go to the desired example an follow the next steps : 

```
cd ./interactive_pool/examples/scoped_pool_with_average_time_control/
md build
cd build
cmake ...
make all
```

