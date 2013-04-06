#include "semaphore.H"
#include <pthread.h>

/*
internal datastructures
*/

 

  

  /* -- CONSTRUCTOR/DESTRUCTOR */
  
  /* Here is the link to what you need to make this work
	http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html#BASICS
  */
	
using namespace std;

  Semaphore::Semaphore(int _val) {
	
		m = PTHREAD_MUTEX_INITIALIZER;
		c = PTHREAD_COND_INITIALIZER;
		value= _val; // Sets the value of the semaphore equal to the predetermined value
  }


  Semaphore::~Semaphore() {
	}
	
	

  /* -- SEMAPHORE OPERATIONS */

  // P is also known as wait()
  // Decrements the semaphore S
  // Decrements the value of semaphore variable by 1. If the balue becomes negative,
  // the process executing wait() is blocked. i.e: added to the semaphores queue
  
  int Semaphore::P() {
	
	// pthread_mutex_lock protect the critical region that is the memory segment for the value
	
		pthread_mutex_lock( &m ); 
	
			value--;
		
			if(value < 0) {
				pthread_cond_wait(&c, &m); // Wait while some other function operates on value?
			}
		
		pthread_mutex_unlock( &m ); 
		
		return value;
	
	

  }
  
  // V increments the semaphore S
  // V is also known as signal()
  /* Increments the value of semaphore variable by 1. After the increment, if the pre-increment value was 
  negative (meaning there are processes waiting for a resource), it transfers a blocked process from the 
  semaphores waiting queue to the ready queue
  */
  
  int Semaphore::V() {
  
	// pthread_mutex_lock protect the critical region that is the memory segment for the value
	
		pthread_mutex_lock(&m);
			
			value++; // Increment the value
			
			if(value == 0) {
				pthread_cond_signal(&c); // Allow the other process to execute and mess with count
			}
		
		pthread_mutex_unlock(&m);
	
	
		return value;
  }
	
	int main() {
	}