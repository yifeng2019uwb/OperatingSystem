//
// main.cpp/assignment2
//
// Simulate the sleeping barbers problem.
//
// Author: Yifeng Zhang base on Prof. Morris Bernstein's skelton file
// Copyright 2019, Systems Deployment, LLC.


#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <random>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

const char* PROG_NAME = "";

class Shop;
class Barber;
class Customer;

class Lock {
public:
    Lock(pthread_mutex_t* mutex): mutex(mutex) {
        int rc = pthread_mutex_lock(this->mutex);
        if (rc < 0) {
            perror("can't lock mutex");
            exit(EXIT_FAILURE);
        }
    }
    
    ~Lock() {
        int rc = pthread_mutex_unlock(mutex);
        if (rc < 0) {
            perror("can't unlock mutex");
            exit(EXIT_FAILURE);
        }
    }
    
private:
    pthread_mutex_t* mutex;
};


class Shop {
public:
    struct BarberOrWait {
        Barber* barber;        // Barber is available
        bool chair_available;    // If barber isn't available,
        // waiting chair is available.
    };
    
    // Constructor initializes shop and creates Barber threads (which
    // will immediately start calling next_customer to fill the
    // collection of sleeping barbers).
    Shop(int n_barbers,
         unsigned waiting_chairs,
         int average_service_time,
         int service_time_deviation,
         int average_customer_arrival,
         int duration);
    
    // Main thread: open the shop and spawn customer threads until
    // closing time.  Report summary statistics for the day.
    void run();
    
    // Customer thread announces arrival to shop. If the collection of
    // currently sleeping barbers is not empty, remove and return one
    // barber from the collection. If all the barbers are busy and there
    // is an empty chair in the waiting room, add the customer to the
    // waiting queue and return {nullptr, true}.  Otherwise, the
    // customer will leave: return {nullptr, false}.
    BarberOrWait arrives(Customer* customer);
    
    // Barber thread requests next customer.  If no customers are
    // currently waiting, add the barber to the collection of
    // currently sleeping barbers and return nullptr.
    Customer* next_customer(Barber* barber);
    
    // Return random service time
    int service_time();
    
    // Return random customer arrival.
    int customer_arrival_time();
    
    ~Shop();

private:
    vector<pthread_t*> barber_threads;  // barber thread
    // Customers on waiting chairs
    queue<Customer*> waiting_customers;
    queue<Barber*> sleeping_barbers;


    // Barbers in shop
    vector<Barber*> barbers;
    
    pthread_mutex_t* shop_mutex;
    //pthread_cond_t shop_cond;

    struct timespec time_limit;
    
    //
    int customers_served_immediately;
    int customers_waited;
    int customers_turned_away;
    int customers_total;
    
    int n_barbers;
    unsigned int waiting_chairs;
    int average_service_time;
    int service_time_deviation;
    int average_customer_arrival;
    int duration;
    
    default_random_engine random_generator;
};


class Barber {
public:
    Barber(Shop* shop, int id);
    ~Barber();
    
    // Barber thread function.
    void run();
    
    // Shop tells barber it's closing time.
    void closing_time();
    
    // Customer wakes up barber.
    void awaken(Customer* customer);
    
    // Customer sits in barber's chair.
    void customer_sits(Customer* customer);
    
    // Customer proffers payment.
    void payment();
    
    int id;
    //bool shop_closing;
    
private:
    Shop* shop;
    pthread_mutex_t* barber_mutex;
    pthread_cond_t barber_cond;
    Customer* current_customer;
    bool isClosing;
    bool isSleeping;
    bool customer_sit_down;
    bool getPayment;
    
};


class Customer {
public:
    Customer(Shop* shop, int id);
    ~Customer();
    
    // Customer thread function.
    void run();
    
    // Barber calls this customer after checking the waiting-room
    // queue (customer should be waiting).
    void next_customer(Barber* barber);
    
    // Barber finishes haircut (Customer should be receiving haircut).
    void finished();
    
    // Barber has accepted payment for service.
    void payment_accepted();
    
    int id;
    
private:
    Shop* shop;
    pthread_mutex_t* customer_mutex;
    pthread_cond_t customer_cond;
    Barber* current_barber;
    bool isWaiting;
    bool hair_cut;
    bool paid;
 
};


// Barber thread.  Must be joined by Shop thread to allow shop to
// determine when barbers have left.
void* run_barber(void* arg) {
    Barber* barber = reinterpret_cast<Barber*>(arg);
    barber->run();
    return nullptr;
}


// Customer thread.  Runs in detatched mode so resources are
// automagically cleaned up when customer leaves shop.
void* run_customer(void* arg) {
    Customer* customer = reinterpret_cast<Customer*>(arg);
    customer->run();
    delete customer;
    return nullptr;
}


Shop::Shop(int n_barbers,
           unsigned int waiting_chairs,
           int average_service_time,
           int service_time_deviation,
           int average_customer_arrival,
           int duration)
{
    // Initialize stats
    customers_served_immediately = 0;
    customers_waited = 0;
    customers_turned_away = 0;
    customers_total = 0;
    
    this->n_barbers = n_barbers;
    this->waiting_chairs = waiting_chairs;
    this->average_service_time = average_service_time;
    this->service_time_deviation = service_time_deviation;
    this->average_customer_arrival = average_customer_arrival;
    this->duration = duration;

    int rc = clock_gettime(CLOCK_REALTIME, &time_limit);
    if (rc < 0) {
        perror("reading realtime clock");
        exit(EXIT_FAILURE);
    }
    // Round to nearest second
    time_limit.tv_sec += duration + (time_limit.tv_nsec >= 500000000);
    time_limit.tv_nsec = 0;
    
    // create n barbers
    for (int i = 0; i < n_barbers; i++) {
        barbers.push_back(new Barber(this, i));
    }
    
    // init shop_mutex
    shop_mutex = reinterpret_cast<pthread_mutex_t*>(malloc(sizeof(pthread_mutex_t)));
    pthread_mutex_init(shop_mutex,nullptr);
    
}

Shop::~Shop() {
    for (int i = 0; i < n_barbers; i++) {
        delete barbers[i];
    }
    delete shop_mutex;
}

void Shop::run() {
    cout << "the shop opens\n";
    
    {
        // lock the shop_mutex, and create barber threads
        Lock lock(shop_mutex);
        for (int i = 0; i < n_barbers; i++){
            pthread_t* thread = reinterpret_cast<pthread_t*>(malloc(sizeof(pthread_t)));
            int rc = pthread_create(thread,nullptr, run_barber,  reinterpret_cast<void*>(barbers[i]));
            if (rc < 0) {
                errno = rc;
                perror("creating barber thread failed");
                exit(EXIT_FAILURE);
            }
            barber_threads.push_back(thread);
        }
    }

    // get customers coming
    for (int next_customer_id = 0; ; next_customer_id++) {
        struct timespec now;
        int rc = clock_gettime(CLOCK_REALTIME, &now);
        if (rc < 0) {
            errno = rc;
            perror("reading realtime clock");
            exit(EXIT_FAILURE);
        }
        if (now.tv_sec >= time_limit.tv_sec) {
            // Shop closes.
            for(int i = 0; i < n_barbers; ++i)
            {
                barbers[i]->closing_time();
            }
            break;
        }
        
        // Wait for random delay, then create new Customer thread.
        usleep(customer_arrival_time());
        
        {
            Lock lock(shop_mutex);
            Customer* new_customer = new Customer(this, next_customer_id);
            cout << "customer " << new_customer->id << " arrives" << endl;
        
            pthread_t* new_customer_thread = reinterpret_cast<pthread_t*>(malloc(sizeof(pthread_t)));
            pthread_create(new_customer_thread, nullptr, run_customer, reinterpret_cast<void*>(new_customer));
            pthread_detach(*new_customer_thread);//
            customers_total++;
            delete new_customer_thread;
        }
    } //end loop for new customer
    
    cout << "the shop closes\n";
    
    // wait for all barber threads to finish
    for (auto thread: barber_threads) {
        pthread_join(*thread, nullptr);
    }
    
    // print statistical data
    cout << "customers served immediately: " << customers_served_immediately << endl;
    cout << "customers waited " << customers_waited << endl;
    cout << "total customers served " << (customers_served_immediately + customers_waited) << endl;
    cout << "customers turned away: " << customers_turned_away << endl;
    cout << "total customers: " << customers_total << endl;
}


Shop::BarberOrWait Shop::arrives(Customer* customer) {
    // Find a sleeping barber.
    {
        Lock lock (shop_mutex);
        if (!sleeping_barbers.empty())
        {
            // Return one sleeping barber
            Barber* working_barber = sleeping_barbers.front();
            sleeping_barbers.pop();
            customers_served_immediately++;
            return BarberOrWait{working_barber, false};
        } else {
            // No barber: check for a waiting-area chair.
            if (waiting_customers.size() < waiting_chairs){
                // waiting chair availale, push in waiting list
                waiting_customers.push(customer);
                customers_waited++;
                return BarberOrWait{nullptr, true};
            } else {
                // Otherwise, customer leaves.
                customers_turned_away++;
                return BarberOrWait{nullptr, false};
            }
        }
    }
}

Customer* Shop::next_customer(Barber* barber) {
    {
        Lock lock(shop_mutex);
        Customer* wait_customer = nullptr;
        if (!waiting_customers.empty())
        {
            wait_customer = waiting_customers.front();
            waiting_customers.pop();
            cout << "barber " << barber->id
            << " calls customer " << wait_customer->id << endl;
        }else {
            cout << "barber " << barber->id << " takes a nap" << endl;
            sleeping_barbers.push(barber);
        }
        return wait_customer;
    } // end unlock mutex
}

int Shop::service_time() {
    normal_distribution<double> distribution(average_service_time, service_time_deviation);
    return max(distribution(random_generator), average_service_time * 0.8) * 1000;
}


int Shop::customer_arrival_time() {
    poisson_distribution<int> distribution(average_customer_arrival);
    return distribution(random_generator) * 1000;
}


Barber::Barber(Shop* shop, int id)
{
    this->shop = shop;
    this->id = id;
    isClosing = false;
    isSleeping = false;
    customer_sit_down = false;
    getPayment = false;
    current_customer = nullptr;
    
    // init mutex
    barber_mutex = reinterpret_cast<pthread_mutex_t*>(malloc(sizeof(pthread_mutex_t)));
    pthread_mutex_init(barber_mutex, nullptr);
}


Barber::~Barber(){
   //this->current_customer = nullptr;
    delete this->current_customer;
   // this->shop = nullptr;
    delete this->shop;
    delete barber_mutex;
}


void Barber::run() {

   cout << "barber " << id << " arrives for work" << endl;
    
    while(!isClosing)
    {
        while (!isSleeping)
        {
            // condition1: barber was awaken by customer, current_customer!=nullptr
            // condition2: barber finished a haircut, call next customer,
            // current_customer = nullptr;
            if ( !current_customer) {
                current_customer = shop->next_customer(this);
            }
            
            if (current_customer) {

                // call customer for haircut
                current_customer->next_customer(this);
                // customer sits in barber's chair
                cout << "customer " << current_customer->id << " sits in barber " << id << "'s chair" << endl;
                customer_sits(current_customer);
                cout << "barber " << id << " gives customer " << current_customer->id << " a haircut" << endl;
                usleep(this->shop->service_time());
                cout << "barber " << id << " finishes customer " << current_customer->id << "'s haircut" << endl;
                current_customer->finished();
                cout << "customer " << current_customer->id << " gets up and proffers payment to barber " << id << endl;
                payment();
                cout << "barber " << id << " accepts payment form customer " << current_customer->id << endl;
                current_customer->payment_accepted();
                
                cout << "customer " << current_customer->id
                    << " leaves satisfied" << endl;
                
                // after all done, change the status variable
                {
                    Lock lock(barber_mutex);
                    current_customer = nullptr;
                    customer_sit_down = false;
                    getPayment = false;
                }
            
            }else {
                // no customer, take a nap and wait for customer to awake
                if (isClosing) break;
                Lock lock(barber_mutex);
                isSleeping = true;
                pthread_cond_wait(&barber_cond, barber_mutex);
            }
            // if (isClosing) break;
        }// end while(isSleeping)
    }// end while(isClosing)
    
    cout << "barber " << id << " leaves for home" << endl;
}



void Barber::closing_time() {
    Lock lock(barber_mutex);
    isClosing= true;
    pthread_cond_signal(&barber_cond);
}


void Barber::awaken(Customer* customer) {
    
    Lock lock(barber_mutex);
    cout << "barber " << id << " wakes up" << endl;
    this->current_customer = customer;
    isSleeping = false;
    pthread_cond_signal(&barber_cond);
}


void Barber::customer_sits(Customer* customer) {
    Lock lock(barber_mutex);
    customer_sit_down = true;
    pthread_cond_signal(&barber_cond);
}


void Barber::payment() {
    Lock lock(barber_mutex);
    getPayment = true;
    pthread_cond_signal(&barber_cond);
}


Customer::Customer(Shop* shop, int id)
{
    this->shop = shop;
    this->id = id;
    current_barber = nullptr;
    paid = false;
    isWaiting = false;
    hair_cut = false;
    //init mutex
    customer_mutex = reinterpret_cast<pthread_mutex_t*>(malloc(sizeof(pthread_mutex_t)));
    pthread_mutex_init(customer_mutex, nullptr);
    
}


Customer::~Customer() {
    this->shop = nullptr;
    delete this->shop;
    this->current_barber = nullptr;
    delete this->current_barber;
    delete customer_mutex;
 }


void Customer::run() {
    // customer check if there are available barber or waiting chair;
    // if barber available --> wake the barber
    // if barber not available and waiting chair available --> takes a sit in waiting room
    // or turn away
    Shop::BarberOrWait barberOrWait = shop->arrives(this);
    
        if (barberOrWait.barber) {
            // if barber are available, wake the barber
           // this->current_barber = barberOrWait.barber;
            cout << "customer " << id << " wakes barber " << barberOrWait.barber->id << endl;
            isWaiting = true;
            barberOrWait.barber->awaken(this);
        }else {
            // if barber == nullptr, check the waiting chairs
            if (barberOrWait.chair_available) {
                // take a sit in waiting room
                cout << "customer " << id << " takes a set in the waiting room" << endl;
                {
                    Lock lock(customer_mutex);
                    isWaiting = true;
                    pthread_cond_wait(&customer_cond, customer_mutex);
                }
            }else {
                cout << "customer " << id << " leaves without getting haircut" << endl;
                //isWaiting = false;
                return;
            }
        }
}


void Customer::next_customer(Barber* barber) {
    Lock lock(customer_mutex);
    this->current_barber = barber;
    isWaiting = false;
    pthread_cond_signal(&customer_cond);
}


void Customer::finished() {
    Lock lock(customer_mutex);
    hair_cut = true;
    pthread_cond_signal(&customer_cond);
}


void Customer::payment_accepted() {
    Lock lock(customer_mutex);
    paid = true;
    pthread_cond_signal(&customer_cond);
}


void usage() {
    cerr
    << "usage: "
    << PROG_NAME
    << " <nbarbers>"
    << " <nchairs>"
    << " <avg_service_time>"
    << " <service_time_std_deviation>"
    << " <avg_customer_arrival_time>"
    << " <duration>"
    << endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    PROG_NAME = argv[0];
    
    if (argc != 7) {
        usage();
    }
    int barbers = atoi(argv[1]);
    if (barbers <= 0) {
        usage();
    }
    int chairs = atoi(argv[2]);
    if (chairs < 0) {
        usage();
    }
    int service_time = atoi(argv[3]);
    if (service_time <= 0) {
        usage();
    }
    int service_deviation = atoi(argv[4]);
    if (service_time <= 0) {
        usage();
    }
    int customer_arrivals = atoi(argv[5]);
    if (customer_arrivals <= 0) {
        usage();
    }
    int duration = atoi(argv[6]);
    if (duration <= 0) {
        usage();
    }
    
    Shop barber_shop(barbers,
                     chairs,
                     service_time,
                     service_deviation,
                     customer_arrivals,
                     duration);
    barber_shop.run();
    
    return EXIT_SUCCESS;
}
