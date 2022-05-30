#include <getopt.h>
#include <cstdlib>
#include <ctype.h>
#include <signal.h>
#include <cstdio>

#include <Os/Log.hpp>
#include <GpsApp/Top/GpsAppTopologyAc.hpp>

void print_usage(const char* app) {
    (void) printf("Usage: ./%s [options]\n-p\tport_number\n-a\thostname/IP address\n",app);
}

// Topology state structure
GpsApp::TopologyState state;

// Enable the console logging provided by Os::Log
Os::Log logger;

// Set 'terminate' flag
volatile sig_atomic_t terminate = 0;

static void sighandler(int signum) {
    GpsApp::teardown(state);
    terminate = 1;
}

void run1cycle() {
    // call interrupt to emulate a clock
    GpsApp::blockDrv.callIsr();
    Os::Task::delay(1000); //10Hz
}

void runcycles(NATIVE_INT_TYPE cycles) {
    if (cycles == -1) {
        while (true) {
            run1cycle();
        }
    }

    for (NATIVE_INT_TYPE cycle = 0; cycle < cycles; cycle++) {
        run1cycle();
    }
}

int main(int argc, char* argv[]) {
    U32 port_number = 0; // Invalid port number forced
    I32 option;
    char *hostname;
    char *device;
    option = 0;
    hostname = nullptr;

    while ((option = getopt(argc, argv, "hp:a:d:")) != -1){
        switch(option) {
            case 'h':
                print_usage(argv[0]);
                return 0;
                break;
            case 'p':
                port_number = static_cast<U32>(atoi(optarg));
                break;
            case 'a':
                hostname = optarg;
                break;
            case 'd':
                device = optarg;
                break;
            case '?':
                return 1;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // console prompt for quit
    (void) printf("Hit Ctrl-C to quit\n");

    // run setup
    state = GpsApp::TopologyState(hostname, port_number, device);
    GpsApp::setup(state);

    // register signal handlers to exit program
    signal(SIGINT,sighandler);
    signal(SIGTERM,sighandler);
    
    // run block driver timer
    int cycle = 0;

    while (!terminate) {
        //(void) printf("Cycle %d\n",cycle);
        runcycles(1);
        cycle++;
    }

    // Give time for threads to exit before shutdown
    (void) printf("Waiting for threads...\n");
    Os::Task::delay(1000);

    (void) printf("Exiting...\n");

    return 0;
}