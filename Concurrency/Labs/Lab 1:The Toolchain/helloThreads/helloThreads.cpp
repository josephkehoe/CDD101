/*! \file hellothreads.cpp
    \brief A Documented file.
    
    You should document all your files using doxygen.  
    NOTE: Any files not documented in this manner will not be accepted!
*/
#include <iostream>
#include <thread>
#include <vector>

/*! \var static const int num_threads=10
    \brief The number of threads we intend to create
*/
 static const int num_threads = 10;
  
 
 /*! \fn void call_from_thread(int tid)
    \brief This function will be called from a thread
    \param tid the thread number
    
    tid is assigned a number when the thread is created. 
    Any function  created by a thread must have this function signature
    The function prints out its thread id.
*/ 
 void call_from_thread(int tid) {
          std::cout << "Launched by thread " << tid << std::endl; 
}


 /*! \fn int main()
    \brief The eponymous main function
    
    Forks off 10 threads using a vector of num_threads std::thread objects.
    Then joins each thread with the main thread and exits 
   
*/
int main() {

	 std::vector<std::thread> vt(num_threads);
 
          /**< Launch a group of threads  */ 
	 int i=0;
	 for(std::thread& t: vt){
	   t=std::thread(call_from_thread,i++);
	 }

 
         std::cout << "Launched from the main\n";
 
         /**< Join the threads with the main thread */
         for (auto& v :vt){
	   v.join();
         }
 
         return 0;
}


