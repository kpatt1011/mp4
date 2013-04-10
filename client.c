/* 
		READ THE NOTE IN THE REPORT BEFORE GRADING!!!

		TO RUN PROGRAM ENTER COMMAND IN COMMAND LINE AS FOLLOWS
		
		./client 4 5 6
				- where each of the digits is a parameter as specified in the project specs
				
		RESULTS OF THE HISTOGRAMS ARE INCLUDED IN THE stats.txt file!
		
    File: client.c

    Author: Keith Pattison
           
    Date  : 2013/03/08

    Client main program for MP3 in CSCE 313
		
		
*/





/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>      /* printf, fgets */
#include <stdlib.h>     /* atoi */
#include <sstream>
#include <string>
#include <vector>
#include <pthread.h>
#include <errno.h>
#include "reqchannel.H"
#include <stack>
#include <iostream>
#include <fstream>

using namespace std;


/*I was having VERY significant issues working with makefiles and the linker, so to save time and focus on the acutal
  programming part of the assignment, I just included the Semaphore, and Statistics classes in this file. This doesn't
	make for very good code clarity, or easy grading, and I apologize for that in advance. But it's better than nothing!
	Thanks :)
	*/

struct Semaphore {

  /* -- INTERNAL DATA STRUCTURES
     You may need to change them to fit your implementation. */

  int             value;
  pthread_mutex_t m;
  pthread_cond_t  c;

  Semaphore(int _val); // Constructor
	
  Semaphore(); // Default Constructor

  ~Semaphore(); // Destructor

  /* -- SEMAPHORE OPERATIONS */
  
  int P(); // Decrements the semaphore S, P is also known as wait()
  
  int V();  // V increments the semaphore S, also known as signal()
};

  Semaphore::Semaphore(int _val) {
	
		m = PTHREAD_MUTEX_INITIALIZER;
		c = PTHREAD_COND_INITIALIZER;
		value= _val; // Sets the value of the semaphore equal to the predetermined value
  }


  Semaphore::~Semaphore() {
	}
	

  /* -- SEMAPHORE OPERATIONS 

  // P is also known as wait()
  // Decrements the semaphore S
  // Decrements the value of semaphore variable by 1. If the balue becomes negative,
  // the process executing wait() is blocked. i.e: added to the semaphores queue
  */
  int Semaphore::P() {
	
	
	
		pthread_mutex_lock( &m );  // pthread_mutex_lock protect the critical region that is the memory segment for the value
	
			value--;
		
			if(value < 0) {
				pthread_cond_wait(&c, &m); // Wait while some other function operates on value?
			}
		
		pthread_mutex_unlock( &m ); 
		
		return value;
	
	

  }
  
  /* V increments the semaphore S
  // V is also known as signal()
  /* Increments the value of semaphore variable by 1. After the increment, if the pre-increment value was 
  negative (meaning there are processes waiting for a resource), it transfers a blocked process from the 
  semaphores waiting queue to the ready queue
  */
  int Semaphore::V() {
	
		pthread_mutex_lock(&m); // protects the critical region that is the memory segment for the value
			
			value++; // Increment the value
			
			if(value == 0) {
				pthread_cond_signal(&c); // Allow the other process to execute and mess with count
			}
		
		pthread_mutex_unlock(&m);
	
	
		return value;
  }

	
	
	
	
/* A statistics class where a histogram is represented by an array of integers.
	where the index in the array represents the value, and the actual value represents
	the frequency
*/
class Statistics {
	public:
			vector<int> histogram;
			
			Statistics(int size);
			
			void add_entry(int index);
			
			void print_histogram();
			
			void print_histogram_to_file(ofstream &file);
};

Statistics::Statistics(int size) {
	histogram = vector<int> (size);
}

void Statistics::add_entry(int index) {
		histogram[index]++; // Add one to the value of that current index
}

void Statistics::print_histogram() {
	for(int i=0; i < this->histogram.size(); i++) {
		cout<<  "Index:" << i <<"  Frequency:"<< histogram[i] <<"\n"; 
	}
}

void Statistics::print_histogram_to_file(ofstream &file) {

			for(int i=0; i < this->histogram.size(); i++) {
				file<<  "Index:" << i <<"  Frequency:"<< histogram[i] <<"\n"; 
			}	
}



/* Class provides two functions that return the read and write file
descriptor of the request channel, respectivley. These file descriptors can be used 
to monitor activity on the request channel.

If activity has been detected on the read file descriptor, the code 
may then read the data either by accessing RequestChannel::cread() or by
reading directly from the file descriptor returned by RequestChannel::read_fs()

Similarly the next request can be sent to the request channel using RequestChannel::cwrite()
or by writing to file descriptor specified by RequestChannel::write_fs()
*/


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
		int* num_data_request;
		int* bounded_buf_size;
    	int* num_request_channels;

		
		bool keep_going; // Global variable to signal when program should halt
		int total_count; // The total number of request put into all of the histograms (cummulative) 
		
		RequestChannel* chan; // Main control channel
		
		// Semaphores used to solve producer/consumer issues between threads
		Semaphore* request_fillCount;
		Semaphore* request_emptyCount;
		Semaphore* stat_fillCount;
		Semaphore* stat_emptyCount;
		
		// Statistics classes for each person to record the histogram
		Statistics* JaneStats; 
		Statistics* JohnStats;
		Statistics* JoeStats;
		
		stack<string> request_stack; // Used for communication between request threads and worker threads
		stack<string> stats_stack; // Used for communication between worker threads and statistics threads
		vector<RequestChannel*> rChannels;
		

void *event_handler(void *arg) {

	keep_going=true;
	fd_set readset;
	fd_set writeset;


	for(int i=0; i < *num_request_channels; i++) {

		string shit = rChannels[i]->name();
		cout <<"\n" << shit << "   "  << i << "\n";

		/*cout<<"\nGOT HERE \n";
		int h = rChannels.size();;
		cout<< "\n" << h << "\n";
		int rfd = rChannels[i]->read_fd();
		int wfd = rChannels[i]->write_fd();
		
		FD_SET(rfd, &readset);
		FD_SET(wfd, &writeset);
		*/
	}


	while(keep_going) {
		
		int highest_fd = rChannels[*num_request_channels]->read_fd();
		highest_fd++; // Increase it to one more than the largest file descriptor
		int selected = select(highest_fd, &readset, &writeset, NULL, NULL);
		
		cout<<"\nThe Event Handler Selected: " << selected << "\n";

	}	

}
		

// Implementation of a request thread
void *request_thread(void *arg) {
		

		 
			
		string* request_name = (string*) arg;

		string request;
		request = "data "+ *request_name;
		
		// Add 1000 requests to the stack for the worker threads to consume
		for(int i=0; i < 1000; i++){
			
			request_emptyCount->P(); 
					request_stack.push(request);
			request_fillCount->V();
		}
		
}

// Implementation of a statistics thread
void *statistics_thread(void *arg) {
	

		/*		while(keep_going) {
				
					string reply;
					string request;
					stringstream ss;
					
					// Make sure the stack isn't empty before trying to access the stack
					
						if(stats_stack.size() != 0) {
						
					 stat_fillCount->P(); // As a consumer of statistics threads this decrements the fill count
	
								// Read both the reply and the request from the stack. 
							 request = stats_stack.top();
							 stats_stack.pop();
							 reply = stats_stack.top();
							 stats_stack.pop(); 
						stat_emptyCount->V(); // Increment the empty count
							 	
				
				
					
			
			
					// Convert the returned string into an integer
					int value;
					
					ss << reply;
					ss >> value;
				
			
					// Add a histogram value to one of the histograms
					if(request == "data Jane Smith") {
						JaneStats->add_entry(value);
					}
					
					if(request == "data John Doe"){
						JohnStats->add_entry(value);
					}
					
					if(request == "data Joe Smith") {
							JoeStats->add_entry(value);
					}
					
					total_count++;
					
				}	
			}
			*/
}

// Implementation of a worker thread
void *worker_thread(void *arg){
	
		string name = chan->send_request("newthread");
		RequestChannel* worker= new RequestChannel(name, RequestChannel::CLIENT_SIDE);
		
		// Continue to loop until the global keep_going becomes false signaling the desired number of elements in the histogram 
		// have been reached
		
		while(keep_going) {
			string request;
			
			if((request_stack.size() != 0)) {
				
				request_fillCount->P(); // Decrement fillCount
					request = request_stack.top();
					request_stack.pop();
				request_emptyCount->V();
					
					string reply = worker->send_request(request);
				
				stat_emptyCount->P();
					stats_stack.push(reply);
					stats_stack.push(request);
				stat_fillCount->V(); 
				
			} // End of if statement
		}
		
		string last_reply = worker->send_request("quit"); // Quit the request channel of the worker thread	
}

/* 
client -n -b -w

-n <number of data requests per a person>

-b <size of bounded buffer in request>

-w <number of request channels to be handled by event handler thread>
*/

/*MAIN FUNCTION*/
int main(int argc, char * argv[]) {

			keep_going = true; // This the variable to make everything keep going
			int total_count=0;
			
				// Establish Statistics Threads
				JaneStats = new Statistics(100);
				JohnStats = new Statistics(100);
				JoeStats = new Statistics(100);
		
			// Turn argument char values into int* values
			int dr = atoi(argv[1]);
			num_data_request = &dr;
			
			int bb = atoi(argv[2]);
			bounded_buf_size = &bb;
			
			int w=atoi(argv[3]);
			num_request_channels = &w;
		
		
			// Instantiate the required fillcount and emptycount semaphores to solve consumer/producer problem amongst the threads
			request_fillCount = new Semaphore(0);
			request_emptyCount = new Semaphore(*bounded_buf_size);
		
			stat_fillCount = new Semaphore(0);
			stat_emptyCount = new Semaphore(*bounded_buf_size);
			
			pid_t childpid;
	
		childpid = fork(); // Fork process to start the dataserver in a seperate thread
	
	if(childpid == -1) {
		perror("Failed to fork dataserver process\n");
	}
	if(childpid == 0) {  /* Child Process */
		execl("./dataserver", "dataserver", NULL, NULL);
		cout << "Dataserver STARTED:" << endl;
	}
	else {
	
	usleep(200000);  // Sleep to allow adequate time for chan to connect
	cout << "Establishing control channel... " << flush;
  chan = new RequestChannel("control", RequestChannel::CLIENT_SIDE);
  cout << "done." << endl;;

		// Create Request Threads
	pthread_t jane;
	pthread_t joe;
	pthread_t john;
	
	string jas = "Jane Smith";
	string jos = "Joe Smith";
	string jod = "John Doe";
	
	int error;
	
	//error = pthread_create(&jane, NULL, request_thread, (void*) &jas  );
	/*
	if(error != 0) {
		perror("\nUnable to create request thread for JANE\n");
	}
	
	error = pthread_create(&joe, NULL, request_thread, (void*) &jos );
	if(error != 0) {
		perror("\nUnable to create request thread for JOE\n");
	}
	
	error = pthread_create(&john, NULL, request_thread, (void*) &jod );
	if(error != 0) {
		perror("\nUnable to create request thread for JOHN\n");
	}
	*/

	// Create the designated number of RequestChannels, and add them to the array of RequestChannels
	for(int i =0; i < *num_request_channels; i++) {

		string name = chan->send_request("newthread"); // Request a new RequestChannel on the server
		RequestChannel* a= new RequestChannel(name, RequestChannel::CLIENT_SIDE);
		
		rChannels.push_back(a);

	}



	pthread_t eventHandler;

	error = pthread_create(&eventHandler, NULL, event_handler, NULL); 

	if(error != 0) {
		perror("\nUnable to create event handler thread\n");
	}


	usleep(200000);  // Sleep to allow adequate time for chan to connect

	//string fuck = rChannels[3]->name();
	//string fuck = rChannels[0]->name();
	//cout << "\n" << fuck << "\n";
	//rChannels[5]->cwrite("Testing if this worked");
	string fuck = rChannels[0]->cread();
	cout << "\n" << fuck << "\n";

	// Create worker threads
	
	//for(int i =0; i < *num_worker_threads; i++) {
	
			//	pthread_t worker; // Declare worker thread
				
			//	error = pthread_create(&worker, NULL, worker_thread, NULL);
				
			//	if(error != 0) {
			//		perror("\nUNABLE TO CREATE A WORKER */THREAD");
			//	}
//	}
	
	
	/*
	// Create statistics threads
	pthread_t janeStat;
	pthread_t joeStat;
	pthread_t johnStat;
	
	
	error = pthread_create(&janeStat, NULL, statistics_thread, NULL );
	
	if(error != 0) {
		perror("\nUnable to create statistics thread for JANE\n");
	}
	
	error = pthread_create(&joeStat, NULL, statistics_thread, NULL );
	if(error != 0) {
		perror("\nUnable to create statistics thread for JOE\n");
	}
	
	error = pthread_create(&johnStat, NULL, statistics_thread, NULL );
	if(error != 0) {
		perror("\nUnable to create statistics thread for JOHN\n");
	}
	
	
	// Print histogram results into a file to check for accuracy.
	int stat_round = 1;
	ofstream myfile;
	//myfile.open ("stats.txt");
	
	while(keep_going) {
		usleep(20000000);  // Sleep to allow adequate time for chan to connect
		if(true) {
				myfile<< "\nThese are the end result of the histograms \n\n";
		
				
				myfile<< "JANE STATS\n";
				JaneStats->print_histogram_to_file(myfile);
		
				myfile<< "Joe STATS\n";
				JoeStats->print_histogram_to_file(myfile);
		
				myfile<< "John STATS\n";
				JohnStats->print_histogram_to_file(myfile);
				
				//keep_going = false;
		}
	
		

		
		
	}
	myfile.close(); // Close the statistics log file
*/	
	string reply4 = chan->send_request("quit"); // Quit the chan request channel

	}


}

