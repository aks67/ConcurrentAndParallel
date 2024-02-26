#include <iostream>
#include <thread>

using namespace std;


/*
    Base Definitions
*/


void do_something(int i) {
    cout << "Doing something: " << i  << endl;
}


struct func {
   int& i;
   func(int&  i_) : i(i_) {}

    void operator()() {
        for (int j = 0; j < 10; j++) {
            do_something(i);
        }
    }
};

void oops() {
    int local_state = 0;
    func my_func(local_state);
    thread my_thread(my_func);
    my_thread.detach();
}

/*
    First program using threads - hello world using concurrency
*/
void helloWorld() {
    cout << "Hello World" << endl;
}

void threadIt(void (*func)()) {

    thread t(func);
    t.detach();

}

/*
    Ensuring the thread is finished
*/


/*
    Thread guaridng using RAII - Resource Acquisition IS Initialization
*/
class thread_guard {
    thread& t;

    public:
        explicit thread_guard(thread& t_): t(t_) {}
            
        //Destructor
        ~thread_guard() {
            if (t.joinable()) {
                t.join();
            }
        }

        thread_guard (thread_guard const&) = delete;
        thread_guard& operator=(thread_guard const&)=delete;
};




/*
    Scope thread
*/

class scoped_thread {
    thread t;
    public:
        explicit scoped_thread(thread t_): t(move(t_)) {
            if (!t.joinable()) throw logic_error("No Thread");
        }            
        
        ~scoped_thread() {
            t.join();
        }


        scoped_thread(scoped_thread const&) =delete;
        scoped_thread& operator= (scoped_thread cosnt&) =delete;
};



void f() {
    int local_state = 0;
   scoped_thread t(thread(func(local_state)));
    do_something(local_state);
}



/*

    Detachign threads to handle other processes
*/


int main(void) {
    
    f();
    return 0;
}